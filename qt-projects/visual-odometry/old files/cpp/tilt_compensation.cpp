//Tilt compensation
Mat vecP1P2(3,1,CV_64F);
Mat vecY(3,1,CV_64F);
vecP1P2 = Mat(point2Pos3D);
vecY = Mat(double(norm(point1Pos3D)) + Matx31d(0,1,0));

double c = vecP1P2.dot(vecY);
Mat v = vecP1P2.cross(vecY);
cout << endl << v << endl;
Mat vx = Mat::zeros(3,3,CV_64F);
vx.at<double>(0,1) = -v.at<double>(2);
vx.at<double>(0,2) = v.at<double>(1);
vx.at<double>(1,0) = v.at<double>(2);
vx.at<double>(1,2) = -v.at<double>(0);
vx.at<double>(2,0) = -v.at<double>(1);
vx.at<double>(2,1) = v.at<double>(0);
cout << vx << endl;
Mat R3D = Mat::eye(3,3,CV_64F) + vx + (vx.mul(vx) * 1/(1+c));
point1Pos3D = HRotTrans(R3D, Matx31d(0,0,0), point1Pos3D);
point2Pos3D = HRotTrans(R3D, Matx31d(0,0,0), point2Pos3D);
//Normalize both vectors (pythagoras for length and then divide the scalars)
