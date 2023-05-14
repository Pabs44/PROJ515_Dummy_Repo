//Calculate hip circumference - front (repeat process for side and other measurements)
string circPointsName = "Pos_3D";
FileStorage fsHip;
fsHip.open(fileHipCirc, FileStorage::WRITE);
if(fsHip.isOpened() != true) return -1;
fsHip << "points" << "{";

double circVal = 0, minZ = 0;
bool firstJ = true;
int circCnt = 0;
vector<Point2f> circXZ;
//Filter and place all hip feature points into a vector
for(int j = 0; j < (int)points[i-1].size(); j++){
    Matx31d pointPos((double)points[i-1].at(j).x, (double)points[i-1].at(j).y, focal);
    if(pointPos(1) > pose25d.at<double>(0,1)-20
            && pointPos(1) < pose25d.at<double>(0,1)+20
            && pointPos(0) > pose25d.at<double>(2,0)-90
            && pointPos(0) < pose25d.at<double>(3,0)+100)
    {
        if(firstJ){
            firstJ = false;
            minZ = realPos3D[j](2);
        }

        fsHip << circPointsName << realPos3D[j];
        circXZ.push_back(Point2f(realPos3D[j](0), realPos3D[j](2)));
        circCnt++;
    }
}
fsHip << "}";
fsHip.release();

//Sort hip feature points along the x-axis and compute the circumference by following along the points
sort(circXZ.begin(), circXZ.end(), comp);
for(int j = 0; j < (int)circXZ.size()-1; j++){
    double dx = circXZ.at(j+1).x - circXZ.at(j).x;
    double dz = (circXZ.at(j+1).y-minZ) - (circXZ.at(j).y-minZ);
    circVal += sqrt(pow(dx, 2) + pow(dz, 2));
}
cout << "Waist (front) circumference: " << circVal << " mm / " << circVal/10 << " cm" << endl;
