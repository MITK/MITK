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
#include <mitkCommandLineParser.h>
#include <boost/lexical_cast.hpp>
#include <random>

// Caffe
#include <caffe/caffe.hpp>
#include <caffe/sgd_solvers.hpp>
#include <caffe/layers/memory_data_layer.hpp>
#include <memory>


// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

std::default_random_engine generator;
std::normal_distribution<double> distribution(0.0,0.1);

void predict(caffe::shared_ptr<caffe::Net<float>> network)
{
  caffe::Blob<float>* input_layer = network->input_blobs()[0];
  float* input_data = input_layer->mutable_cpu_data();

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

            input_data[counter] += distribution(generator);

            counter++;
          }

//  // not necessary here
//  input_layer->Reshape(1, num_channels, height, width);
//  network->Reshape();

  network->Forward();

  cv::Mat input_img = cv::Mat(input_layer->height(), input_layer->width(), CV_32FC1, input_data) * 100;
  input_img.convertTo(input_img, CV_8UC1);
  cv::namedWindow( "Input", cv::WINDOW_NORMAL );
  cv::imshow( "Input", input_img );

  boost::shared_ptr<caffe::Blob<float> > output_layer = network->blob_by_name("argmax");
  cv::Mat out_img = cv::Mat(input_layer->height(), input_layer->width(), CV_32FC1, output_layer->mutable_cpu_data()) * 100;
  out_img.convertTo(out_img, CV_8UC1);

  cv::namedWindow( "Output", cv::WINDOW_NORMAL );
  cv::imshow( "Output", out_img );
  cv::waitKey(0);
}

/*!
* \brief Segmentation example for Caffe in MITK
*
*/
int main(int argc, char* argv[])
{
  caffe::Caffe::set_mode(caffe::Caffe::GPU);

  mitkCommandLineParser parser;
  parser.setTitle("CaffeTest");
  parser.setCategory("MITK Diffusion Deep Learning");
  parser.setContributor("MIC");
  parser.setDescription("Minimal example for Caffe usage in MITK");
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("network_file", "", mitkCommandLineParser::String, "network_file:", ".prototxt", us::Any());
  parser.addArgument("weights_file", "", mitkCommandLineParser::String, "weights_file:", ".caffemodel", us::Any());

//  google::InitGoogleLogging(argv[0]);
//  google::SetCommandLineOption("GLOG_minloglevel", "0");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string network_file = us::any_cast<std::string>(parsedArgs["network_file"]);
  std::string weights_file = us::any_cast<std::string>(parsedArgs["weights_file"]);

  caffe::shared_ptr<caffe::Net<float>> network;
  const std::vector<std::string> stages = {"deploy"};
  network.reset(new caffe::Net<float>(network_file, caffe::TEST, 0, &stages));
  network->CopyTrainedLayersFrom(weights_file);

  predict(network);

  return EXIT_SUCCESS;
}

