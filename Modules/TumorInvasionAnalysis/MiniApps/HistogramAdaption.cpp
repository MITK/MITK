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
#include "mitkIOUtil.h"
#include "mitkImageCast.h"
#include <mitkCommandLineParser.h>
#include <mitkITKImageImport.h>

using namespace std;

typedef itk::Image<unsigned char, 3> SeedImage;
typedef itk::Image<mitk::ScalarType, 3> FeatureImage;

typedef itk::ImageRegionIterator<SeedImage> SeedIteratorType;
typedef itk::ImageRegionIterator<FeatureImage> FeatureIteratorType;

int main(int argc, char *argv[])
{
  mitkCommandLineParser parser;
  parser.setTitle("Contrast Adaption");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("Stretches or pushes image intensities above a given threshold");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--", "-");

  // Add command line argument names
  parser.addArgument("input", "i", mitkCommandLineParser::Image, "input image", "", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "output image", "", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("cutOff", "c", mitkCommandLineParser::Float, "value at which different slope is to be applied");
  parser.addArgument(
    "slope", "s", mitkCommandLineParser::Float, "slope to be applied (total delta to max value starting from cutOff)");

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size() == 0)
    return EXIT_FAILURE;

  // Show a help message
  if (parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string outFile = us::any_cast<string>(parsedArgs["output"]);
  mitk::Image::Pointer inputFile = mitk::IOUtil::Load<mitk::Image>(us::any_cast<string>(parsedArgs["input"]));
  float cutOff = us::any_cast<float>(parsedArgs["cutOff"]);
  float slope = us::any_cast<float>(parsedArgs["slope"]);

  FeatureImage::Pointer itkInputImage = FeatureImage::New();

  mitk::CastToItkImage(inputFile, itkInputImage);
  double max = -99999;

  itk::ImageRegionIterator<FeatureImage> inputIt(itkInputImage, itkInputImage->GetLargestPossibleRegion());

  while (!inputIt.IsAtEnd())
  {
    if (inputIt.Get() > max)
      max = inputIt.Get();

    ++inputIt;
  }

  inputIt.GoToBegin();
  // Mapping
  while (!inputIt.IsAtEnd())
  {
    if (inputIt.Get() > cutOff)
    {
      inputIt.Set(cutOff + slope * (inputIt.Get() - cutOff) / (max - cutOff));
    }
    ++inputIt;
  }

  mitk::Image::Pointer mitkResult = mitk::Image::New();
  // !! CastToItk behaves very differently depending on the original data type
  // if the target type is the same as the original, only a pointer to the data is set
  // and an additional GrabItkImageMemory will cause a segfault when the image is destroyed
  // GrabItkImageMemory - is not necessary in this case since we worked on the original data
  // See Bug 17538.
  if (inputFile->GetPixelType().GetComponentTypeAsString() != "double")
    mitkResult = mitk::GrabItkImageMemory(itkInputImage);
  else
    mitkResult = inputFile;

  mitk::IOUtil::Save(mitkResult, outFile);
  return EXIT_SUCCESS;
}
