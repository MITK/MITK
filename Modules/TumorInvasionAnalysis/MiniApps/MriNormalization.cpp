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
  parser.setDescription(
    "Normalizes an MRI volume between medians of two given masks (e.g. ventricles and brain matter)");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--", "-");

  // Add command line argument names
  parser.addArgument("input", "i", mitkCommandLineParser::Image, "input image", "", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "output image", "", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument(
    "maxMask", "m", mitkCommandLineParser::Image, "mask of which median is set as maximal value (1)", "", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument(
    "minMask", "l", mitkCommandLineParser::Image, "mask of which median is set as minimal value (0)", "", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument(
    "excludeMask", "e", mitkCommandLineParser::Image, "region which is exluded from the other two", "", us::Any(), false, false, false, mitkCommandLineParser::Input);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  // Show a help message
  if (parsedArgs.size() == 0 || parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string outFile = us::any_cast<string>(parsedArgs["output"]);
  mitk::Image::Pointer inputFile = mitk::IOUtil::Load<mitk::Image>(us::any_cast<string>(parsedArgs["input"]));
  mitk::Image::Pointer maxMask = mitk::IOUtil::Load<mitk::Image>(us::any_cast<string>(parsedArgs["maxMask"]));
  mitk::Image::Pointer minMask = mitk::IOUtil::Load<mitk::Image>(us::any_cast<string>(parsedArgs["minMask"]));
  mitk::Image::Pointer excludeMask = mitk::IOUtil::Load<mitk::Image>(us::any_cast<string>(parsedArgs["excludeMask"]));

  SeedImage::Pointer itkMaxImage = SeedImage::New();
  SeedImage::Pointer itkMinImage = SeedImage::New();
  SeedImage::Pointer itkExcludeImage = SeedImage::New();

  FeatureImage::Pointer itkInputImage = FeatureImage::New();

  mitk::CastToItkImage(inputFile, itkInputImage);
  mitk::CastToItkImage(maxMask, itkMaxImage);
  mitk::CastToItkImage(minMask, itkMinImage);
  mitk::CastToItkImage(excludeMask, itkExcludeImage);

  std::vector<mitk::ScalarType> medianMin;
  std::vector<mitk::ScalarType> medianMax;

  itk::ImageRegionIterator<FeatureImage> inputIt(itkInputImage, itkInputImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<SeedImage> excludeIt(itkExcludeImage, itkExcludeImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<SeedImage> minIt(itkMinImage, itkMinImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<SeedImage> maxIt(itkMaxImage, itkMaxImage->GetLargestPossibleRegion());

  while (!inputIt.IsAtEnd())
  {
    if (excludeIt.Get() == 0)
    {
      if (minIt.Get() != 0)
        medianMin.push_back(inputIt.Get());

      if (maxIt.Get() != 0)
        medianMax.push_back(inputIt.Get());
    }
    ++inputIt;
    ++excludeIt;
    ++minIt;
    ++maxIt;
  }

  std::sort(medianMax.begin(), medianMax.end());
  std::sort(medianMin.begin(), medianMin.end());

  mitk::ScalarType minVal = medianMin.at(medianMin.size() / 2);
  mitk::ScalarType maxVal = medianMax.at(medianMax.size() / 2);

  inputIt.GoToBegin();
  // Create mapping
  while (!inputIt.IsAtEnd())
  {
    inputIt.Set(1.0 * (inputIt.Get() - minVal) / (maxVal - minVal));
    ++inputIt;
  }

  mitk::Image::Pointer mitkResult = mitk::Image::New();
  mitkResult = mitk::GrabItkImageMemory(itkInputImage);

  mitk::IOUtil::Save(mitkResult, outFile);

  return EXIT_SUCCESS;
}
