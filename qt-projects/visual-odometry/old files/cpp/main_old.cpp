#include </home/pablo/opencv/include/opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat img1 = imread("phone1.jpg");
Mat img2 = imread("phone2.jpg");
Mat cameraMatrix, depthMap, gray1, gray2, map1, map2, imgRect1, imgRect2, R, t;
Mat distCoeffs = Mat::zeros(8, 1, CV_64F);
vector<Point2f> points1, points2;
vector<uchar> status;
Size imageSize = img1.size();

const string filePoints1 = "points1.xml";
const string filePoints2 = "points2.xml";
const string calib_file = "calibration_output.xml";
const int userHeight = 1800; //This is in mm -> 180 cm

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
    Size winSize = imageSize;
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

    cout << "Traking: Finished tracking" << endl;
}

int main(){
    namedWindow("Image 1", WINDOW_KEEPRATIO);
    resizeWindow("Image 1",imageSize/4);
    namedWindow("Image 2", WINDOW_KEEPRATIO);
    resizeWindow("Image 2",imageSize/4);
//    namedWindow("points1", WINDOW_KEEPRATIO);
//    resizeWindow("points1",imageSize/4);
//    namedWindow("points2", WINDOW_KEEPRATIO);
//    resizeWindow("points2",imageSize/4);

    FileStorage fs(calib_file, FileStorage::READ);
    if(!fs.isOpened()){
        printf("Failed to open file %s\n", calib_file.c_str());
        return -1;
    }
    fs["camera_matrix"] >> cameraMatrix;
    fs["distortion_coefficients"] >> distCoeffs;

    initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0), imageSize, CV_16SC2, map1, map2);
    remap(img1, imgRect1, map1, map2, INTER_LINEAR);
    remap(img2, imgRect2, map1, map2, INTER_LINEAR);

    imshow("Image 1", imgRect1);
    imshow("Image 2", imgRect2);

    cvtColor(imgRect1, gray1, COLOR_BGR2GRAY);
    cvtColor(imgRect2, gray2, COLOR_BGR2GRAY);

//    featureDetection(imgRect1, points1);
//    featureDetection(imgRect2, points2);
//    featureTracking(imgRect1, imgRect2, points1, points2, status);

//    FileStorage fs1(filePoints1, FileStorage::WRITE);
//    fs1 << "points1" << points1;
//    FileStorage fs2(filePoints2, FileStorage::WRITE);
//    fs2 << "points2" << points2;

    FileStorage fs1_open(filePoints1, FileStorage::READ);
    if(!fs1_open.isOpened()){
        printf("Failed to open file %s\n", filePoints1.c_str());
        return -1;
    }else fs1_open["points1"] >> points1;

    FileStorage fs2_open(filePoints2, FileStorage::READ);
    if(!fs2_open.isOpened()){
        printf("Failed to open file %s\n", filePoints2.c_str());
        return -1;
    }else fs2_open["points2"] >> points2;

    double focal = cameraMatrix.at<double>(0,0);
    Point2d pp = Point2d(cameraMatrix.at<double>(0,2), cameraMatrix.at<double>(1,2));
    Mat mask;
    Mat E = findEssentialMat(points2, points1, focal, pp, RANSAC, 0.999, 1.0, mask);
    cout << "The essential matrix is: " << E << endl;
    recoverPose(E, points2, points1, R, t, focal, pp, mask);
    cout << "Rotation is: " << R << endl;
    cout << "Translation is: " << t << endl;

//    imshow("points1", points1);
//    imshow("points2", points2);
    waitKey(0);
}
