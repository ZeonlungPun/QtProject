#ifndef THREAD_H
#define THREAD_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>
#include <iostream>




class Detect : public QObject
{
    Q_OBJECT
public:
    explicit Detect(QObject *parent = nullptr);

    void working(cv::Mat frame,int model_input_h, int model_input_w,int model_output_h,int model_output_w,std::vector<std::string> input_node_names,std::vector<std::string> output_node_names,Ort::Session& session);

signals:

    void curResult(cv::Mat raw_img);

};




#endif // THREAD_H
