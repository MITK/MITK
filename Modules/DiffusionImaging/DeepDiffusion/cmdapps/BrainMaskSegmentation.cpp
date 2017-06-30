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
#include <mitkImageStatisticsCalculator.h>

#include <itkCropImageFilter.h>
#include <itkPadImageFilter.h>
#include <itkShiftScaleImageFilter.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>
#include <itkResampleImageFilter.h>
#include <itkNormalizeImageSlicesFilter.h>

// Caffe
#include <caffe/caffe.hpp>
#include <caffe/sgd_solvers.hpp>
#include <caffe/layers/memory_data_layer.hpp>
#include <memory>

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <omp.h>

std::default_random_engine generator;
std::normal_distribution<double> distribution(0.0,0.1);

itk::Image< float, 3 >::Pointer predict(caffe::shared_ptr<caffe::Net<float>> network, itk::Image< float, 3 >::Pointer input_image)
{
  itk::Image< float, 3 >::Pointer outImage = itk::Image< float, 3 >::New();
  outImage->SetSpacing( input_image->GetSpacing() );   // Set the image spacing
  outImage->SetOrigin( input_image->GetOrigin() );     // Set the image origin
  outImage->SetDirection( input_image->GetDirection() );  // Set the image direction
  outImage->SetLargestPossibleRegion( input_image->GetLargestPossibleRegion());
  outImage->SetBufferedRegion(input_image->GetLargestPossibleRegion());
  outImage->SetRequestedRegion( input_image->GetLargestPossibleRegion());
  outImage->Allocate();
  outImage->FillBuffer(0);

  itk::Size<3> image_size = input_image->GetLargestPossibleRegion().GetSize();
  caffe::Blob<float>* input_layer = network->input_blobs()[0];

  int padX = 0;
  int padY = 0;
  if (image_size[0]>256)
      padX = (image_size[0]-256)/2;
  if (image_size[2]>256)
      padY = (image_size[2]-256)/2;

  input_layer->Reshape(1, 1, 256, 256);
  network->Reshape();

  MITK_INFO << "Predicting";
  for (unsigned int slice = 0; slice<image_size[1]; slice++)
  {
    MITK_INFO << "Slice " << slice;
    float* input_data = input_layer->mutable_cpu_data();
    int counter = 0;

    for (int x = 0; x < input_layer->height(); x++)
      for (int y = 0; y < input_layer->width(); y++)
      {
        itk::Image< float, 3 >::IndexType idx;
        idx[0] = padY+y;
        idx[1] = slice;
        idx[2] = input_layer->height() - (padX+x) - 1;
        if (input_image->GetLargestPossibleRegion().IsInside(idx))
          input_data[counter] = input_image->GetPixel(idx);
        else
          input_data[counter] = 0;
        counter++;
      }

    network->Forward();

//    cv::Mat input_img = cv::Mat(input_layer->height(), input_layer->width(), CV_32FC1, input_data)*50 + 128;
//    input_img.convertTo(input_img, CV_8UC1);
//    cv::namedWindow( "Input", cv::WINDOW_NORMAL );
//    cv::imshow( "Input", input_img );

    boost::shared_ptr<caffe::Blob<float> > output_layer = network->blob_by_name("argmax");
    float* output_data = output_layer->mutable_cpu_data();
    cv::Mat out_img = cv::Mat(output_layer->height(), output_layer->width(), CV_32FC1, output_data) * 50;
    out_img.convertTo(out_img, CV_8UC1);

//    cv::namedWindow( "Output", cv::WINDOW_NORMAL );
//    cv::imshow( "Output", out_img );
//    cv::waitKey(1);

    counter = 0;
    for (int x = 0; x < output_layer->height(); x++)
      for (int y = 0; y < output_layer->width(); y++)
      {
        itk::Image< float, 3 >::IndexType idx;
        idx[0] = y;
        idx[1] = slice;
        idx[2] = output_layer->height() - x - 1;

        if (outImage->GetLargestPossibleRegion().IsInside(idx))
          outImage->SetPixel(idx, output_data[counter]);
        counter++;
      }
  }

  return outImage;
}

/*!
* \brief Segmentation example for Caffe in MITK
*
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setTitle("CaffeTest");
  parser.setCategory("Fiber Tracking and Processing Methods");
  parser.setContributor("MIC");
  parser.setDescription("Barin mask extraction");
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("input_image", "i", mitkCommandLineParser::String, "Input image:", "Input image to segment (T1 or T2)", us::Any(), false);
  parser.addArgument("output_image", "o", mitkCommandLineParser::String, "Output image:", "Output segmentation", us::Any(), false);
  parser.addArgument("network_file", "", mitkCommandLineParser::String, "network_file:", ".prototxt", us::Any(), false);
  parser.addArgument("weights_file", "", mitkCommandLineParser::String, "weights_file:", ".caffemodel", us::Any(), false);
  parser.addArgument("use_cpu", "", mitkCommandLineParser::Bool, "Use CPU:", "Use CPU for prediction. Default is GPU.");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string network_file = us::any_cast<std::string>(parsedArgs["network_file"]);
  std::string weights_file = us::any_cast<std::string>(parsedArgs["weights_file"]);
  std::string input_image_file = us::any_cast<std::string>(parsedArgs["input_image"]);
  std::string output_image_file = us::any_cast<std::string>(parsedArgs["output_image"]);

  if (parsedArgs.count("use_cpu"))
    caffe::Caffe::set_mode(caffe::Caffe::CPU);
  else
    caffe::Caffe::set_mode(caffe::Caffe::GPU);

  mitk::Image::Pointer input_input_image = mitk::IOUtil::LoadImage(input_image_file);

//  mitk::ImageStatisticsCalculator::Pointer statisticsCalculator = mitk::ImageStatisticsCalculator::New();
//  statisticsCalculator->SetInputImage(t1_input_image);
//  statisticsCalculator->SetNBinsForHistogramStatistics(1000);
//  mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer statistics = statisticsCalculator->GetStatistics();
//  float percentile = statistics->GetHistogram()->Quantile(0, 0.95);

  typedef itk::Image< float, 3 >    FloatImgType;
  FloatImgType::Pointer float_img = FloatImgType::New();
  mitk::CastToItkImage(input_input_image, float_img);

  typedef itk::ResampleImageFilter< FloatImgType, FloatImgType > ResamplerType;

  FloatImgType::SpacingType image_spacing = float_img->GetSpacing();
  FloatImgType::SpacingType target_spacing; target_spacing.Fill(0.7);
  if (fabs(image_spacing[0]-target_spacing[0]) > mitk::eps || fabs(image_spacing[1]-target_spacing[1]) > mitk::eps || fabs(image_spacing[2]-target_spacing[2]) > mitk::eps)
  {
    MITK_INFO << "Resampling";
    itk::Vector< double, 3 > sampling;
    itk::ImageRegion<3>   newRegion;
    sampling[0] = image_spacing[0]/target_spacing[0];
    sampling[1] = image_spacing[1]/target_spacing[1];
    sampling[2] = image_spacing[2]/target_spacing[2];
    newRegion = float_img->GetLargestPossibleRegion();
    newRegion.SetSize(0, newRegion.GetSize(0)*sampling[0]);
    newRegion.SetSize(1, newRegion.GetSize(1)*sampling[1]);
    newRegion.SetSize(2, newRegion.GetSize(2)*sampling[2]);

    itk::Point<double,3> newOrigin = float_img->GetOrigin();
    newOrigin[0] -= image_spacing[0]/2;
    newOrigin[1] -= image_spacing[1]/2;
    newOrigin[2] -= image_spacing[2]/2;
    newOrigin[0] += target_spacing[0]/2;
    newOrigin[1] += target_spacing[1]/2;
    newOrigin[2] += target_spacing[2]/2;

    typename FloatImgType::Pointer newImage = FloatImgType::New();
    newImage->SetSpacing( target_spacing );
    newImage->SetOrigin( newOrigin );
    newImage->SetDirection( float_img->GetDirection() );
    newImage->SetLargestPossibleRegion( newRegion );
    newImage->SetBufferedRegion( newRegion );
    newImage->SetRequestedRegion( newRegion );
    newImage->Allocate();

    ResamplerType::Pointer resampler = ResamplerType::New();
    resampler->SetInput(float_img);
    resampler->SetOutputParametersFromImage(newImage);
    resampler->Update();
    float_img = resampler->GetOutput();
  }

  MITK_INFO << "Normalizing";
  itk::NormalizeImageSlicesFilter< float >::Pointer normalizer = itk::NormalizeImageSlicesFilter< float >::New();
  normalizer->SetInput(float_img);
  normalizer->SetAxis(1);
  normalizer->Update();
  float_img = normalizer->GetOutput();

  caffe::shared_ptr<caffe::Net<float>> network;
  const std::vector<std::string> stages = {"deploy"};
  network.reset(new caffe::Net<float>(network_file, caffe::TEST, 0, &stages));
  network->CopyTrainedLayersFrom(weights_file);

  itk::Image< float, 3 >::Pointer output_image = predict(network, float_img);

  typedef  itk::ImageFileWriter< itk::Image< float, 3 >  > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(output_image_file);
  writer->SetInput(output_image);
  writer->Update();

  return EXIT_SUCCESS;
}

