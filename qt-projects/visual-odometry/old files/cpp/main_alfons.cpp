#include </home/pablo/opencv/include/opencv2/opencv.hpp>

using namespace cv;
using namespace std;

//Body section defintions
enum bodySections{
    BODY_NECK,
    BODY_CHEST,
    BODY_WAIST,
    BODY_HIP
};

//3D vector holding coordinates for origin and point location in image
struct Vector3D{
    Matx31d Org = Matx31d(0,0,0);
    Matx31d Scl = Matx31d(0,0,0);
};
//Comparator used to sort vector of Point2f's according to x-axis values
struct str{
    bool operator() (Point2f a, Point2f b){
        if (a.x != b.x) return a.x < b.x;
        return a.y <= b.y;
    }
}comp;

const int startFrame = 151;
const int skipFrame = 10;

const string circPointsName = "pos3D";
const string fileHipCirc = "point-clouds/hipCirc.xyz";       //Output file path for the hip circumference points
const string fileWaistCirc = "point-clouds/waistCirc.xyz";   //Output file path for the waist circumference points
const string fileChestCirc = "point-clouds/chestCirc.xyz";   //Output file path for the chest circumference points
const string fileNeckCirc = "point-clouds/neckCirc.xyz";     //Output file path for the neck circumference points
const string file3D = "point-clouds/points3D.xyz";           //Output file path for point coordinates in real space
const string filePoints = "points.xml";                      //Output file path for feature location within image space

const string fileML = "bodypose_3dbp_alfons.json";           //Input file path for machine learning skeleton coordinates
const string fileVid = "./photos/alfonso_side_low2.mp4";      //Input file path
const string calib_file = "calibration_output_vid.xml";      //Input file path for camera calibration file for videos at full resolution (16:9) of Samsung Galaxy S20 FE 5G camera
//const string calib_file = "calibration_output.xml";         //Input file path for camera calibration file for images at full resolution (4:3) of Samsung Galaxy S20 FE 5G camera
//const string calib_file = "calibration_output_fourth.xml";  //Input file path for camera calibration file for images at 1/4th of full resolution (4:3) of Samsung Galaxy S20 FE 5G camera

const double knownUnit = 1830;          //Known distance between two points in mm to find it
const Matx31d fp = Matx31d(0, 0, 0);    //Focal point is on z-plane to keep focal length positive
const Scalar white(255,255,255), black(0,0,0); //Shorthand for black and white BGR values

double imgScalar = 0;   //Scalar used to set point coordinates in real space
bool setWrite = false;  //Boolean checking whether to read/write from/to points.xml (will write on empty/corrupted file)
bool imgInput = true;   //Boolean checking whether input format is video or image set

Mat cameraMatrix, map1, map2;               //Matrices containing the camera matrix, distortion correction maps, rectified images, and grayscale rectified images
Mat distCoeffs = Mat::zeros(8, 1, CV_64F);  //Matrix containing the distortion coefficients (empty in case camera calibration file cannot be opened)
Mat img = imread("photos/tom_front1.jpg");  //First photo/frame in group/video (used to set size throughout program)
VideoCapture capture(fileVid);              //Retrieve video input

vector<uchar> status;   //Vector containing point and corresponding filter status for tracking

//Size imgSize = img.size(); //Size of images used throughout program
//Size winSize = Size(img.size().width/2, img.size().height/4); //Size of windows used throughout program
Size imgSize = Size(capture.get(CAP_PROP_FRAME_HEIGHT), capture.get(CAP_PROP_FRAME_WIDTH)); //Size of frames used throughout program
Size winSize = imgSize; //Size of windows used throughout program

//Dot product matrix multiplication
float DotProd(Matx31d a, Matx31d b){
    return a(0,0)*b(0,0)+a(0,1)*b(0,1)+a(0,2)*b(0,2);
}

//Dot product matrix multiplication (for 3x1 matrix)
Matx31d DotProdMatx31d(Mat E, Matx31d point){
    double new_x = E.at<double>(0,0)*point(0)+E.at<double>(0,1)*point(1)+E.at<double>(0,2)*point(2);
    double new_y = E.at<double>(1,0)*point(0)+E.at<double>(1,1)*point(1)+E.at<double>(1,2)*point(2);
    double new_z = E.at<double>(2,0)*point(0)+E.at<double>(2,1)*point(1)+E.at<double>(2,2)*point(2);

    return Matx31d(new_x, new_y, new_z);
}

//Homogenous transformation calculation
Matx31d HRotTrans(Mat Rot, Matx31d Trans, Matx31d point){
    double new_x = (Rot.at<double>(0,0)*point(0))+(Rot.at<double>(0,1)*point(1))+(Rot.at<double>(0,2)*point(2))+Trans(0);
    double new_y = (Rot.at<double>(1,0)*point(0))+(Rot.at<double>(1,1)*point(1))+(Rot.at<double>(1,2)*point(2))+Trans(1);
    double new_z = (Rot.at<double>(2,0)*point(0))+(Rot.at<double>(2,1)*point(1))+(Rot.at<double>(2,2)*point(2))+Trans(2);

    return Matx31d(new_x, new_y, new_z);
}

//Function to return random colors
Scalar randColor(){
    return Scalar(int(rand()%255+1), int(rand()%255+1), int(rand()%255+1));
}

//Returns circumferences for different parts of the body - FIX LIMITS THEY ARE NOT CORRECT
double findCirc(Mat pose25d, vector<Matx31d> realPos3D, vector<Point2f> points, double focal, bodySections bodySection){
    string filename;
    double xMin=0, xMax=0, yMin=0, yMax=0;
    switch (bodySection){
    case BODY_NECK:
        xMin = pose25d.at<double>(6,0)-50; //60
        xMax = pose25d.at<double>(6,0)+60; //60
        yMin = pose25d.at<double>(6,1)-80;
        yMax = pose25d.at<double>(6,1)-40;
        filename = fileNeckCirc;
        break;
    case BODY_CHEST:
        xMin = pose25d.at<double>(21,0)+35; //10
        xMax = pose25d.at<double>(20,0)-210; //30
        yMin = pose25d.at<double>(3,1)-75; //75
        yMax = pose25d.at<double>(3,1)-25; //10
        filename = fileChestCirc;
        break;
    case BODY_WAIST:
        xMin = pose25d.at<double>(2,0)+10; //10
        xMax = pose25d.at<double>(1,0)-30; //30
        yMin = pose25d.at<double>(3,1)+90;
        yMax = pose25d.at<double>(3,1)+140;
        filename = fileWaistCirc;
        break;
    case BODY_HIP:
        xMin = pose25d.at<double>(21,0)+35; //10 //pose25d.at<double>(3,0) = hipL
        xMax = pose25d.at<double>(20,0)-205; //50 //pose25d.at<double>(2,0) = hipR
        yMin = pose25d.at<double>(0,1)-15;
        yMax = pose25d.at<double>(0,1)+15;
        filename = fileHipCirc;
        break;
    default:
        break;
    }

    FileStorage fsCirc;
    fsCirc.open(filename, FileStorage::WRITE);
    if(fsCirc.isOpened() != true) return -1;
    fsCirc << "points" << "{";

    double circVal = 0, minZ = 0;
    bool firstI = true;
    int circCnt = 0;
    vector<Point2f> circXZ;
    //Filter and place all body section feature points into a vector
    for(int i = 0; i < (int)points.size(); i++){
        Matx31d pointPos((double)points.at(i).x, (double)points.at(i).y, focal);
        if(pointPos(1) > yMin && pointPos(1) < yMax && pointPos(0) > xMin && pointPos(0) < xMax){
            if(firstI){
                firstI = false;
                minZ = realPos3D[i](2);
            }

            fsCirc << circPointsName << realPos3D[i];
            circXZ.push_back(Point2f(realPos3D[i](0), realPos3D[i](2)));
            circCnt++;
        }
    }
    fsCirc << "}";
    fsCirc.release();

    //Sort body section feature points along the x-axis and compute the circumference by following along the points
    sort(circXZ.begin(), circXZ.end(), comp);
    for(int i = 0; i < (int)circXZ.size()-1; i++){
        double dx = circXZ.at(i+1).x - circXZ.at(i).x;
        double dz = (circXZ.at(i+1).y-minZ) - (circXZ.at(i).y-minZ);
        circVal += sqrt(pow(dx, 2) + pow(dz, 2));
    }

    return circVal;
}

//Builds skeleton according to input bodypose .json file
void buildSkeleton(Mat maskSkeleton, Mat pose25d){
    const int lineSz = 150;
    //Ear to top of head ~120mm
    line(maskSkeleton, Point(pose25d.row(0)), Point(pose25d.row(1)), white, lineSz);    //Pelvis - HipL
    line(maskSkeleton, Point(pose25d.row(0)), Point(pose25d.row(2)), white, lineSz);    //Pelvis - HipR
    line(maskSkeleton, Point(pose25d.row(0)), Point(pose25d.row(3)), white, lineSz+50);    //Pelvis - Torso
    line(maskSkeleton, Point(pose25d.row(1)), Point(pose25d.row(20)), white, lineSz-50);   //HipL - ShoulderL
    line(maskSkeleton, Point(pose25d.row(2)), Point(pose25d.row(21)), white, lineSz-50);   //HipR - ShoulderR
    line(maskSkeleton, Point(pose25d.row(20)), Point(pose25d.row(6)), white, lineSz-50);   //ShoulderL - Neck
    line(maskSkeleton, Point(pose25d.row(21)), Point(pose25d.row(6)), white, lineSz-50);   //ShoulderR - Neck
    line(maskSkeleton, Point(pose25d.row(3)), Point(pose25d.row(6)), white, lineSz+20);    //Torso - Neck
    line(maskSkeleton, Point(pose25d.row(6)), Point(pose25d.row(15)), white, lineSz-100);   //Neck - Nose
    line(maskSkeleton, Point(pose25d.row(15)), Point(pose25d.row(15))-Point(0,100), white, lineSz);   //Nose - TopHead
    line(maskSkeleton, Point(pose25d.row(15)), Point(pose25d.row(16)), white, lineSz);  //Nose - EyeL
    line(maskSkeleton, Point(pose25d.row(15)), Point(pose25d.row(17)), white, lineSz);  //Nose - EyeR
    line(maskSkeleton, Point(pose25d.row(16)), Point(pose25d.row(18)), white, lineSz);  //EyeL - EarL
    line(maskSkeleton, Point(pose25d.row(17)), Point(pose25d.row(19)), white, lineSz);  //EyeR - EarR
    line(maskSkeleton, Point(pose25d.row(19)), Point(pose25d.row(19))-Point(50,75), white, lineSz);   //EarR - TopHead
    line(maskSkeleton, Point(pose25d.row(19))-Point(50,75), Point(pose25d.row(15))-Point(0,100), white, lineSz);   //TopHeadEar - TopHeadNose
//    line(maskSkeleton, Point(pose25d.row(1)), Point(pose25d.row(4)), white, lineSz);    //HipL - KneeL
//    line(maskSkeleton, Point(pose25d.row(2)), Point(pose25d.row(5)), white, lineSz);    //HipR - KneeR
    line(maskSkeleton, Point(pose25d.row(2)), Point(pose25d.row(4))-Point(50,0), white, lineSz+10);    //HipR - KneeL
    line(maskSkeleton, Point(pose25d.row(2)), Point(pose25d.row(2))+Point(-50,100), white, lineSz+50);    //HipR - Butt
    line(maskSkeleton, Point(pose25d.row(21))+Point(0,50), Point(pose25d.row(2))+Point(-50,100), white, lineSz+10);   //ShoulderR - Butt
//    line(maskSkeleton, Point(pose25d.row(4)), Point(pose25d.row(7)), white, lineSz);    //KneeL - AnkleL
//    line(maskSkeleton, Point(pose25d.row(5)), Point(pose25d.row(8)), white, lineSz+10);    //KneeR - AnkleR
    line(maskSkeleton, Point(pose25d.row(4))-Point(50,0), Point(pose25d.row(8)), white, lineSz+10);    //KneeL - AnkleR
    line(maskSkeleton, Point(pose25d.row(8)), Point(pose25d.row(10))+Point(50,100), white, lineSz-100);   //AnkleR - BigToeR
    line(maskSkeleton, Point(pose25d.row(8)), Point(pose25d.row(12))+Point(50,50), white, lineSz-100);   //AnkleR - LilToeR
//    line(maskSkeleton, Point(pose25d.row(10))+Point(50,100), Point(pose25d.row(12))+Point(50,50), white, lineSz-100);  //BigToeR - LilToeR
    line(maskSkeleton, Point(pose25d.row(14)), Point(pose25d.row(12))+Point(50,50), white, lineSz-50);  //HeelR - LilToeR
    line(maskSkeleton, Point(pose25d.row(8)), Point(pose25d.row(14)), white, lineSz-50);   //AnkleR - HeelR
//    line(maskSkeleton, Point(pose25d.row(7)), Point(pose25d.row(9)), white, lineSz-50);    //AnkleL - BigToeL
//    line(maskSkeleton, Point(pose25d.row(9)), Point(pose25d.row(11)), white, lineSz-50);   //BigToeL - LilToeL
//    line(maskSkeleton, Point(pose25d.row(7)), Point(pose25d.row(11)), white, lineSz-50);   //AnkleL - LilToeL
//    line(maskSkeleton, Point(pose25d.row(7)), Point(pose25d.row(13)), white, lineSz-50);   //AnkleL - HeelL
//    line(maskSkeleton, Point(pose25d.row(20)), Point(pose25d.row(22)), white, lineSz-50);  //ShoulderL - ElbowL
//    line(maskSkeleton, Point(pose25d.row(21)), Point(pose25d.row(23)), white, lineSz-60);  //ShoulderR - ElbowR
//    line(maskSkeleton, Point(pose25d.row(22)), Point(pose25d.row(24)), white, lineSz-50);  //ElbowL - WristL
//    line(maskSkeleton, Point(pose25d.row(23)), Point(pose25d.row(25)), white, lineSz-50);  //ElbowR - WristR
}

//FAST feature detection
void featureDetection(Mat img_1, vector<Point2f>& points1){
  vector<KeyPoint> keypoints_1;
  int fast_threshold = 3;
  bool nonmaxSuppression = true;
  FAST(img_1, keypoints_1, fast_threshold, nonmaxSuppression);
  KeyPoint::convert(keypoints_1, points1, vector<int>());

  cout << "Detection: Finished detection" << endl;
}

//KLT tracker
void featureTracking(Mat img_1, Mat img_2, vector<Point2f>& points1, vector<Point2f>& points2, vector<uchar>& status){
    //this function automatically gets rid of points for which tracking fails
    vector<float> err;
    Size winSize = Size(71,126); //85,85 / 101,101
    cout << "Tracking: Finding term criteria" << endl;
    TermCriteria termcrit = TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 60, 0.0001); //50, 0.001

    cout << "Tracking: Calculating optical flow" << endl;
    calcOpticalFlowPyrLK(img_1, img_2, points1, points2, status, err, winSize, 3, termcrit, 0, 0.0001);

    //getting rid of points for which the KLT tracking failed or those who have gone outside the frame
    cout << "Tracking: Correcting" << endl;
    int indexCorrection = 0;
    for(int i = 0; i < (int)status.size(); i++){
        Point2f pt = points2.at(i - indexCorrection);
        if ((status.at(i) == 0)||(pt.x<0)||(pt.y<0)){
            if(pt.x<0 || pt.y<0) status.at(i) = 0;
            points1.erase(points1.begin() + i - indexCorrection);
            points2.erase(points2.begin() + i - indexCorrection);
            indexCorrection++;
        }
    }

    cout << "Tracking: Finished tracking" << endl;
}

//Depth extrapolation
void ClosestApproach(Vector3D P, Vector3D Q, float& PCoeff, float& QCoeff){
    Matx31d w = P.Org - Q.Org;

    float a = DotProd(P.Scl, P.Scl);
    float b = DotProd(P.Scl, Q.Scl);
    float c = DotProd(Q.Scl, Q.Scl);
    float d = DotProd(P.Scl, w);
    float e = DotProd(Q.Scl, w);

    if((a*c-b*b) != 0){
        PCoeff = (b*e-c*d)/(a*c-b*b);
        QCoeff = (a*e-b*d)/(a*c-b*b);
    }else{
        PCoeff = 0;
        QCoeff = 0;
    }
}

int main(){
    //Create windows for visualization of feature tracking
//    namedWindow("Image", WINDOW_KEEPRATIO);
//    resizeWindow("Image", winSize);
    namedWindow("Image2", WINDOW_KEEPRATIO);
    resizeWindow("Image2", winSize);
    namedWindow("Image3", WINDOW_KEEPRATIO);
    resizeWindow("Image3", winSize);
    namedWindow("Subject Features", WINDOW_KEEPRATIO);
    resizeWindow("Subject Features", winSize);

    //Grab camera matrix and distortion coefficients from camera calibration file
    FileStorage fs;
    fs.open(calib_file, FileStorage::READ);
    if(!fs.isOpened()){
        printf("Failed to open file %s\n", calib_file.c_str());
        return -1;
    }
    fs["camera_matrix"] >> cameraMatrix;
    fs["distortion_coefficients"] >> distCoeffs;
    fs.release();

    //Lens correction
    double focal = cameraMatrix.at<double>(0,0);                                        //Focal length
    Point2d pp = Point2d(cameraMatrix.at<double>(0,2), cameraMatrix.at<double>(1,2));   //Location of the focal point on the z-plane
    initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imgSize, 1, imgSize, 0), imgSize, CV_16SC2, map1, map2);

    //Grab video source and take a frame
    int numImgs = capture.get(CAP_PROP_FRAME_COUNT);
    int numFrames = 1;
    Mat imgRect[numImgs], gray[numImgs], frame;
    vector<Point2f> points[numImgs]; //Array of vectors containing all points (features) within an image/frame
    imgInput = false;

    //Attempt to open video file, if impossible, use image set
    try{
        capture.open(fileVid);
    }catch(exception& e){
        cout << "No video input, switching to image input..." << endl;
        imgInput = true;
        numImgs = 2;    //Edit this to specify the number of images you want to look at
    }

    //Rectify and grayscale images
    if(!imgInput){
        capture.set(CAP_PROP_POS_FRAMES, startFrame-1);
        cout << "Video frame correction" << endl;
        for(int currentFrame = startFrame; currentFrame <= numImgs; currentFrame += skipFrame){
            capture.set(CAP_PROP_POS_FRAMES, currentFrame-1);
            capture >> frame;
            if(!capture.read(frame)){
                cout << "\rFrame " << currentFrame << " empty, skipping..." << endl;
            }else{
                cout << "\rCorrecting frame " << currentFrame;
                rotate(frame, frame, ROTATE_90_CLOCKWISE);
                remap(frame, imgRect[numFrames-1], map1, map2, INTER_LINEAR);
                cvtColor(imgRect[numFrames-1], gray[numFrames-1], COLOR_BGR2GRAY);
                numFrames++;
            }
        }
        numImgs = numFrames;
    }else if(!img.empty()){
        cout << "Image correction" << endl;
        for(int currentImg = 1; currentImg <= numImgs; currentImg++){
            cout << "Correcting image " << currentImg << endl;

            ostringstream imgName;
            imgName << "photos/tom_full" << currentImg << ".jpg";
            img = imread(imgName.str());

            remap(img, imgRect[currentImg-1], map1, map2, INTER_LINEAR);
            cvtColor(imgRect[currentImg-1], gray[currentImg-1], COLOR_BGR2GRAY);
        }
    }else{
        cout << "Cannot open image file, please check the file path and try again" << endl;
        return -1;
    }

    //Try to open machine learning output
    fs.open(fileML, FileStorage::READ);
    Mat pose25d = Mat::zeros(34,2,CV_64F), pose3d = Mat::zeros(34,2,CV_64F);;
    if(!fs.isOpened()){
        printf("\nUnable to open machine learning output file %s, check file path or contents \n", fileML.c_str());
        return -1;
    }else{
        cout << "\nMachine learning file opened" << endl;

        FileNode nodeBatch = fs["batches"]["objects"], node25d = nodeBatch["pose25d"], node3d = nodeBatch["pose3d"];
        if (node25d.type() != FileNode::SEQ){
            cerr << "pose25d is not a sequence! Check the machine learning output .json before reattempting..." << endl;
            return 1;
        }
        if (node3d.type() != FileNode::SEQ){
            cerr << "pose3d is not a sequence! Check the machine learning output .json before reattempting..." << endl;
            return 1;
        }

        FileNodeIterator nodeIt25d = node25d.begin(), nodeIt3d = node3d.begin(); // Go through the node
        for(int x = 0; x < 34; x++){
            for(int y = 0; y < 4; y++){
                if(y < 2){
                    pose25d.at<double>(x,y) = (double)*nodeIt25d;
                    pose3d.at<double>(x,y) = (double)*nodeIt3d;
                }
                nodeIt25d++;
                nodeIt3d++;
            }
        }

        fs.releaseAndGetString();
    }

    //Try to open points.xml
    FileStorage fs3D;
    try{
        fs.open(filePoints, FileStorage::READ);
    }catch(exception& e){
        cout << "\nWarning, file is empty. Writing to .xml now, this may take a while..." << endl;
        setWrite = true;
        fs.open(filePoints, FileStorage::WRITE);
    }

    //Main loop processing and finding pointclouds (all points between all frames)
    for(int currentFrame = 1; currentFrame < numImgs-1; currentFrame++){
        cout << "\nFrames " << currentFrame << " to " << currentFrame+1 << ": " << endl;

        //XML section names
        ostringstream pointsNameF1, pointsNameF2, pointsNameF2_real;
        pointsNameF1 << "points" << currentFrame ;
        pointsNameF2 << "points_frame" << currentFrame+1;
        pointsNameF2_real << "points" << currentFrame+1;

        //Write to or read from xml files depending on whether features have been found + tracked
        if(setWrite){
            cout << "\nDetecting and tracking features on images " << currentFrame << " and " << currentFrame+1 << endl;

            featureDetection(gray[currentFrame-1], points[currentFrame-1]);
            featureDetection(gray[currentFrame], points[currentFrame]);

            cout << "Features in Image 1: " << points[currentFrame-1].size() << endl << "Features in Image 2: " << points[currentFrame].size() << endl;

            featureTracking(gray[currentFrame-1], gray[currentFrame], points[currentFrame-1], points[currentFrame], status);

            fs << pointsNameF1.str() << points[currentFrame-1];
            fs << pointsNameF2.str() << points[currentFrame];
        }else{
            fs[pointsNameF1.str()] >> points[currentFrame-1];
            fs[pointsNameF2.str()] >> points[currentFrame];
        }

        //Draw lines across frames to visualize feature tracking
        Mat hImgRect;
        hconcat(imgRect[currentFrame-1], imgRect[currentFrame], hImgRect);
        for(int j = 0; j < (int)points[currentFrame-1].size(); j++)
            line(hImgRect, points[currentFrame-1].at(j), points[currentFrame].at(j)+Point2f(imgRect[currentFrame-1].size().width, 0), randColor());
//        while(waitKey(10) != 'x') imshow("Image", hImgRect);

        //Draw mask with bodypose skeleton - use line function to follow points
        namedWindow("Skeleton Mask", WINDOW_KEEPRATIO);
        resizeWindow("Skeleton Mask", winSize);
        Mat maskSkeleton(imgSize.height, imgSize.width, CV_8UC3, black);
        buildSkeleton(maskSkeleton, pose25d);
        imshow("Skeleton Mask", maskSkeleton);

        //Filter out points that stray too far from skeleton (non-subject features)
        int indexCorrection = 0;
        cout << "Matching features to bodypose skeleton. Please wait, this may take a while..." << endl << endl << "0%";
        for(int y = 0; y < (int)maskSkeleton.size().height; y++){
            for(int x = 0; x < (int)maskSkeleton.size().width; x++){
                Vec3b pt = maskSkeleton.at<Vec3b>(y,x);
                vector<Point2f>::iterator itr = find(points[currentFrame-1].begin(), points[currentFrame-1].end(), Point2f(x,y));

                if(pt == Vec3b::zeros() && itr != points[currentFrame-1].end()){
                    indexCorrection = distance(points[currentFrame-1].begin(), itr);
                    points[currentFrame-1].erase(points[currentFrame-1].begin() + indexCorrection);
                    points[currentFrame].erase(points[currentFrame].begin() + indexCorrection);
                }
            }
            cout << "\r" << (100 * (1080*(y+1))/(int)maskSkeleton.size().area()) << "%";
        }
        cout << "\r" << "100%" << endl;

        //Find essential matrix
        Mat mask, R, t;
        Mat E = findEssentialMat(points[currentFrame-1], points[currentFrame], focal, pp, RANSAC, 0.999, 2.0, mask);
        E.convertTo(E, CV_32S, 100, 0.5);
        E.convertTo(E, CV_64F, 0.01);
        cout << "\nmask elements: " << (int)mask.size().height << endl;

        //Filter out mathematically impossible feature pairs (according to essential matrix)
        indexCorrection = 0;
        for(int j = 0; j < (int)mask.size().height; j++){
            Point2f pt1 = points[currentFrame].at(j - indexCorrection), pt2 = points[currentFrame-1].at(j - indexCorrection);
            if (mask.at<int>(j,0) == 0 || pt1.x<0 || pt1.y<0 || abs(pt1.x-pt2.x)>500 || abs(pt1.y-pt2.y)>500){
                points[currentFrame-1].erase(points[currentFrame-1].begin() + j - indexCorrection);
                points[currentFrame].erase(points[currentFrame].begin() + j - indexCorrection);
                indexCorrection++;
            }
        }
        cout << "Points after correction: " << (int)points[currentFrame-1].size() << endl;

        //Draw lines across frames to visualize feature tracking of filtered points
        Mat hImgRect2, hImgRect3;
        hconcat(imgRect[currentFrame-1], imgRect[currentFrame], hImgRect2);
        hconcat(imgRect[currentFrame-1], imgRect[currentFrame], hImgRect3);
        for(int j = 0; j < (int)points[currentFrame-1].size(); j++){
            line(hImgRect2, points[currentFrame-1].at(j), points[currentFrame].at(j)+Point2f(imgSize.width, 0), randColor());
            circle(hImgRect3, points[currentFrame-1].at(j), 5, Scalar(0,0,255), -1);
            circle(hImgRect3, points[currentFrame].at(j)+Point2f(imgSize.width, 0), 5, Scalar(0,0,255), -1);

            //HEAD
            if(((int)points[currentFrame-1].at(j).x>=280 && (int)points[currentFrame-1].at(j).x<=505)
                    && ((int)points[currentFrame-1].at(j).y>=95 && (int)points[currentFrame-1].at(j).y<=185))
            {
                circle(hImgRect3, points[currentFrame-1].at(j), 5, Scalar(255,0,0), -1);
                circle(hImgRect3, points[currentFrame].at(j)+Point2f(imgSize.width, 0), 5, Scalar(255,0,0), -1);
                cout << "Head feature number " << j << endl << points[currentFrame-1].at(j).x << ", " << points[currentFrame-1].at(j).y << endl;
                cout << points[currentFrame].at(j).x << ", " << points[currentFrame].at(j).y << endl;
            }
            //FEET
            else if(((int)points[currentFrame-1].at(j).x>=300 && (int)points[currentFrame-1].at(j).x<=600)
                    && ((int)points[currentFrame-1].at(j).y>=1690 && (int)points[currentFrame-1].at(j).y<=1870))
            {
                circle(hImgRect3, points[currentFrame-1].at(j), 5, Scalar(255,0,0), -1);
                circle(hImgRect3, points[currentFrame].at(j)+Point2f(imgSize.width, 0), 5, Scalar(255,0,0), -1);
                cout << "Feet feature number " << j << endl << points[currentFrame-1].at(j).x << ", " << points[currentFrame-1].at(j).y << endl;
                cout << points[currentFrame].at(j).x << ", " << points[currentFrame].at(j).y << endl;
            }
        }
        while(waitKey(10) != 'x'){
            imshow("Image2", hImgRect2);
            imshow("Subject Features", hImgRect3);
        }

        //Extract the rotation and translation matrices from the essential matrix
        recoverPose(E, points[currentFrame], points[currentFrame-1], R, t, focal, pp);
        R.convertTo(R, CV_32S, 100, 0.5);
        R.convertTo(R, CV_64F, 0.01);
        cout << "Essential matrix: " << endl << E << endl << "Rotation: " << endl << R << endl << "Translation: " << endl << t << endl;

        //Draw viable points on frames
        Mat hImgRect4;
        hconcat(imgRect[currentFrame-1], imgRect[currentFrame], hImgRect4);

        //Find scalar for real position and depth (position in 3D space of a feature)
        if(currentFrame == 1){
            int pointNum = 14;   //HEAD
            int pointNumP2 = 8829;    //FEET 10527

            line(hImgRect4, points[currentFrame-1].at(pointNum), points[currentFrame-1].at(pointNumP2), white);
            circle(hImgRect4, points[currentFrame-1].at(pointNum), 5, white, -1);
            circle(hImgRect4, points[currentFrame-1].at(pointNumP2), 5, white, -1);

            float LeftCoeffP1, RightCoeffP1, LeftCoeffP2, RightCoeffP2;
            Vector3D point1F1Pos, point1F2Pos, point2F1Pos, point2F2Pos;
            point1F1Pos.Org = fp;
            point1F1Pos.Scl = Matx31d((double)points[currentFrame-1].at(pointNum).x - pp.x, (double)points[currentFrame-1].at(pointNum).y - pp.y, focal);
            point1F2Pos.Org = HRotTrans(R, t, fp);
            point1F2Pos.Scl = HRotTrans(R, t, Matx31d((double)points[currentFrame].at(pointNum).x - pp.x, (double)points[currentFrame].at(pointNum).y - pp.y, focal));

            point2F1Pos.Org = fp;
            point2F1Pos.Scl = Matx31d((double)points[currentFrame-1].at(pointNumP2).x - pp.x, (double)points[currentFrame-1].at(pointNumP2).y - pp.y, focal);
            point2F2Pos.Org = HRotTrans(R, t, fp);
            point2F2Pos.Scl = HRotTrans(R, t, Matx31d((double)points[currentFrame].at(pointNumP2).x - pp.x, (double)points[currentFrame].at(pointNumP2).y - pp.y, focal));

            ClosestApproach(point1F2Pos, point1F1Pos, RightCoeffP1, LeftCoeffP1);
            Matx31d point1Pos3D = (point1F2Pos.Org + point1F2Pos.Scl * RightCoeffP1 + point1F1Pos.Org + point1F1Pos.Scl * LeftCoeffP1) * 0.5;
            ClosestApproach(point2F2Pos, point2F1Pos, RightCoeffP2, LeftCoeffP2);
            Matx31d point2Pos3D = (point2F2Pos.Org + point2F2Pos.Scl * RightCoeffP2 + point2F1Pos.Org + point2F1Pos.Scl * LeftCoeffP2) * 0.5;

            //Calculation of vector length between two points
            double dx = point2Pos3D(0)-point1Pos3D(0);
            double dy = point2Pos3D(1)-point1Pos3D(1);
            double dz = point2Pos3D(2)-point1Pos3D(2);
            double l1 = sqrt(pow(dx, 2) + pow(dy, 2));
            double lengthP1P2 = sqrt(pow(l1, 2) + pow(dz, 2));
            imgScalar = knownUnit / lengthP1P2;

            cout << "\nPOINT 1:" << endl;
            cout << "F1 Org (focal point): " << endl << point1F1Pos.Org << endl << "F1 Scl (feature point): " << endl << point1F1Pos.Scl << endl;
            cout << "F2 Org (focal point): " << endl << point1F2Pos.Org << endl << "F2 Scl (feature point): " << endl << point1F2Pos.Scl << endl;
            cout << "Right coefficient = " << RightCoeffP1 << endl << "Left coefficient = " << LeftCoeffP1 << endl;
            cout << "Feature position in 3D space: " << endl << point1Pos3D << endl;
            cout << "\nPOINT 2:" << endl;
            cout << "F1 Org (focal point): " << endl << point2F1Pos.Org << endl << "F1 Scl (feature point): " << endl << point2F1Pos.Scl << endl;
            cout << "F2 Org (focal point): " << endl << point2F2Pos.Org << endl << "F2 Scl (feature point): " << endl << point2F2Pos.Scl << endl;
            cout << "Right coefficient = " << RightCoeffP2 << endl << "Left coefficient = " << LeftCoeffP2 << endl;
            cout << "Feature position in 3D space: " << endl << point2Pos3D << endl;
            cout << "\nDistance between points = " << lengthP1P2 << endl << "Image Scalar = " << imgScalar << endl;
            cout << "\nPoint 1 real position in 3D space: " << endl << point1Pos3D*imgScalar << endl;
            cout << "Point 2 real position in 3D space: " << endl << point2Pos3D*imgScalar << endl << endl;
        }

        //Setup file output for real space points`
        fs3D.open(file3D, FileStorage::WRITE);
        ostringstream frames3D;
        frames3D << "frames_" << currentFrame << "_to_" << currentFrame +1;

        //Calculate the real-space position of each individual point
        indexCorrection = 0;
        vector<Matx31d> realPos3D;
        for (int j = 0; j < (int)points[currentFrame-1].size()-1; j++){
            float LeftCoeff, RightCoeff;
            Vector3D pointF1Pos, pointF2Pos;
            pointF1Pos.Org = fp;
            pointF1Pos.Scl = Matx31d((double)points[currentFrame-1].at(j).x - pp.x, (double)points[currentFrame-1].at(j).y - pp.y, focal);
            pointF2Pos.Org = HRotTrans(R, t, fp);
            pointF2Pos.Scl = HRotTrans(R, t, Matx31d((double)points[currentFrame].at(j).x - pp.x, (double)points[currentFrame].at(j).y - pp.y, focal));

            ClosestApproach(pointF2Pos, pointF1Pos, RightCoeff, LeftCoeff);
            Matx31d pointPos3D = (pointF2Pos.Org + pointF2Pos.Scl * RightCoeff + pointF1Pos.Org + pointF1Pos.Scl * LeftCoeff) * 0.5;
            Matx31d realPos3Dtmp = pointPos3D*imgScalar;

            realPos3Dtmp(2) += 200;

            //Filter out irrelevant/impossible points
            if(realPos3Dtmp(2) < double(1800.00) || realPos3Dtmp(2) > double(2200.00)){
                points[currentFrame-1].erase(points[currentFrame-1].begin() + j - indexCorrection);
                points[currentFrame].erase(points[currentFrame].begin() + j - indexCorrection);
                indexCorrection++;
            }else if(realPos3Dtmp(2) < double(1900.00)){
                Scalar BGR = Scalar(255,0,0);
                line(hImgRect4, points[currentFrame-1].at(j), points[currentFrame].at(j)+Point2f(imgSize.width, 0), randColor());
                circle(hImgRect4, points[currentFrame-1].at(j), 5, BGR, -1);
                circle(hImgRect4, points[currentFrame].at(j)+Point2f(imgSize.width, 0), 5, BGR, -1);
                fs3D << frames3D.str() << realPos3Dtmp;
                realPos3D.push_back(realPos3Dtmp);
            }else if(realPos3Dtmp(2) > double(2100.00)){
                Scalar BGR = Scalar(0,0,255);
                line(hImgRect4, points[currentFrame-1].at(j), points[currentFrame].at(j)+Point2f(imgSize.width, 0), randColor());
                circle(hImgRect4, points[currentFrame-1].at(j), 5, BGR, -1);
                circle(hImgRect4, points[currentFrame].at(j)+Point2f(imgSize.width, 0), 5, BGR, -1);
                fs3D << frames3D.str() << realPos3Dtmp;
                realPos3D.push_back(realPos3Dtmp);
            }
            else{
                Scalar BGR = Scalar(0,255,0);
                line(hImgRect4, points[currentFrame-1].at(j), points[currentFrame].at(j)+Point2f(imgSize.width, 0), randColor());
                circle(hImgRect4, points[currentFrame-1].at(j), 5, BGR, -1);
                circle(hImgRect4, points[currentFrame].at(j)+Point2f(imgSize.width, 0), 5, BGR, -1);
                fs3D << frames3D.str() << realPos3Dtmp;
                realPos3D.push_back(realPos3Dtmp);
            }
        }

        //Filter real space array and remove all irrelevant/impossible points
//        indexCorrection = 0;
//        for (int j = 0; j < (int)realPos3D.size(); j++) {
//            if(realPos3D[j](2) < double(1900.00) || realPos3D[j](2) > double(2100.00) /*|| (int)abs(realPos3D[j](1)-realPos3D[j-1](1)) > double(500.00)*/){
//                realPos3D.erase(realPos3D.begin() + j - indexCorrection);
//                indexCorrection++;
//            }
//        }
        while(waitKey(10) != 'x') imshow("Image3", hImgRect4);
        fs3D.release();

        //Calculate circumferences and output into files/console
        for(int bodySection = BODY_NECK; bodySection <= BODY_HIP; bodySection++){
            double circValTest = findCirc(pose25d, realPos3D, points[currentFrame-1], focal, (bodySections)bodySection);
            cout << bodySection << " circumference: " << circValTest << " mm / " << circValTest/10 << " cm" << endl;
        }
        break;
    }
    cout << "Process complete, press x on an image to exit..." << endl;
    while(waitKey(10) != 'x');
    fs.release();
}
