#include </home/pablo/opencv/include/opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat imgL = imread("bottleL.jpg");
Mat imgR = imread("bottleR.jpg");
Mat cameraMatrix, output, disparityMap, depthMap, grayL, grayR, map1, map2, imgRectL, imgRectR;
Mat distCoeffs = Mat::zeros(8, 1, CV_64F);
Size imageSize = imgL.size();
const string fileDisparity = "disparity.xml";
const string fileDepth = "depth.xml";
const string calib_file = "calibration_output.xml";
const int minDisparity = 32;
const int numDisparities = 192;
const int blockSize = 3;
int depthScale = 20;
Ptr<StereoSGBM> stereo = StereoSGBM::create(minDisparity, numDisparities, blockSize);

int main(int argc, char *argv[]){

    FileStorage fs(calib_file, FileStorage::READ);
    if(!fs.isOpened()){
        printf("Failed to open file %s\n", calib_file.c_str());
        return -1;
    }

    fs["camera_matrix"] >> cameraMatrix;
    fs["distortion_coefficients"] >> distCoeffs;

    initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0), imageSize, CV_16SC2, map1, map2);
    undistort(imgL, imgRectL, cameraMatrix, distCoeffs, Mat());
    undistort(imgR, imgRectR, cameraMatrix, distCoeffs, Mat());
    remap(imgRectL, imgRectL, map1, map2, INTER_LINEAR);
    remap(imgRectR, imgRectR, map1, map2, INTER_LINEAR);
    namedWindow("Left", WINDOW_KEEPRATIO);
    resizeWindow("Left",imageSize/4);
    imshow("Left", imgRectL);


//    resize(imgL, imgL, Size(),0.25,0.25, INTER_LINEAR_EXACT);
//    resize(imgR, imgR, Size(),0.25,0.25, INTER_LINEAR_EXACT);
    cvtColor(imgRectL, grayL, COLOR_BGR2GRAY);
    cvtColor(imgRectR, grayR, COLOR_BGR2GRAY);
    stereo->compute(grayL, grayR, disparityMap);

    int b = 50;
    double f = cameraMatrix.at<double>(0,0);
    f = f + 0.5;
    cout << "focal length: " << (int)f << endl;
    depthMap = depthScale * ((b*(int)f)/disparityMap);

    FileStorage fs1(fileDisparity, FileStorage::WRITE);
    fs1 << "Disparity" << disparityMap;
    FileStorage fs2(fileDepth, FileStorage::WRITE);
    fs2 << "Depth" << depthMap;

    cout << "Depth at middle: " << depthMap.at<int16_t>(2016,1512)/depthScale << endl; // Middle pixel (roughly)
//    cout << "Depth at bottle position (approx): " << depthMap.at<int16_t>(2016,1200)/depthScale << endl; // Pixel near left (to verify if range of values is valid)
    cout << "Depth at bottle position (approx): " << depthMap.at<int16_t>(2016,1600)/depthScale << endl; // Pixel near right (to verify if range of values is valid)
    cout << "Depth around bottle position: " << depthMap.at<int16_t>(2016,1700)/depthScale << endl; // Pixel near right (to verify if range of values is valid)

    namedWindow("disparity", WINDOW_KEEPRATIO);
    resizeWindow("disparity",imageSize/4);
    namedWindow("depth", WINDOW_KEEPRATIO);
    resizeWindow("depth",imageSize/4);
    imshow("disparity", disparityMap);
    imshow("depth", depthMap);
    waitKey(0);
}
