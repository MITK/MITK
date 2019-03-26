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

#include <mitkIOUtil.h>

#include "mitkCommandLineParser.h"
#include "mitkITKImageImport.h"
#include "mitkImage.h"
#include <mitkImageCast.h>

#include "itkSignedDanielssonDistanceMapImageFilter.h"

using namespace std;

typedef itk::Image<unsigned char, 3> BinaryType;
typedef itk::Image<mitk::ScalarType, 3> ResultType;

int main(int argc, char *argv[])
{
  mitkCommandLineParser parser;
  parser.setTitle("Basic Distance Maps");
  parser.setCategory("Features");
  parser.setDescription("Creates Eucledian Distance Maps of a given ROI segmentation");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("input", "i", mitkCommandLineParser::Image, "(binary) seed file", "", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "distance map file name", "", us::Any(), false, false, false, mitkCommandLineParser::Output);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  // Show a help message
  if (parsedArgs.size() == 0 || parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string resultFile = us::any_cast<string>(parsedArgs["output"]);
  std::string seedFile = us::any_cast<string>(parsedArgs["input"]);

  mitk::Image::Pointer seedImage = mitk::IOUtil::Load<mitk::Image>(seedFile);

  BinaryType::Pointer itkSeed = BinaryType::New();

  mitk::CastToItkImage(seedImage, itkSeed);

  itk::SignedDanielssonDistanceMapImageFilter<BinaryType, ResultType>::Pointer danielssonDistance =
    itk::SignedDanielssonDistanceMapImageFilter<BinaryType, ResultType>::New();
  danielssonDistance->SetInput(itkSeed);
  danielssonDistance->SetUseImageSpacing(true);
  danielssonDistance->Update();

  mitk::Image::Pointer result = mitk::Image::New();
  mitk::GrabItkImageMemory(danielssonDistance->GetOutput(), result);
  mitk::IOUtil::Save(result, resultFile);

  return EXIT_SUCCESS;
}
