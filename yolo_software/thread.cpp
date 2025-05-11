#include "thread.h"
#include <QDebug>
#include <QThread>


Detect::Detect(QObject *parent) : QObject(parent)
{

}



void Detect::working(cv::Mat frame,int model_input_h, int model_input_w,int model_output_h,int model_output_w,std::vector<std::string> input_node_names,std::vector<std::string> output_node_names,Ort::Session& session)
{
    qDebug() << "子線程地址1: " << QThread::currentThread();
    cv::Mat raw_img=frame.clone();
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    float ratio = std::min(static_cast<float>(model_input_h) / frame.rows,
                           static_cast<float>(model_input_w) / frame.cols);
    int newh=(int) std::round(frame.rows*ratio);
    int neww=(int) std::round(frame.cols*ratio);
    cv::Size new_unpad(neww,newh);
    //get the padding length in each size
    float dw=(model_input_w-neww)/2;
    float dh=(model_input_h-newh)/2;

    if (neww !=model_input_w || newh !=model_input_h)
    {  //resize the image with same ratio for wdith and height
        cv::resize(frame,frame,new_unpad,cv::INTER_LINEAR);
    }
    // calculate the padding pixel around
    int top =(int) std::round(dh-0.1);
    int bottom= (int) std::round(dh+0.1);
    int left = (int) std::round(dw-0.1);
    int right= (int) std::round(dw+0.1);


    cv::copyMakeBorder(frame, frame, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(114, 114, 114));



    //normalize the image to [0,1] and resize the image to the model input
    cv::Mat blob;
    cv::dnn::blobFromImage(frame,blob, 1.0 / 255.0, cv::Size(model_input_w,model_input_h), cv::Scalar(), true, false);
    // matrix to save the innitial output from yolov model
    cv::Mat det_output;

    // Get input tensor shape info
    size_t tpixels = model_input_h * model_input_w * 3;
    std::array<int64_t, 4> input_shape_info{1, 3, model_input_h, model_input_w};
    // Create input tensor
    auto allocator_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(allocator_info, blob.ptr<float>(), tpixels, input_shape_info.data(), input_shape_info.size());

    const std::array<const char*, 1> inputNames = {input_node_names[0].c_str()};
    const std::array<const char*, 1> outNames = {output_node_names[0].c_str()};

    // Perform inference
    std::vector<Ort::Value> ort_outputs;
    try {
        ort_outputs = session.Run(Ort::RunOptions{nullptr}, inputNames.data(), &input_tensor, 1, outNames.data(), outNames.size());
    } catch (const std::exception& e) {
        std::cerr << "Error during ONNX inference: " << e.what() << std::endl;

    }

    // Get model output
    const float* pdata = ort_outputs[0].GetTensorMutableData<float>();
    cv::Mat dout(model_output_h, model_output_w, CV_32F, (float*)pdata);
    // (num_anchors, 4+num_classes)=(8400,4+num_classes)
    det_output = dout.t();

    //vector to save the results index after NMS
    std::vector<int> indexes;
    float NMSValue = 0.5;
    float ConfValue= 0.5;
    std::vector<cv::Rect> boxes;
    std::vector<int>classIds;
    std::vector<float>confidences;
    // Post-process detections
    for (int i = 0; i < det_output.rows; i++) {
        cv::Mat classes_scores = det_output.row(i).colRange(4, 4 +80);

        cv::Point classIdPoint;
        double score;
        minMaxLoc(classes_scores, 0, &score, 0, &classIdPoint);

        if (score > 0.05) {
            float cx = det_output.at<float>(i, 0);
            float cy = det_output.at<float>(i, 1);
            float ow = det_output.at<float>(i, 2);
            float oh = det_output.at<float>(i, 3);

            // Adjust coordinates based on padding and scaling
            cx = (cx - left) / ratio;
            cy = (cy - top) / ratio;
            ow = ow / ratio;
            oh = oh / ratio;



            int x = static_cast<int>(cx - 0.5 * ow);
            int y = static_cast<int>(cy - 0.5 * oh);
            int width = static_cast<int>(ow);
            int height = static_cast<int>(oh);

            cv::Rect box(x, y, width, height);
            boxes.push_back(box);
            classIds.push_back(classIdPoint.x);
            confidences.push_back(score);
        }
    }

    // Apply Non-Maximum Suppression (NMS)
    cv::dnn::NMSBoxes(boxes, confidences, ConfValue, NMSValue, indexes);
    for (size_t i = 0; i < indexes.size(); i++)
    {

        int index = indexes[i];
        int label_idx = classIds[index];
        float score= confidences[index];
        std::ostringstream scoreStream;
        scoreStream << score;
        cv::putText(raw_img,
                    scoreStream.str(), // 標籤名稱
                    cv::Point(boxes[index].x, boxes[index].y + 5), // 顯示位置
                    cv::FONT_HERSHEY_COMPLEX, // 使用複雜字體
                    1, // 字體大小（可以根據需要調整）
                    cv::Scalar(0, 0, 255), // 顏色（紅色）
                    2, // 字體粗細
                    8); // 線型（8表示開放型字形）


        // 繪製邊界框
        cv::rectangle(raw_img, boxes[index], cv::Scalar(0, 0, 255), 2, 8);

    }
    emit curResult(raw_img);




}

