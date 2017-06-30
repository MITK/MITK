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
#include <mitkLogMacros.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <caffe/caffe.hpp>
#include <random>

class mitkCaffePredictionTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkCaffePredictionTestSuite);
  MITK_TEST(Test1);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/

public:

  std::default_random_engine generator;

  void setUp() override
  {
  }

  void tearDown() override
  {
  }

  void Test1()
  {
    generator.seed(0);
    std::normal_distribution<double> distribution(0.0,0.1);

    std::string network_file = GetTestDataFilePath("DiffusionImaging/DeepDiffusion/Caffe_Prediction_Test/unet_structure.prototxt");
    std::string weights_file = GetTestDataFilePath("DiffusionImaging/DeepDiffusion/Caffe_Prediction_Test/unet_weights.caffemodel");

    MITK_INFO << "1";
    caffe::shared_ptr<caffe::Net<float>> network;
    const std::vector<std::string> stages = {"deploy"};
    network.reset(new caffe::Net<float>(network_file, caffe::TEST, 0, &stages));
    network->CopyTrainedLayersFrom(weights_file);
    MITK_INFO << "2";

    caffe::Blob<float>* input_layer = network->input_blobs()[0];
    float* input_data = input_layer->mutable_cpu_data();

    cv::Mat ground_truth(input_layer->height(), input_layer->width(), CV_8UC1);
    int counter = 0;
    for (int x = 0; x < input_layer->height(); x++)
        for (int y = 0; y < input_layer->width(); y++)
            for (int j = 0; j < input_layer->channels(); j++)
            {
              input_data[counter] = 0;
              if (x>20 && x<40 && y>20 && y<40)
                  input_data[counter] = 1;
              if (x>35 && x<55 && y>25 && y<45)
                  input_data[counter] = 2;

              if (x>2 && x<12 && y>45 && y<55)
                  input_data[counter] = 1;

              if (x>50 && x<60 && y>50 && y<60)
                  input_data[counter] = 1;

              ground_truth.data[counter] = input_data[counter] * 100;
              input_data[counter] += distribution(generator);

              counter++;
            }

    network->Forward();

    cv::Mat input_img = cv::Mat(input_layer->height(), input_layer->width(), CV_32FC1, input_data) * 100;
    input_img.convertTo(input_img, CV_8UC1);

    boost::shared_ptr<caffe::Blob<float> > output_layer = network->blob_by_name("argmax");
    cv::Mat out_img = cv::Mat(input_layer->height(), input_layer->width(), CV_32FC1, output_layer->mutable_cpu_data()) * 100;
    out_img.convertTo(out_img, CV_8UC1);

    cv::imwrite(MITK_DATA_DIR + 'DiffusionImaging/DeepDiffusion/Caffe_Prediction_Test/input.png', input_img);
    cv::imwrite(MITK_DATA_DIR + 'DiffusionImaging/DeepDiffusion/Caffe_Prediction_Test/output.png', out_img);
    cv::imwrite(MITK_DATA_DIR + 'DiffusionImaging/DeepDiffusion/Caffe_Prediction_Test/ground_truth.png', ground_truth);
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkCaffePrediction)
