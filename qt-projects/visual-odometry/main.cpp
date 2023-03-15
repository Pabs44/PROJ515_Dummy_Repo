#include </home/pablo/opencv/include/opencv2/opencv.hpp>

using namespace cv;
using namespace std;

struct Vector3D{
    Matx31d Org = Matx31d(0,0,0);
    Matx31d Scl = Matx31d(0,0,0);
};

const string filePoints = "points.xml";
const string fileERT = "ERT.xml";
const string calib_file = "camera_calibration_fourth.xml";
const int userHeight = 1800; //This is in mm -> 180 cm
const int numImgs = 14;

Mat img = imread("photos/phone1.jpg");
Mat cameraMatrix, map1, map2;
Mat imgRect[numImgs];
Mat gray[numImgs];
Mat distCoeffs = Mat::zeros(8, 1, CV_64F);

vector<Point2f> points[numImgs];
vector<vector<Point2f>> pointsVector;
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
    //Size winSize = Size(21,21);
    Size winSize = imgSize;
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
    namedWindow("Image", WINDOW_KEEPRATIO);
    resizeWindow("Image",imgSize);

    FileStorage fsCalib(calib_file, FileStorage::READ);
    if(!fsCalib.isOpened()){
        printf("Failed to open file %s\n", calib_file.c_str());
        return -1;
    }
    fsCalib["camera_matrix"] >> cameraMatrix;
    fsCalib["distortion_coefficients"] >> distCoeffs;

    //Image correction
    double focal = cameraMatrix.at<double>(0,0);
    Point2d pp = Point2d(cameraMatrix.at<double>(0,2), cameraMatrix.at<double>(1,2));
    initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imgSize, 1, imgSize, 0), imgSize, CV_16SC2, map1, map2);
    for(int i = 1; i <= numImgs; i++){
        cout << "Correcting image " << i << endl;
        ostringstream imgName;
        imgName << "photos/phone" << i << ".jpg";
        img = imread(imgName.str());
        Mat imgTemp;
        resize(img, imgTemp, imgSize, INTER_LINEAR);
        remap(imgTemp, imgRect[i-1], map1, map2, INTER_LINEAR);
        cvtColor(imgRect[i-1], gray[i-1], COLOR_BGR2GRAY);
        imshow("Image", imgRect[i-1]);
//        waitKey(0);
    }
    Matx31d fp = Matx31d(pp.x, pp.y, 0);

    //Feature detection/tracking and essential matrix/rotation/transformation calculation
//    FileStorage fsPoints_write(filePoints, FileStorage::WRITE);
    FileStorage fsERT_write(fileERT, FileStorage::WRITE);
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

        //Uncomment this portion when creating the points for the first time
//        cout << "\nDetecting and tracking features on images " << i << " and " << i+1 << endl;
//        featureDetection(imgRect[i-1], points[i-1]);
//        featureDetection(imgRect[i], points[i]);
//        featureTracking(imgRect[i-1], imgRect[i], points[i-1], points[i], status);

//        fsPoints_write << pointsNameF1.str() << points[i-1];
//        fsPoints_write << pointsNameF2.str() << points[i];

        //Uncomment this portion when points.xml already contains all the needed info
        FileStorage fsPoints_open(filePoints, FileStorage::READ);
        if(!fsPoints_open.isOpened()){
            printf("Failed to open file %s\n", filePoints.c_str());
            return -1;
        }else{
            fsPoints_open[pointsNameF1.str()] >> points[i-1];
            fsPoints_open[pointsNameF2.str()] >> points[i];
        }

        cout << "First feature in image = " << points[i-1].at(0) << endl;

        Mat mask, R, t;
        Mat E = findEssentialMat(points[i], points[i-1], focal, pp, RANSAC, 0.999, 1.0, mask);
        cout << "The essential matrix is: " << E << endl;
        recoverPose(E, points[i], points[i-1], R, t, focal, pp, mask);
        cout << "Rotation is: " << R << endl;
        cout << "Translation is: " << t << endl;

        fsERT_write << framesERT.str() << "{";
        fsERT_write << essERT.str() << E;
        fsERT_write << rotERT.str() << R;
        fsERT_write << transERT.str() << t;
        fsERT_write << "}";

        float LeftCoeff, RightCoeff;
        Matx31d point1Vec = Matx31d((double)points[i-1].at(0).x, (double)points[i-1].at(0).y, focal);
        Vector3D point1Pos;
        point1Pos.Org = fp;
        point1Pos.Scl = point1Vec;
        cout << point1Pos.Scl << endl;

        Matx31d point2Vec = Matx31d((double)points[i].at(0).x, (double)points[i].at(0).y, focal);
        Vector3D point2Pos;

        double fp_new_x = E.at<double>(0,0)*fp(0,0)+E.at<double>(0,1)*fp(0,1)+E.at<double>(0,2)*fp(0,2);
        double fp_new_y = E.at<double>(1,0)*fp(0,0)+E.at<double>(1,1)*fp(0,1)+E.at<double>(1,2)*fp(0,2);
        double fp_new_z = E.at<double>(2,0)*fp(0,0)+E.at<double>(2,1)*fp(0,1)+E.at<double>(2,2)*fp(0,2);
        Matx31d fp_new = Matx31d(fp_new_x, fp_new_y, fp_new_z);

        point2Pos.Org = fp_new;
        point2Pos.Scl = point2Vec;
        cout << point2Pos.Scl << endl;

        ClosestApproach(point1Pos, point2Pos, RightCoeff, LeftCoeff);
        cout << "Right coefficient = " << RightCoeff << endl;
        cout << "Left coefficient = " << LeftCoeff << endl;

//        cout << "\nNumber of points: " << (int)points[i-1].size() << endl;
//        for(int j = 0; j < (int)points[i-1].size(); j++){
//            Matx31d point1Vec = Matx31d((double)points[i-1].at(j).x, (double)points[i-1].at(j).y, focal);
//            Vector3D point1Pos;
//            point1Pos.Org = fp;
//            point1Pos.Scl = point1Vec;
//            cout << point1Pos.Scl << endl;

//            Matx31d point2Vec = Matx31d((double)points[i].at(j).x, (double)points[i].at(j).y, focal);
//            Vector3D point2Pos;
//            point2Pos.Org = fp;
//            point2Pos.Scl = point2Vec;
//            cout << point2Pos.Scl << endl;

//            ClosestApproach(point1Pos, point2Pos, RightCoeff, LeftCoeff);
//            cout << "Right coefficient = " << RightCoeff << endl;
//            cout << "Left coefficient = " << LeftCoeff << endl;
//        }
    }
    cout << "Process complete..." << endl;
//    fsPoints_write.release();
    fsERT_write.release();

//    BackLED.Pos3D = (BackLED.RightVec.Org+BackLED.RightVec.Scl * RightCoeff+BackLED.LeftVec.Org + BackLED.LeftVec.Scl * LeftCoeff)*0.5;
}
