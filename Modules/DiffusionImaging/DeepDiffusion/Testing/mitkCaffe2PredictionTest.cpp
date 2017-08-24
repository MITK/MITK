/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestingConfig.h>
#include <mitkTestFixture.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <caffe2/core/operator.h>
#include <caffe2/core/predictor.h>
#include <caffe2/core/tensor.h>

class mitkCaffe2PredictionTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkCaffe2PredictionTestSuite);
  MITK_TEST(Test1);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/

public:

  void setUp() override
  {
  }

  void tearDown() override
  {
  }

  // converts cv::Mat to a Caffe2 Tensor object that serves as input for the network
  void to_tensorcpu(cv::Mat &img, caffe2::TensorCPU &tensor)
  {
    const int num_channel = tensor.dim(1);
    const int height = tensor.dim(2);
    const int width = tensor.dim(3);

    assert(img.channels() == num_channel);
    assert(img.cols == width);
    assert(img.rows == height);

    // convert to float
    if (num_channel==3 && img.type()!=CV_32FC3)
      img.convertTo(img, CV_32FC3);
    else if(num_channel==1 && img.type()!=CV_32FC1)
      img.convertTo(img, CV_32FC1);

    // subtract mean
    img -= 128;  // necessary?

    float* tensor_data = tensor.mutable_data<float>();

    // create array of cv::Mat where each matrix points to the tensor data
    std::vector<cv::Mat> bgr;
    for (int i = 0; i < num_channel; ++i)
    {
      bgr.push_back(cv::Mat(height, width, CV_32FC1, tensor_data));
      tensor_data += width * height;
    }

    // HWC to CHW
    // splits 3 channel matrix into three single channel matrices
    cv::split(img, bgr);
  }

  void Test1()
  {
    std::cout << "loading test image" << std::endl;
    cv::Mat img = cv::imread(GetTestDataFilePath("DiffusionImaging/DeepDiffusion/Caffe2_Prediction_Test/flower.jpg"), CV_LOAD_IMAGE_COLOR);

    std::cout << "loading network definition and weights (predict_net and init_net)" << std::endl;
    // use pretrained squeezenet in this case (https://github.com/caffe2/models)
    caffe2::NetDef init_net, predict_net;
    ReadProtoFromFile(GetTestDataFilePath("DiffusionImaging/DeepDiffusion/Caffe2_Prediction_Test/squeezenet/init_net.pb"), &init_net);
    ReadProtoFromFile(GetTestDataFilePath("DiffusionImaging/DeepDiffusion/Caffe2_Prediction_Test/squeezenet/predict_net.pb"), &predict_net);
    std::cout << "Predict net: " << ProtoDebugString(predict_net) << std::endl;

    std::cout << "creating predictor object" << std::endl;
    auto predictor = caffe2::make_unique<caffe2::Predictor>(init_net, predict_net);

    // input image specifications
    int width = 227;
    int height = 227;
    int channels = 3;

    std::cout << "creating tensor object (Caffe2 network input)" << std::endl;
    caffe2::TensorCPU input(std::vector<int>({1, channels, height, width}));

    std::cout << "converting opencv image to tensor object" << std::endl;
    to_tensorcpu(img, input);

    // the predictor takes a vector of tensors as input
    caffe2::Predictor::TensorVector inputVec{&input};

    // the netork output is also stored in a vector of tensors
    caffe2::Predictor::TensorVector outputVec;

    std::cout << "running prediction" << std::endl;
    predictor->run(inputVec, &outputVec);

    // since we only had one input tensor, we also have one output tensor in the vector
    caffe2::TensorCPU* output = outputVec[0];

    // the output tensor is of size 1x1000 (batch size 1 and 1000 classes from imagenet)
    const float* output_data =  output->data<float>();

    std::cout << "looking for max. probability class" << std::endl;
    int max_class = -1;
    float max_prob = 0;
    for (int i=0; i<output->size(); i++)
      if(output_data[i]>max_prob)
      {
        max_prob = output_data[i];
        max_class = i;
      }
    std::cout << "Class: " << max_class << std::endl;
    std::cout << "Probability: " << max_prob << std::endl;

    // should be 985 (daisy)
    CPPUNIT_ASSERT_MESSAGE("Class probability should be ~0.995", fabs(max_prob-0.995)<0.001);
    CPPUNIT_ASSERT_MESSAGE("Class should be 985", max_class==985);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkCaffe2Prediction)
