#include </home/pablo/opencv/include/opencv2/opencv.hpp>

using namespace cv;
using namespace std;

//3D vector holding coordinates for origin and point location in image
struct Vector3D{
    Matx31d Org = Matx31d(0,0,0);
    Matx31d Scl = Matx31d(0,0,0);
};

const string file3D = "points3D.xyz";
const string fileML = "bodypose_3dbp_tomf.json";
// /opt/nvidia/deepstream/deepstream-6.2/sources/apps/sample_apps/deepstream-bodypose-3d/streams/
const string fileVid = "./photos/tom_front.mp4";
const string filePoints = "points.xml";                     //Output file for feature location within image space
const string fileERT = "ERT.xml";                           //Output file for essential, rotation, and translation matrices
const string calib_file = "calibration_output_vid.xml";     //Camera calibration file for video at full resolution (16:9) of Samsung Galaxy S20 FE 5G camera
//const string calib_file = "calibration_output.xml";         //Camera calibration file for images at full resolution (4:3) of Samsung Galaxy S20 FE 5G camera
//const string calib_file = "calibration_output_fourth.xml";  //Camera calibration file for images at 1/4th resolution (4:3) of Samsung Galaxy S20 FE 5G camera
const double knownUnit = 1900;                              //Known distance between two points in mm --> 12.5 cm for the mouse / 190 cm for Tom height / 24.5 cm for James chest to neck distance
double imgScalar;                                           //Scalar used to set point coordinates in real space
bool setWrite = false;
bool imgInput = true;

Mat cameraMatrix, map1, map2;               //Matrices containing the camera matrix, distortion correction maps, rectified images, and grayscale rectified images
Mat distCoeffs = Mat::zeros(8, 1, CV_64F);  //Matrix containing the distortion coefficients (empty in case camera calibration file cannot be opened)
Mat img = imread("photos/tom_front1.jpg");   //First photo/frame in group/video (used to set size throughout program)
VideoCapture capture(fileVid);              //Retrieve video input

const Matx31d fp = Matx31d(0, 0, 0); //Focal point is on z-plane to keep focal length positive

vector<uchar> status;   //Vector containing point and corresponding filter status for tracking

//Size imgSize = img.size(); //Size of images/frames used throughout program
Size imgSize = Size(capture.get(CAP_PROP_FRAME_HEIGHT), capture.get(CAP_PROP_FRAME_WIDTH));
//Size winSize = Size(img.size().width/2, img.size().height/4);
Size winSize = imgSize;

//FAST feature detection
void featureDetection(Mat img_1, vector<Point2f>& points1){
  vector<KeyPoint> keypoints_1;
  int fast_threshold = 10;
  bool nonmaxSuppression = true;
  FAST(img_1, keypoints_1, fast_threshold, nonmaxSuppression);
  KeyPoint::convert(keypoints_1, points1, vector<int>());

  cout << "Detection: Finished detection" << endl;
}

//KLT tracker
void featureTracking(Mat img_1, Mat img_2, vector<Point2f>& points1, vector<Point2f>& points2, vector<uchar>& status){
    //this function automatically gets rid of points for which tracking fails
    vector<float> err;
    Size winSize = Size(25,25); //85,85
    cout << "Tracking: Finding term criteria" << endl;
    TermCriteria termcrit = TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, 0.01);

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

    //Create windows for visualization of feature tracking
    namedWindow("Image", WINDOW_KEEPRATIO);
    resizeWindow("Image", winSize);
    namedWindow("Image2", WINDOW_KEEPRATIO);
    resizeWindow("Image2", winSize);
    namedWindow("Image3", WINDOW_KEEPRATIO);
    resizeWindow("Image3", winSize);
    namedWindow("Subject Features", WINDOW_KEEPRATIO);
    resizeWindow("Subject Features", winSize);

    //Grab video source and take a frame
    int numImgs = capture.get(CAP_PROP_FRAME_COUNT);
    int numFrames = 1;
    Mat imgRect[numImgs], gray[numImgs], frame;
    vector<Point2f> points[numImgs]; //Array of vectors containing all points (features) within an image/frame
    imgInput = false;

    try{
        capture.open(fileVid);
    }catch(exception& e){
        cout << "No video input, switching to image input..." << endl;
        imgInput = true;
        numImgs = 2;
    }

    //Rectify and grayscale images
    if(!imgInput){
        int i = 1;
        int skipF = 0;
        int skipFCnt = 0;
        cout << "Video frame correction" << endl;
        for(;;){
            if(i >= numImgs-skipFCnt) break;
            capture >> frame;
            if(!capture.read(frame)){
                cout << "\rFrame " << i << " empty, skipping..." << endl;
                i++;
                continue;
            }

            if(skipF > 0 && skipF < 20){
                cout << "Skipping frame " << skipF << endl;
                skipFCnt++;
                skipF++;
                continue;
            }else{
                cout << "\rCorrecting frame " << i << endl;
                rotate(frame, frame, ROTATE_90_CLOCKWISE);
                remap(frame, imgRect[i-1], map1, map2, INTER_LINEAR);
                cvtColor(imgRect[i-1], gray[i-1], COLOR_BGR2GRAY);
                skipF = 1;
                numFrames++;
                i++;
            }
        }
    }else{
        cout << "Image correction" << endl;
        for(int i = 1; i <= numImgs; i++){
            cout << "Correcting image " << i << endl;

            ostringstream imgName;
            imgName << "photos/tom_full" << i << ".jpg";
            img = imread(imgName.str());

            remap(img, imgRect[i-1], map1, map2, INTER_LINEAR);
            cvtColor(imgRect[i-1], gray[i-1], COLOR_BGR2GRAY);
        }
    }

    //Try to open machine learning output
    fs.open(fileML, FileStorage::READ);
    if(!fs.isOpened()){
        printf("\nUnable to open machine learning output file %s, check file path or contents \n", fileML.c_str());
        return -1;
    }else cout << "\nMachine learning file opened" << endl;

    FileNode nodeBatch = fs["batches"]["objects"], node25d = nodeBatch["pose25d"], node3d = nodeBatch["pose3d"];
    if (node25d.type() != FileNode::SEQ){
        cerr << "pose25d is not a sequence! FAIL" << endl;
        return 1;
    }
    if (node3d.type() != FileNode::SEQ){
        cerr << "pose3d is not a sequence! FAIL" << endl;
        return 1;
    }

    Mat pose25d = Mat::zeros(34,4,CV_64F), pose3d = Mat::zeros(34,4,CV_64F);
    FileNodeIterator nodeIt25d = node25d.begin(), nodeIt3d = node3d.begin(); // Go through the node
    for(int x = 0; x < 34; x++)
        for(int y = 0; y < 4; y++){
            pose25d.at<double>(x,y) = (double)*nodeIt25d;
            pose3d.at<double>(x,y) = (double)*nodeIt3d;
            nodeIt25d++;
            nodeIt3d++;
        }

    fs.releaseAndGetString();
    cout << "\nPose 2.5D: " << endl << pose25d << endl;
    cout << "\nPose 3D: " << endl << pose3d << endl;

    //Try to open points.xml
    FileStorage fs1(fileERT, FileStorage::WRITE);
    FileStorage fs3D;
    try{
        fs.open(filePoints, FileStorage::READ);
    }catch(exception& e){
        cout << "\nWarning, file is empty. Writing to .xml now, this may take a while (edit the float at the end of calcOpticalFlowPyrLK for performance) ..." << endl;
        setWrite = true;
        fs.open(filePoints, FileStorage::WRITE);
    }

    //Main loop processing and finding pointclouds (all points between all frames)
    numImgs = numFrames;
    for(int i = 1; i < numImgs-1; i++){
        cout << "\nFrames " << i << " to " << i+1 << ": " << endl;

        //XML section names
        ostringstream pointsNameF1, pointsNameF2, pointsNameF2_real;
        pointsNameF1 << "points" << i;
        pointsNameF2 << "points_frame" << i+1;
        pointsNameF2_real << "points" << i+1;
        ostringstream framesERT, essERT, rotERT, transERT;
        framesERT << "frames_" << i << "_" << i+1;
        essERT << framesERT.str() << "_essential";
        rotERT << framesERT.str() << "_rotation";
        transERT << framesERT.str() << "_transformation";

        //Write to or read from xml files depending on whether features have been found + tracked
        if(setWrite){
            fs3D.open(file3D, FileStorage::WRITE);
            cout << "\nDetecting and tracking features on images " << i << " and " << i+1 << endl;

            featureDetection(gray[i-1], points[i-1]);
            featureDetection(gray[i], points[i]);

            cout << "Features in Image 1: " << points[i-1].size() << endl << "Features in Image 2: " << points[i].size() << endl;

            featureTracking(gray[i-1], gray[i], points[i-1], points[i], status);

            fs << pointsNameF1.str() << points[i-1];
            fs << pointsNameF2.str() << points[i];
        }else{
            fs[pointsNameF1.str()] >> points[i-1];
            fs[pointsNameF2.str()] >> points[i];
        }

        //Draw lines across frames to visualize feature tracking
        Mat hImgRect;
        hconcat(imgRect[i-1], imgRect[i], hImgRect);
        for(int j = 0; j < (int)points[i-1].size(); j++)
            line(hImgRect, points[i-1].at(j), points[i].at(j)+Point2f(imgRect[i-1].size().width, 0), Scalar(int(rand()%255+1), int(rand()%255+1), int(rand()%255+1)));
        while(waitKey(10) != 'x') imshow("Image", hImgRect);

        //Find essential matrix
        Mat mask, R, t;
        Mat E = findEssentialMat(points[i-1], points[i], focal, pp, RANSAC, 0.999, 2.0, mask);
        E.convertTo(E, CV_32S, 100, 0.5);
        E.convertTo(E, CV_64F, 0.01);
        cout << "mask elements: " << (int)mask.size().height << endl;

        //Filter out mathematically impossible feature pairs (according to essential matrix)
        int indexCorrection = 0;
        for(int j = 0; j < (int)mask.size().height; j++){
            Point2f pt = points[i].at(j - indexCorrection);
            if ((mask.at<int>(j,0) == 0)||(pt.x<0)||(pt.y<0)
                    ||abs(points[i].at(j-indexCorrection).x-points[i-1].at(j-indexCorrection).x)>500
                    ||abs(points[i].at(j-indexCorrection).y-points[i-1].at(j-indexCorrection).y)>500)
            {
                if(pt.x<0 || pt.y<0) mask.at<int>(j,0) = 0;
                points[i-1].erase(points[i-1].begin() + j - indexCorrection);
                points[i].erase(points[i].begin() + j - indexCorrection);
                indexCorrection++;
            }
        }

        cout << "Points after correction: " << (int)points[i-1].size() << endl;

        //Draw lines across frames to visualize feature tracking of filtered points
        Mat hImgRect2, hImgRect3;
        hconcat(imgRect[i-1], imgRect[i], hImgRect2);
        hconcat(imgRect[i-1], imgRect[i], hImgRect3);
        for(int j = 0; j < (int)points[i-1].size(); j++){
            line(hImgRect2, points[i-1].at(j), points[i].at(j)+Point2f(imgSize.width, 0), Scalar(int(rand()%255+1), int(rand()%255+1), int(rand()%255+1)));
            circle(hImgRect3, points[i-1].at(j), 5, Scalar(0,0,255), -1);
            circle(hImgRect3, points[i].at(j)+Point2f(imgSize.width, 0), 5, Scalar(0,0,255), -1);

            //HEAD
            if(((int)points[i-1].at(j).x>=500 && (int)points[i-1].at(j).x<=680) && ((int)points[i-1].at(j).y>=140 && (int)points[i-1].at(j).y<=225)){
                circle(hImgRect3, points[i-1].at(j), 5, Scalar(255,0,0), -1);
                circle(hImgRect3, points[i].at(j)+Point2f(imgSize.width, 0), 5, Scalar(255,0,0), -1);
//                cout << "Head feature number " << j << endl << points[i-1].at(j).x << ", " << points[i-1].at(j).y << endl;
//                cout << points[i].at(j).x << ", " << points[i].at(j).y << endl;
            }
            //FEET
            else if(((int)points[i-1].at(j).x>=385 && (int)points[i-1].at(j).x<=705) && ((int)points[i-1].at(j).y>=1575 && (int)points[i-1].at(j).y<=1690)){
                circle(hImgRect3, points[i-1].at(j), 5, Scalar(255,0,0), -1);
                circle(hImgRect3, points[i].at(j)+Point2f(imgSize.width, 0), 5, Scalar(255,0,0), -1);
//                cout << "Feet feature number " << j << endl << points[i-1].at(j).x << ", " << points[i-1].at(j).y << endl;
//                cout << points[i].at(j).x << ", " << points[i].at(j).y << endl;
            }
        }
        while(waitKey(10) != 'x'){
            imshow("Image2", hImgRect2);
            imshow("Subject Features", hImgRect3);
        }

        recoverPose(E, points[i], points[i-1], R, t, focal, pp);
        R.convertTo(R, CV_32S, 100, 0.5);
        R.convertTo(R, CV_64F, 0.01);
        cout << "The essential matrix is: " << endl << E << endl << "Rotation is: " << endl << R << endl << "Translation is: " << endl << t << endl; //Output ess, rot, and trans matrices to console
        fs1 << framesERT.str() << "{" << essERT.str() << E << rotERT.str() << R << transERT.str() << t << "}";                                       //Output ess, rot and trans matrices to file

        //Find scalar for real position and depth (position in 3D space of a feature)
        if(i == 1){
            int pointNum = 19561;    //FEET
            int pointNumP2 = 258;   //HEAD

            float LeftCoeffP1, RightCoeffP1, LeftCoeffP2, RightCoeffP2;
            Vector3D point1F1Pos, point1F2Pos, point2F1Pos, point2F2Pos;
            point1F1Pos.Org = fp;
            point1F1Pos.Scl = Matx31d((double)points[i-1].at(pointNum).x - pp.x, (double)points[i-1].at(pointNum).y - pp.y, focal);
            point1F2Pos.Org = HRotTrans(R, t, fp);
            point1F2Pos.Scl = HRotTrans(R, t, Matx31d((double)points[i].at(pointNum).x - pp.x, (double)points[i].at(pointNum).y - pp.y, focal));

            point2F1Pos.Org = fp;
            point2F1Pos.Scl = Matx31d((double)points[i-1].at(pointNumP2).x - pp.x, (double)points[i-1].at(pointNumP2).y - pp.y, focal);
            point2F2Pos.Org = HRotTrans(R, t, fp);
            point2F2Pos.Scl = HRotTrans(R, t, Matx31d((double)points[i].at(pointNumP2).x - pp.x, (double)points[i].at(pointNumP2).y - pp.y, focal));

            ClosestApproach(point1F2Pos, point1F1Pos, RightCoeffP1, LeftCoeffP1);
            Matx31d point1Pos3D = (point1F2Pos.Org + point1F2Pos.Scl * RightCoeffP1 + point1F1Pos.Org + point1F1Pos.Scl * LeftCoeffP1) * 0.5;
            ClosestApproach(point2F2Pos, point2F1Pos, RightCoeffP2, LeftCoeffP2);
            Matx31d point2Pos3D = (point2F2Pos.Org + point2F2Pos.Scl * RightCoeffP2 + point2F1Pos.Org + point2F1Pos.Scl * LeftCoeffP2) * 0.5;

            //Tilt compensation
//            Mat vecP1P2(3,1,CV_64F);
//            Mat vecY(3,1,CV_64F);
//            vecP1P2 = Mat(point2Pos3D);
//            vecY = Mat(double(norm(point1Pos3D)) + Matx31d(0,1,0));

//            double c = vecP1P2.dot(vecY);
//            Mat v = vecP1P2.cross(vecY);
//            cout << endl << v << endl;
//            Mat vx = Mat::zeros(3,3,CV_64F);
//            vx.at<double>(0,1) = -v.at<double>(2);
//            vx.at<double>(0,2) = v.at<double>(1);
//            vx.at<double>(1,0) = v.at<double>(2);
//            vx.at<double>(1,2) = -v.at<double>(0);
//            vx.at<double>(2,0) = -v.at<double>(1);
//            vx.at<double>(2,1) = v.at<double>(0);
//            cout << vx << endl;
//            Mat R3D = Mat::eye(3,3,CV_64F) + vx + (vx.mul(vx) * 1/(1+c));
//            point1Pos3D = HRotTrans(R3D, Matx31d(0,0,0), point1Pos3D);
//            point2Pos3D = HRotTrans(R3D, Matx31d(0,0,0), point2Pos3D);

            //Calculation of vector length between two points
            double dx = point1Pos3D(0)-point2Pos3D(0);
            double dy = point1Pos3D(1)-point2Pos3D(1);
            double dz = point1Pos3D(2)-point2Pos3D(2);
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
//            cout << "3D Rot:" << endl << R3D << endl;
            cout << "\nPoint 1 real position in 3D space: " << endl << point1Pos3D*imgScalar << endl;
            cout << "Point 2 real position in 3D space: " << endl << point2Pos3D*imgScalar << endl << endl;
//            break;
        }

        Mat hImgRect4;
        hconcat(imgRect[i-1], imgRect[i], hImgRect4);
        indexCorrection = 0;
        Matx31d realPos3D[(int)points[i-1].size()];
        for (int j = 0; j < (int)points[i-1].size()-1; j++){ //(int)points[i-1].size()-1 or 362
            float LeftCoeff, RightCoeff;
            Vector3D pointF1Pos, pointF2Pos;
            pointF1Pos.Org = fp;
            pointF1Pos.Scl = Matx31d((double)points[i-1].at(j).x - pp.x, (double)points[i-1].at(j).y - pp.y, focal);
            pointF2Pos.Org = HRotTrans(R, t, fp);
            pointF2Pos.Scl = HRotTrans(R, t, Matx31d((double)points[i].at(j).x - pp.x, (double)points[i].at(j).y - pp.y, focal));

            ClosestApproach(pointF2Pos, pointF1Pos, RightCoeff, LeftCoeff);
            Matx31d pointPos3D = (pointF2Pos.Org + pointF2Pos.Scl * RightCoeff + pointF1Pos.Org + pointF1Pos.Scl * LeftCoeff) * 0.5;
            realPos3D[j] = pointPos3D*imgScalar;

            ostringstream frames3D;
            frames3D << "frames_" << i << "_to_" << i+1;
            fs3D << frames3D.str() << realPos3D[j];

            if((int)realPos3D[j](2) < 0 || (int)realPos3D[j](2) >= 2200 || (int)abs(realPos3D[j](1)-realPos3D[j-1](1)) > 500){
                points[i-1].erase(points[i-1].begin() + j - indexCorrection);
                points[i].erase(points[i].begin() + j - indexCorrection);
                indexCorrection++;
            }else if(((int)points[i-1].at(j).x>=250 && (int)points[i-1].at(j).x<=900) && ((int)points[i-1].at(j).y>=150 && (int)points[i-1].at(j).y<=1750)){
                Scalar BGR = Scalar(0,0,0);
                if(realPos3D[j](2) < 1900){
                    BGR = Scalar(0,0,255);
                    line(hImgRect4, points[i-1].at(j), points[i].at(j)+Point2f(imgSize.width, 0), BGR, 1);
                }else{
                    BGR = Scalar(0,255,0);
                    line(hImgRect4, points[i-1].at(j), points[i].at(j)+Point2f(imgSize.width, 0), Scalar(int(rand()%255+1), int(rand()%255+1), int(rand()%255+1)), 1);
                }
                circle(hImgRect4, points[i-1].at(j), 5, BGR, -1);
                circle(hImgRect4, points[i].at(j)+Point2f(imgSize.width, 0), 5, BGR, -1);

//                cout << "FEATURE PAIR " << j - indexCorrection + 1 << ": " << endl;
//                cout << "Scaled point in 3D space: " << endl << realPos3D[j] << endl;
            }
        }

        ostringstream circPointsName;
        circPointsName << "Pos_3D";
        string fileHipCirc = "hipCirc.xml";
        FileStorage fs2;
        fs2.open(fileHipCirc, FileStorage::WRITE);
        if(fs2.isOpened() != true) return -1;
        fs2 << "points" << "{";

        Matx31d circPoints[(int)points[i-1].size()-indexCorrection];
        for(int j = 0; j < (int)points[i-1].size(); j++){
            Matx31d pointPos((double)points[i-1].at(j).x - pp.x, (double)points[i-1].at(j).y - pp.y, focal);
            //Y-axis value is negative only in 3D space, find another solution
            if(double(pointPos(1)) > -pose25d.at<double>(0,1)-20 && double(pointPos(1)) < -pose25d.at<double>(0,1)+20){
                circPoints[j] = realPos3D[j];
                fs2 << circPointsName.str() << circPoints[j];
            }
        }

        fs2 << "}";
        fs2.release();

        fs3D.release();
        while(waitKey(10) != 'x') imshow("Image3", hImgRect4);
    }
    cout << "Process complete..." << endl;
    fs.release();
    fs1.release();
}
