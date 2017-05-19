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

#include <google/protobuf/text_format.h>
#include "caffe2/core/context.h"
#include "caffe2/core/operator.h"
#include "caffe2/core/predictor.h"
#include "caffe2/core/tensor.h"
#include "caffe2/utils/math.h"

#include <gtest/gtest.h>


/*!
* \brief
*
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setTitle("Caffe2Test");
  parser.setCategory("Fiber Tracking and Processing Methods");
  parser.setContributor("MIC");
  parser.setDescription("Caffe2 test");
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("init_net", "", mitkCommandLineParser::String, "init_net:", "the network weights (.pb)", us::Any());
  parser.addArgument("predict_net", "", mitkCommandLineParser::String, "predict_net:", "the network definition (.pb)", us::Any());

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  std::string init_net_filename = us::any_cast<std::string>(parsedArgs["init_net"]);
  std::string predict_net_filename = us::any_cast<std::string>(parsedArgs["predict_net"]);

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

  float input_data[h*w*c];
  memcpy(input.mutable_data<float>(), input_data, h * w * c * sizeof(float));

  caffe2::Predictor::TensorVector inputVec{&input};

  predictor->run(inputVec, &outputVec);

  caffe2::TensorCPU* output = outputVec[0];

  const float* output_data =  output->data<float>();
//  for (auto el : output->dims())
//    MITK_INFO << el;
//  MITK_INFO << output->size();
  for (int i=0; i<output->size(); i++)
      MITK_INFO << output_data[i];

  return EXIT_SUCCESS;
}

