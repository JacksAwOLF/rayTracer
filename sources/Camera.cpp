#include "Camera.h"

PixelSize::PixelSize(){}

PixelSize::PixelSize(Arr3 w, Arr3 h){
    pixelW = w;
    pixelH = h;
    pixelMid = (pixelW / 2.0) + (pixelH / 2.0);
}

Camera::Camera(){}

Camera::Camera(istream& in){
    in >> eye >> at >> up >> foy;
}

// return w, h of the pixels
PixelSize Camera::calculatePixelSize(int w, int h){
    Arr3 dis = at - eye,
        hor = dis.cross(up).normalize(),
        ver = dis.cross(hor).normalize();
    float theta = (foy/2) * 3.1415926535 / 180.0,
        screenH = sin(theta) / cos(theta) * dis.length() * 2,
        screenW = screenH * ((float)w/h);

    cout<<"screen W/H: "<<screenW<<" , "<<screenH<<endl;
    cout<<"hor/ver: "<<hor<<" , "<<ver<<endl;
    
    return PixelSize(
        hor * (screenW / w),
        ver * (screenH / h)
    );
}