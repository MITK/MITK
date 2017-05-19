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

#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkProperties.h>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include "mitkCommandLineParser.h"
#include <boost/lexical_cast.hpp>

// Caffe2
#include <google/protobuf/text_format.h>
#include "caffe2/core/context.h"
#include "caffe2/core/operator.h"
#include "caffe2/core/predictor.h"
#include "caffe2/core/tensor.h"
#include "caffe2/utils/math.h"

#include <gtest/gtest.h>

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

/*!
* \brief Minimal example for Caffe2 prediction in MITK
*
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setTitle("Caffe2Test");
  parser.setCategory("Fiber Tracking and Processing Methods");
  parser.setContributor("MIC");
  parser.setDescription("Minimal example for Caffe2 prediction in MITK (Imagenet)");
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("image", "", mitkCommandLineParser::String, "image:", "", us::Any());
  parser.addArgument("init_net", "", mitkCommandLineParser::String, "init_net:", "the network weights (.pb)", us::Any());
  parser.addArgument("predict_net", "", mitkCommandLineParser::String, "predict_net:", "the network definition (.pb)", us::Any());

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  std::string init_net_filename = us::any_cast<std::string>(parsedArgs["init_net"]);
  std::string predict_net_filename = us::any_cast<std::string>(parsedArgs["predict_net"]);
  std::string image_filename = us::any_cast<std::string>(parsedArgs["image"]);

  cv::Mat img = cv::imread(image_filename, CV_LOAD_IMAGE_COLOR);

  cv::Mat img2;
  img.convertTo(img2, CV_32FC3);
  //img2 /= 255.0;
  //img2 -= 0.5;
  //img2 -= 127;
  //img2 *= 0.0;

  cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
  cv::imshow( "Display window", img2 );                   // Show our image inside it.
  cv::waitKey(0);                                          // Wait for a keystroke in the window

  caffe2::NetDef init_net, predict_net;
  CAFFE_ENFORCE(ReadProtoFromFile(init_net_filename, &init_net));
  CAFFE_ENFORCE(ReadProtoFromFile(predict_net_filename, &predict_net));

  // Can be large due to constant fills
  //MITK_INFO << "Init net: " << ProtoDebugString(init_net);
  MITK_INFO << "Predict net: " << ProtoDebugString(predict_net);
  auto predictor = caffe2::make_unique<caffe2::Predictor>(init_net, predict_net);
  LOG(INFO) << "Checking that a null forward-pass works";
  caffe2::Predictor::TensorVector outputVec;

  int w,h,c; w=227; h=227; c=3;
  caffe2::TensorCPU input(std::vector<int>({1, c, h, w}));

  //float input_data[h*w*c];
  memcpy(input.mutable_data<float>(), (float*)img2.ptr(), h * w * c * sizeof(float));

  caffe2::Predictor::TensorVector inputVec{&input};

  predictor->run(inputVec, &outputVec);

  caffe2::TensorCPU* output = outputVec[0];

  const float* output_data =  output->data<float>();
//  for (auto el : output->dims())
//    MITK_INFO << el;
//  MITK_INFO << output->size();
  int max_class = -1;
  float max_prob = 0;
  for (int i=0; i<output->size(); i++)
      if(output_data[i]>max_prob)
      {
          max_prob = output_data[i];
          max_class = i;
      }
  MITK_INFO << "Class: " << max_class;
  MITK_INFO << "Probability: " << max_prob;

  return EXIT_SUCCESS;
}

