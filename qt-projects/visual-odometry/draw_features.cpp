for(int j = 0; j < (int)points[i-1].size(); j++){
    circle(imgRect[i-1], points[i-1].at(j), 1, Scalar(0,0,255), -1);
    if((int)points[i-1].at(j).x == 333){
        cout << j << endl << points[i-1].at(j).x << ", " << points[i-1].at(j).y << endl;
        cout << points[i].at(j).x << ", " << points[i].at(j).y << endl;
    }
}

while(waitKey(10) != 'x') imshow("Image", imgRect[i-1]);
