#include </home/pablo/opencv/include/opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main(){
    for(int i = 1; i <= 9; i++){
        Size patternsize(8,6); //interior number of corners
        stringstream filename;
        filename << "/home/pablo/opencv/samples/data/left" << setw(2) << setfill('0') << i << ".jpg";
        Mat img = imread(filename.str().c_str()); //source image
        Mat gray;
        cvtColor(img, gray, COLOR_BGR2GRAY);
        vector<Point2f> corners; //this will be filled by the detected corners

        //CALIB_CB_FAST_CHECK saves a lot of time on images
        //that do not contain any chessboard corners
        bool patternfound = findChessboardCorners(gray, patternsize, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);

        if(patternfound){
            cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::MAX_ITER, 30, 0.1));
            cout << "success" << endl;
        }else cout << "nada" << endl;

        drawChessboardCorners(img, patternsize, Mat(corners), patternfound);
        imshow("output", img);
        waitKey(1000);
        //vector<Point2f> ret, mtx, dist, rvecs, tvecs = calibrateCamera(objpoints, imgpoints, gray.shape[::-1], None, None);
    }
    destroyAllWindows();
}
