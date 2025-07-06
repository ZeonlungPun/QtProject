#include "thread.h"
#include <QDebug>
#include <QThread>


Detect::Detect(QObject *parent) : QObject(parent)
{

}



void Detect::working(cv::Mat raw_img)
{
    // Set input size before inference
    detector->setInputSize(raw_img.size());
    cv::Mat face_coordinates;
    bool faceDetected = detector->detect(raw_img, face_coordinates);
    emit detectFinished(face_coordinates,faceDetected);

}


