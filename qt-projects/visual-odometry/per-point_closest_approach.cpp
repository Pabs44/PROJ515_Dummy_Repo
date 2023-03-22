cout << "\nNumber of points: " << (int)points[i-1].size() << endl;
for(int j = 0; j < (int)points[i-1].size(); j++){
    Matx31d point1Vec = Matx31d((double)points[i-1].at(j).x, (double)points[i-1].at(j).y, focal);
    Matx31d point2Vec = Matx31d((double)points[i].at(j).x, (double)points[i].at(j).y, focal);

    Vector3D point1Pos, point2Pos;
    Matx31d fp_new = DotProdx31d(E, fp);
    point1Pos.Org = fp;
    point1Pos.Scl = point1Vec;
    point2Pos.Org = fp_new;
    point2Pos.Scl = point2Vec;

    ClosestApproach(point1Pos, point2Pos, RightCoeff, LeftCoeff);

    cout << point1Pos.Scl << endl << point2Pos.Scl << endl << "Right coefficient = " << RightCoeff << endl << "Left coefficient = " << LeftCoeff << endl;
}
