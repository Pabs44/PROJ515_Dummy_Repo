#include </home/pablo/opencv/include/opencv2/opencv.hpp>

using namespace cv;
using namespace std;

const int numImgs = 14;

Mat img = imread("photos/phone1.jpg");
Mat cameraMatrix, map1, map2;
Mat imgRect[numImgs];
Mat gray[numImgs];
Mat distCoeffs = Mat::zeros(8, 1, CV_64F);
vector<Point2f> points[numImgs];
vector<uchar> status;
Size imgSize = img.size()/4;

const string filePoints = "points.xml";
const string fileERT = "ERT.xml";
const string calib_file = "camera_calibration_fourth.xml";
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

int main(){
    namedWindow("Image", WINDOW_KEEPRATIO);
    resizeWindow("Image",imgSize);

    FileStorage fs(calib_file, FileStorage::READ);
    if(!fs.isOpened()){
        printf("Failed to open file %s\n", calib_file.c_str());
        return -1;
    }
    fs["camera_matrix"] >> cameraMatrix;
    fs["distortion_coefficients"] >> distCoeffs;

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
        waitKey(0);
    }

//    FileStorage fs1(filePoints, FileStorage::WRITE);
    FileStorage fs2(fileERT, FileStorage::WRITE);
    for(int i = 1; i < numImgs; i++){
        ostringstream pointsNameF1;
        pointsNameF1 << "points" << i;
        ostringstream pointsNameF2;
        pointsNameF2 << "points_frame" << i+1;
        ostringstream pointsNameF2_real;
        pointsNameF2_real << "points" << i+1;
        ostringstream framesERT;
        framesERT << "frames_" << i << "_" << i+1;
        ostringstream essERT, rotERT, transERT;
        essERT << framesERT.str() << "_essential";
        rotERT << framesERT.str() << "_rotation";
        transERT << framesERT.str() << "_transformation";

        //Uncomment this portion when creating the points for the first time
//        cout << "\nDetecting and tracking features on images " << i << " and " << i+1 << endl;
//        featureDetection(imgRect[i-1], points[i-1]);
//        featureDetection(imgRect[i], points[i]);
//        featureTracking(imgRect[i-1], imgRect[i], points[i-1], points[i], status);
//        fs1 << pointsNameF1.str() << points[i-1];
//        fs1 << pointsNameF2.str() << points[i];

        //Uncomment this portion when points.xml already contains all the needed info
        FileStorage fs1_open(filePoints, FileStorage::READ);
        if(!fs1_open.isOpened()){
            printf("Failed to open file %s\n", filePoints.c_str());
            return -1;
        }else{
            fs1_open[pointsNameF1.str()] >> points[i-1];
            fs1_open[pointsNameF2.str()] >> points[i];
        }
        Mat mask;
        Mat E = findEssentialMat(points[i], points[i-1], focal, pp, RANSAC, 0.999, 1.0, mask);
        cout << "The essential matrix is: " << E << endl;
        Mat R, t;
        recoverPose(E, points[i], points[i-1], R, t, focal, pp, mask);
        cout << "Frames " << i << " to " << i+1 << ": " << endl;
        cout << "Rotation is: " << R << endl;
        cout << "Translation is: " << t << endl;
        fs2 << framesERT.str() << "{";
        fs2 << essERT.str() << E;
        fs2 << rotERT.str() << R;
        fs2 << transERT.str() << t;
        fs2 << "}";
    }
//    fs1.release();
    fs2.release();
}
