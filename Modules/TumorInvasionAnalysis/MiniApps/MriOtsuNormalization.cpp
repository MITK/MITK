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

#include "itkImageRegionIterator.h"
#include "mitkCommandLineParser.h"
#include "mitkIOUtil.h"
#include "mitkImageCast.h"
#include <mitkITKImageImport.h>

// ITK
#include <itkOtsuMultipleThresholdsImageFilter.h>

using namespace std;

typedef itk::Image<unsigned char, 3> SeedImage;
typedef itk::Image<mitk::ScalarType, 3> FeatureImage;

typedef itk::ImageRegionIterator<SeedImage> SeedIteratorType;
typedef itk::ImageRegionIterator<FeatureImage> FeatureIteratorType;

int main(int argc, char *argv[])
{
  mitkCommandLineParser parser;
  parser.setTitle("Mri Normalization");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("Normalizes an MRI volume based on regions determined by Otsu.");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--", "-");

  // Add command line argument names
  parser.addArgument("input", "i", mitkCommandLineParser::Image, "input image", "input image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "output image", "output image", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("bins", "b", mitkCommandLineParser::Int, "number of regions (bins)", "number of regions (bins)");

  parser.addArgument(
    "minBin", "l", mitkCommandLineParser::Int, "bin of which median is set to 0", "bin of which median is set to 0");
  parser.addArgument(
    "maxBin", "h", mitkCommandLineParser::Int, "bin of which median is set to 1", "bin of which median is set to 1");

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  // Show a help message
  if (parsedArgs.size() == 0 || parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string outFile = us::any_cast<string>(parsedArgs["output"]);
  mitk::Image::Pointer inputFile = mitk::IOUtil::Load<mitk::Image>(us::any_cast<string>(parsedArgs["input"]));
  int numberOfThresholds = us::any_cast<int>(parsedArgs["bins"]);
  int minBin = us::any_cast<int>(parsedArgs["minBin"]);
  int maxBin = us::any_cast<int>(parsedArgs["maxBin"]);

  FeatureImage::Pointer itkInputImage = FeatureImage::New();
  mitk::CastToItkImage(inputFile, itkInputImage);

  typedef itk::OtsuMultipleThresholdsImageFilter<FeatureImage, FeatureImage> FilterType;
  FilterType::Pointer otsuFilter = FilterType::New();
  otsuFilter->SetInput(itkInputImage);
  otsuFilter->SetNumberOfThresholds(numberOfThresholds - 1);
  otsuFilter->Update();

  FeatureImage::Pointer itkLabelImage = otsuFilter->GetOutput();

  std::vector<mitk::ScalarType> medianMin;
  std::vector<mitk::ScalarType> medianMax;

  itk::ImageRegionIterator<FeatureImage> inputIt(itkInputImage, itkInputImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<FeatureImage> labelIt(itkLabelImage, itkLabelImage->GetLargestPossibleRegion());

  while (!inputIt.IsAtEnd())
  {
    if (labelIt.Get() == minBin)
    {
      medianMin.push_back(inputIt.Get());
    }
    else if (labelIt.Get() == maxBin)
    {
      medianMax.push_back(inputIt.Get());
    }
    ++inputIt;
    ++labelIt;
  }

  std::sort(medianMax.begin(), medianMax.end());
  std::sort(medianMin.begin(), medianMin.end());

  mitk::ScalarType minVal = medianMin.at(medianMin.size() / 2);
  mitk::ScalarType maxVal = medianMax.at(medianMax.size() / 2);

  inputIt.GoToBegin();
  // labelIt.GoToBegin();
  // Create mapping
  while (!inputIt.IsAtEnd())
  {
    inputIt.Set(1.0 * (inputIt.Get() - minVal) / (maxVal - minVal));
    // inputIt.Set(labelIt.Get());
    ++inputIt;

    //++labelIt;
  }

  mitk::Image::Pointer mitkResult = mitk::Image::New();
  mitkResult = mitk::GrabItkImageMemory(itkInputImage);

  mitk::IOUtil::Save(mitkResult, outFile);

  return EXIT_SUCCESS;
}
