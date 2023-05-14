//Filter real space array and remove all irrelevant/impossible points
indexCorrection = 0;
for (int j = 0; j < (int)realPos3D.size(); j++) {
    if(realPos3D[j](2) < double(1900.00) || realPos3D[j](2) > double(2100.00) /*|| (int)abs(realPos3D[j](1)-realPos3D[j-1](1)) > double(500.00)*/){
        realPos3D.erase(realPos3D.begin() + j - indexCorrection);
        indexCorrection++;
        cout << "point removed" << endl;
    }
}
