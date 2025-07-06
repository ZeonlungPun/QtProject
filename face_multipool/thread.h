#ifndef THREAD_H
#define THREAD_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <iostream>




class Detect : public QObject
{
    Q_OBJECT
public:
    void setDetector(const cv::Ptr<cv::FaceDetectorYN>& det) {
           this->detector = det;
       }
    explicit Detect(QObject *parent = nullptr);

    void working(cv::Mat raw_img);

signals:

    void detectFinished(cv::Mat face_coordinates,bool faceDetected);
private:
    cv::Ptr<cv::FaceDetectorYN> detector;

};







#endif // THREAD_H
