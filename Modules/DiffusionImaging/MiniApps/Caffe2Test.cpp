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
#include <mitkFiberBundle.h>
#include <mitkFiberfoxParameters.h>
#include "mitkCommandLineParser.h"

#include <itkTractsToDWIImageFilter.h>
#include <boost/lexical_cast.hpp>

#include <google/protobuf/text_format.h>
#include "caffe2/core/context.h"
#include "caffe2/core/operator.h"
#include "caffe2/core/predictor.h"
#include "caffe2/core/tensor.h"
#include "caffe2/utils/math.h"

#include <gtest/gtest.h>

CAFFE2_DEFINE_string(init_net, "", "The given path to the init protobuffer.");
CAFFE2_DEFINE_string(
    predict_net,
    "",
"The given path to the predict protobuffer.");

namespace caffe2 {

void run() {
  if (FLAGS_init_net.empty()) {
    LOG(FATAL) << "No init net specified. Use --init_net=/path/to/net.";
  }
  if (FLAGS_predict_net.empty()) {
    LOG(FATAL) << "No predict net specified. Use --predict_net=/path/to/net.";
  }
  caffe2::NetDef init_net, predict_net;
  CAFFE_ENFORCE(ReadProtoFromFile("", &init_net));
  CAFFE_ENFORCE(ReadProtoFromFile("", &predict_net));
  // Can be large due to constant fills
  VLOG(1) << "Init net: " << ProtoDebugString(init_net);
  LOG(INFO) << "Predict net: " << ProtoDebugString(predict_net);
  auto predictor = caffe2::make_unique<Predictor>(init_net, predict_net);
  LOG(INFO) << "Checking that a null forward-pass works";
  Predictor::TensorVector inputVec, outputVec;
  predictor->run(inputVec, &outputVec);
  CAFFE_ENFORCE_GT(outputVec.size(), 0);
}
}

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
  parser.addArgument("input", "i", mitkCommandLineParser::String, "Input:",
                     "Input", us::Any(), false);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);


  return EXIT_SUCCESS;
}

