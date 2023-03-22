#include </home/pablo/opencv/include/opencv2/opencv.hpp>

using namespace cv;
using namespace std;

struct Vector3D{
    Matx31d Org = Matx31d(0,0,0);
    Matx31d Scl = Matx31d(0,0,0);
};

const string filePoints = "points.xml";
const string fileERT = "ERT.xml";
//const string calib_file = "calibration_output.xml";
const string calib_file = "camera_calibration_fourth.xml";
const int userHeight = 1800; //This is in mm -> 180 cm
const int numImgs = 2;

Mat img = imread("photos/mouse1.jpg");
Mat cameraMatrix, map1, map2;
Mat imgRect[numImgs];
Mat gray[numImgs];
Mat distCoeffs = Mat::zeros(8, 1, CV_64F);

const Matx31d fp = Matx31d(0, 0, 0); //Focal point is on z-plane to keep focal length positive

vector<Point2f> points[numImgs];
vector<uchar> status;

Size imgSize = img.size();

//FAST feature detection
void featureDetection(Mat img_1, vector<Point2f>& points1){
  vector<KeyPoint> keypoints_1;
  int fast_threshold = 20;
  bool nonmaxSuppression = true;
  FAST(img_1, keypoints_1, fast_threshold, nonmaxSuppression);
  KeyPoint::convert(keypoints_1, points1, vector<int>());

  cout << "Detection: Finished detection" << endl;
}

//KLT tracker
void featureTracking(Mat img_1, Mat img_2, vector<Point2f>& points1, vector<Point2f>& points2, vector<uchar>& status){
    //this function automatically gets rid of points for which tracking fails
    vector<float> err;
    Size winSize = Size(21,21);
    cout << "Tracking: Finding term criteria" << endl;
    TermCriteria termcrit = TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, 0.01);

    cout << "Tracking: Calculating optical flow" << endl;
    calcOpticalFlowPyrLK(img_1, img_2, points1, points2, status, err, winSize, 3, termcrit, 0, 0.001);

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

Matx31d DotProdMatx31d(Mat E, Matx31d point){
    double new_x = E.at<double>(0,0)*point(0,0)+E.at<double>(0,1)*point(1,0)+E.at<double>(0,2)*point(2,0);
    double new_y = E.at<double>(1,0)*point(0,0)+E.at<double>(1,1)*point(1,0)+E.at<double>(1,2)*point(2,0);
    double new_z = E.at<double>(2,0)*point(0,0)+E.at<double>(2,1)*point(1,0)+E.at<double>(2,2)*point(2,0);

    return Matx31d(new_x, new_y, new_z);
}

Matx31d HRotTrans(Mat Rot, Matx31d Trans, Matx31d point){
    double new_x = Rot.at<double>(0,0)*point(0,0)+Rot.at<double>(0,1)*point(1,0)+Rot.at<double>(0,2)*point(2,0)+Trans(0,0);
    double new_y = Rot.at<double>(1,0)*point(0,0)+Rot.at<double>(1,1)*point(1,0)+Rot.at<double>(1,2)*point(2,0)+Trans(0,1);
    double new_z = Rot.at<double>(2,0)*point(0,0)+Rot.at<double>(2,1)*point(1,0)+Rot.at<double>(2,2)*point(2,0)+Trans(0,2);

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
    FileStorage fs;
    fs.open(calib_file, FileStorage::READ);
    if(!fs.isOpened()){
        printf("Failed to open file %s\n", calib_file.c_str());
        return -1;
    }
    fs["camera_matrix"] >> cameraMatrix;
    fs["distortion_coefficients"] >> distCoeffs;
    fs.release();

    //Image correction
    double focal = cameraMatrix.at<double>(0,0);                                        //Focal length
    Point2d pp = Point2d(cameraMatrix.at<double>(0,2), cameraMatrix.at<double>(1,2));   //Location of the focal point on the z-plane

    initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imgSize, 1, imgSize, 0), imgSize, CV_16SC2, map1, map2);

    namedWindow("Image", WINDOW_KEEPRATIO);
    resizeWindow("Image",imgSize);
    namedWindow("Image2", WINDOW_KEEPRATIO);
    resizeWindow("Image2",imgSize);

//    namedWindow("Gray_1", WINDOW_KEEPRATIO);
//    resizeWindow("Gray_1",imgSize);
//    namedWindow("Gray_2", WINDOW_KEEPRATIO);
//    resizeWindow("Gray_2",imgSize);

    for(int i = 1; i <= numImgs; i++){
        cout << "Correcting image " << i << endl;

        ostringstream imgName;
        imgName << "photos/mouse" << i << ".jpg";
        img = imread(imgName.str());

        remap(img, imgRect[i-1], map1, map2, INTER_LINEAR);
        cvtColor(imgRect[i-1], gray[i-1], COLOR_BGR2GRAY);

//        imshow("Image", imgRect[i-1]);
//        imshow("Gray_1", gray[i-1]);
//        waitKey(0);
    }

    //Feature detection/tracking and essential matrix/rotation/transformation calculation
    FileStorage fs1(fileERT, FileStorage::WRITE);
    bool setWrite = false;
    try{
        fs.open(filePoints, FileStorage::READ);
    }catch(exception& e){
        cout << "Warning, file is empty. Writing to .xml now, this may take a while (edit the float at the end of calcOpticalFlowPyrLK for performance) ..." << endl;
        setWrite = true;
        fs.open(filePoints, FileStorage::WRITE);
    }

    for(int i = 1; i < numImgs; i++){
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

        if(setWrite){
            cout << "\nDetecting and tracking features on images " << i << " and " << i+1 << endl;

            featureDetection(gray[i-1], points[i-1]);
            featureDetection(gray[i], points[i]);

            cout << "Features in Image 1: " << points[i-1].size() << endl << "Features in Image 2: " << points[i].size() << endl;

//            points[i].resize(points[i-1].size()); //Only necessary if using OPTFLOW_USE_INITIAL_FLOW
            featureTracking(gray[i-1], gray[i], points[i-1], points[i], status);

            fs << pointsNameF1.str() << points[i-1];
            fs << pointsNameF2.str() << points[i];
        }else{
            fs[pointsNameF1.str()] >> points[i-1];
            fs[pointsNameF2.str()] >> points[i];
        }

        Mat hImgRect;
        hconcat(imgRect[i-1], imgRect[i], hImgRect);
        for(int j = 0; j < (int)points[i-1].size(); j++)
            line(hImgRect, points[i-1].at(j), points[i].at(j)+Point2f(imgRect[i-1].size().width, 0), Scalar(int(rand()%255+1), int(rand()%255+1), int(rand()%255+1)));
        while(waitKey(10) != 'x'){
//            imshow("Gray_1", gray[i-1]);
//            imshow("Gray_2", gray[i]);
            imshow("Image", hImgRect);
        }

        cout << "First feature in image = " << points[i-1].at(0) << endl;

        Mat mask, R, t;
        Mat E = findEssentialMat(points[i-1], points[i], focal, pp, RANSAC, 0.999, 1.0, mask);
        cout << "mask elements: " << (int)mask.size().height << endl;

        int indexCorrection = 0;
        for(int j = 0; j < (int)mask.size().height; j++){
            Point2f pt = points[i].at(j - indexCorrection);
            if ((mask.at<int>(j,0) == 0)||(pt.x<0)||(pt.y<0)){
                if(pt.x<0 || pt.y<0) mask.at<int>(j,0) = 0;
                points[i-1].erase(points[i-1].begin() + j - indexCorrection);
                points[i].erase(points[i].begin() + j - indexCorrection);
                indexCorrection++;
            }
        }

        Mat hImgRect2, hImgRect3;
        hconcat(imgRect[i-1], imgRect[i], hImgRect2);
        hconcat(imgRect[i-1], imgRect[i], hImgRect3);
        int pointNum = 0;
        for(int j = 0; j < (int)points[i-1].size(); j++){
            line(hImgRect2, points[i-1].at(j), points[i].at(j)+Point2f(imgSize.width, 0), Scalar(int(rand()%255+1), int(rand()%255+1), int(rand()%255+1)));
            circle(hImgRect3, points[i-1].at(j), 1, Scalar(0,0,255), -1);
            circle(hImgRect3, points[i].at(j)+Point2f(imgSize.width, 0), 1, Scalar(0,0,255), -1);
            if(((int)points[i-1].at(j).x>=300 && (int)points[i-1].at(j).x<=590) && ((int)points[i-1].at(j).y>=320 && (int)points[i-1].at(j).y<=500)){ //Locate points in the center
                circle(hImgRect3, points[i-1].at(j), 1, Scalar(255,0,0), -1);
                circle(hImgRect3, points[i].at(j)+Point2f(imgSize.width, 0), 1, Scalar(255,0,0), -1);
                cout << "Feature number " << j << endl << points[i-1].at(j).x << ", " << points[i-1].at(j).y << endl;
                cout << points[i].at(j).x << ", " << points[i].at(j).y << endl;
                pointNum = j;
            }
        }
        while(waitKey(10) != 'x'){
            imshow("Center Features", hImgRect3);
            imshow("Image2", hImgRect2);
        }

        recoverPose(E, points[i], points[i-1], R, t, focal, pp);
        cout << "The essential matrix is: " << endl << E << endl << "Rotation is: " << endl << R << endl << "Translation is: " << endl << t << endl; //Output ess, rot, and trans matrices to console

        fs1 << framesERT.str() << "{" << essERT.str() << E << rotERT.str() << R << transERT.str() << t << "}"; //Write essential, rotation, and translation matrices to file

        //Estimate depth (position in 3D space of a feature)
        float LeftCoeff, RightCoeff;
        Vector3D pointF1Pos, pointF2Pos;
        pointF1Pos.Org = fp;
        pointF1Pos.Scl = Matx31d((double)points[i-1].at(pointNum).x - pp.x, (double)points[i-1].at(pointNum).y - pp.y, focal);
        pointF2Pos.Org = HRotTrans(R, t, fp);
        pointF2Pos.Scl = HRotTrans(R, t, Matx31d((double)points[i].at(pointNum).x - pp.x, (double)points[i].at(pointNum).y - pp.y, focal));

        ClosestApproach(pointF2Pos, pointF1Pos, RightCoeff, LeftCoeff);
        Matx31d pointPos3D = (pointF2Pos.Org + pointF2Pos.Scl * RightCoeff + pointF1Pos.Org + pointF1Pos.Scl * LeftCoeff) * 0.5;

        cout << "F1 Org (focal point): " << endl << pointF1Pos.Org << endl << "F1 Scl (feature point): " << endl << pointF1Pos.Scl << endl;
        cout << "F2 Org (focal point): " << endl << pointF2Pos.Org << endl << "F2 Scl (feature point): " << endl << pointF2Pos.Scl << endl;
        cout << "Right coefficient = " << RightCoeff << endl << "Left coefficient = " << LeftCoeff << endl;
        cout << "Feature position in 3D space: " << endl << pointPos3D << endl;
    }
    cout << "Process complete..." << endl;
    fs.release();
    fs1.release();
}
