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
#ifndef mitkCLPolyToNrrd_cpp
#define mitkCLPolyToNrrd_cpp

#include "time.h"
#include <sstream>
#include <fstream>

#include <mitkIOUtil.h>
#include "mitkCommandLineParser.h"

#include "itkImageRegionIterator.h"
// MITK
#include <mitkITKImageImport.h>
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
// ITK
#include <itkLabelStatisticsImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

typedef itk::Image< double, 3 >                 FloatImageType;
typedef itk::Image< unsigned char, 3 >          MaskImageType;

template<typename TPixel, unsigned int VImageDimension>
void
  Normalize(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Image::Pointer mask0, mitk::Image::Pointer mask1, std::string output)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<int, VImageDimension> MaskType;
  typedef itk::LabelStatisticsImageFilter<ImageType, MaskType> FilterType;
  typedef itk::MinimumMaximumImageCalculator<ImageType> MinMaxComputerType;

  typename MaskType::Pointer itkMask0 = MaskType::New();
  typename MaskType::Pointer itkMask1 = MaskType::New();
  mitk::CastToItkImage(mask0, itkMask0);
  mitk::CastToItkImage(mask1, itkMask1);

  typename MinMaxComputerType::Pointer minMaxComputer = MinMaxComputerType::New();
  minMaxComputer->SetImage(itkImage);
  minMaxComputer->Compute();

  typename FilterType::Pointer labelStatisticsImageFilter = FilterType::New();
  labelStatisticsImageFilter->SetUseHistograms(true);
  labelStatisticsImageFilter->SetHistogramParameters(256, minMaxComputer->GetMinimum(),minMaxComputer->GetMaximum());
  labelStatisticsImageFilter->SetInput( itkImage );

  labelStatisticsImageFilter->SetLabelInput(itkMask0);
  labelStatisticsImageFilter->Update();
  double median0 = labelStatisticsImageFilter->GetMedian(1);

  labelStatisticsImageFilter->SetLabelInput(itkMask1);
  labelStatisticsImageFilter->Update();
  double median1 = labelStatisticsImageFilter->GetMedian(1);

  double offset = std::min(median0, median1);
  double scaling = std::max(median0, median1) - offset;
  if (scaling < 0.0001)
    return;

  itk::ImageRegionIterator<ImageType> iter(itkImage, itkImage->GetLargestPossibleRegion());
  while (! iter.IsAtEnd())
  {
    TPixel value = iter.Value();
    iter.Set((value - offset) / scaling);
    ++iter;
  }

  mitk::Image::Pointer img = mitk::ImportItkImage(itkImage);
  mitk::IOUtil::SaveImage(img, output);
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("image", "i", mitkCommandLineParser::InputImage, "Input Image", "Path to the input VTK polydata", us::Any(), false);
  parser.addArgument("mask0", "m0", mitkCommandLineParser::InputImage, "Input Mask", "The median of the area covered by this mask will be set to 0", us::Any(), false);
  parser.addArgument("mask1", "m1", mitkCommandLineParser::InputImage, "Input Mask", "The median of the area covered by this mask will be set to 1", us::Any(), false);
  parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output Image", "Target file. The output statistic is appended to this file.", us::Any(), false);

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("MR Normalization Tool");
  parser.setDescription("Normalizes a MR image. Sets the Median of the tissue covered by mask 0 to 0 and the median of the area covered by mask 1 to 1.");
  parser.setContributor("MBI");

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
  {
    return EXIT_FAILURE;
  }
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    return EXIT_SUCCESS;
  }

  mitk::Image::Pointer image = mitk::IOUtil::LoadImage(parsedArgs["image"].ToString());
  mitk::Image::Pointer mask0 = mitk::IOUtil::LoadImage(parsedArgs["mask0"].ToString());
  mitk::Image::Pointer mask1 = mitk::IOUtil::LoadImage(parsedArgs["mask1"].ToString());

  AccessByItk_3(image, Normalize, mask0, mask1, parsedArgs["output"].ToString());

  return 0;
}

#endif