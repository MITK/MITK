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

caffe::shared_ptr<caffe::Net<float>> train(caffe::Solver<float>* solver)
{
  int num_samples = 64;
  int num_features = 100;
  float *data = new float[1*1*num_features*num_samples];
  float *label = new float[1*1*1*num_samples];
  float *directions = new float[1*1*3*num_samples];

  caffe::MemoryDataLayer<float> *image_input_layer = (caffe::MemoryDataLayer<float> *) (solver->net()->layer_by_name("image_data").get());
  caffe::MemoryDataLayer<float> *target_input_layer = (caffe::MemoryDataLayer<float> *) (solver->net()->layer_by_name("target_data").get());

  image_input_layer->Reset(data, label, num_samples);
  target_input_layer->Reset(directions, label, num_samples);
  solver->Solve();
  return solver->net();
}

caffe::Solver<float>* read_solver(std::string solver_file)
{
  caffe::SolverParameter solver_param;
  caffe::ReadSolverParamsFromTextFileOrDie(solver_file, &solver_param);
  return new caffe::AdamSolver<float>(solver_param);
}

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

std::vector<float> predict_data_layer(const cv::Mat& img, caffe::shared_ptr<caffe::Net<float>> network) {
  caffe::Blob<float>* input_layer = network->input_blobs()[0];

  assert(img.channels() == input_layer->channels());
  assert(img.cols == input_layer->width());
  assert(img.rows == input_layer->height());

  cv::Mat img2;
  // convert to float
  if (img.channels()==3 && img.type()!=CV_32FC3)
      img.convertTo(img2, CV_32FC3);
  else if(img.channels()==1 && img.type()!=CV_32FC1)
      img.convertTo(img2, CV_32FC1);

  float* input_data = input_layer->mutable_cpu_data();  // pointer to the memory data layer
  std::vector<cv::Mat> bgr;
  for (int i = 0; i < img2.channels(); ++i) {
    cv::Mat channel(input_layer->height(), input_layer->width(), CV_32FC1, input_data);
    bgr.push_back(channel);
    input_data += input_layer->width() * input_layer->height();
  }
  cv::split(img2, bgr);

  network->Forward();

  /* Copy the output layer to a std::vector */
  caffe::Blob<float>* output_layer = network->output_blobs()[0];
  const float* begin = output_layer->cpu_data();
  const float* end = begin + output_layer->channels();
  return std::vector<float>(begin, end);
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
  parser.addArgument("image", "", mitkCommandLineParser::String, "image:", "", us::Any());
  parser.addArgument("solver_file", "", mitkCommandLineParser::String, "solver_file:", "the caffe solver file (.prototxt)", us::Any());
  parser.addArgument("network_file", "", mitkCommandLineParser::String, "network_file:", ".prototxt", us::Any());
  parser.addArgument("weights_file", "", mitkCommandLineParser::String, "weights_file:", ".caffemodel", us::Any());

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string image_filename = us::any_cast<std::string>(parsedArgs["image"]);
  cv::Mat img = cv::imread(image_filename, CV_LOAD_IMAGE_GRAYSCALE);
//  cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
//  cv::imshow( "Display window", img );                   // Show our image inside it.
//  cv::waitKey(0);                                          // Wait for a keystroke in the window

  //// TRAINING
//  std::string solver_file = us::any_cast<std::string>(parsedArgs["solver_file"]);
//  caffe::Solver<float>* solver = read_solver(solver_file);
//  caffe::shared_ptr<caffe::Net<float>> trained_net = train(solver);
//  const float* test_output = test(trained_net);

  std::string network_file = us::any_cast<std::string>(parsedArgs["network_file"]);
  std::string weights_file = us::any_cast<std::string>(parsedArgs["weights_file"]);

  caffe::shared_ptr<caffe::Net<float>> network;
  const std::vector<std::string> stages = {"deploy"};
  network.reset(new caffe::Net<float>(network_file, caffe::TEST, 0, &stages));
  network->CopyTrainedLayersFrom(weights_file);

  MITK_INFO << "predicting";
  predict(network);
  MITK_INFO << "DONE";


  return EXIT_SUCCESS;
}

