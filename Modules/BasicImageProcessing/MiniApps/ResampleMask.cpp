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
#include "mitkProperties.h"

#include "mitkCommandLineParser.h"
#include "mitkIOUtil.h"

#include <mitkTransformationOperation.h>

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Mask  Outlier filtering");
  parser.setCategory("Basic Image Processing");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--", "-");
  // Add command line argument names
  parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("image", "i", mitkCommandLineParser::File, "Input image:", "Input Image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Output Mask", us::Any(), false, false, false, mitkCommandLineParser::Output);

  parser.addArgument("spacing-x", "x", mitkCommandLineParser::Float, "Spacing in x direction", "Spacing in x direction", us::Any(), false);
  parser.addArgument("spacing-y", "y", mitkCommandLineParser::Float, "Spacing in y direction", "Spacing in y direction", us::Any(), false);
  parser.addArgument("spacing-z", "z", mitkCommandLineParser::Float, "Spacing in z direction", "Spacing in z direction", us::Any(), false);

  parser.addArgument("mapping-type", "mapping", mitkCommandLineParser::Int, "0: Origin aligned, 1: Center aligned, 3: Same size", "0: Origin aligned, 1: Center aligned, 3: Same size", us::Any(), true);
  parser.addArgument("interpolator-type", "interpolator", mitkCommandLineParser::Int, "0: Linear, 1: BSpline 3, 2: Nearest Neighbour, 3: WSinc Hamming, 4: WSinc Welch", "0: Linear, 1: BSpline 3, 2: Nearest Neighbour, 3: WSinc Hamming, 4: WSinc Welch", us::Any(), true);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size() == 0)
    return EXIT_FAILURE;

  // Show a help message
  if (parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string inputFilename = us::any_cast<std::string>(parsedArgs["image"]);
  std::string outputFilename = us::any_cast<std::string>(parsedArgs["output"]);

  auto nodes = mitk::IOUtil::Load(inputFilename);
  if (nodes.size() == 0)
  {
    MITK_INFO << "No Image Loaded";
    return 0;
  }
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(nodes[0].GetPointer());

  if (image.IsNull())
  {
    MITK_INFO << "Loaded data (image) is not of type image";
    return 0;
  }

  mitk::Vector3D spacing;
  spacing[0] = us::any_cast<float>(parsedArgs["spacing-x"]);
  spacing[1] = us::any_cast<float>(parsedArgs["spacing-y"]);
  spacing[2] = us::any_cast<float>(parsedArgs["spacing-z"]);

  int interpolator = 0;
  if (parsedArgs.count("interpolator-type"))
  {
    interpolator =  us::any_cast<int>(parsedArgs["interpolator-type"]);
  }

  mitk::ImageMappingInterpolator::Type interpolatorType;
  switch (interpolator)
  {
  case 0:
    interpolatorType = mitk::ImageMappingInterpolator::Linear;
    break;

  case 1:
    interpolatorType = mitk::ImageMappingInterpolator::BSpline_3;
    break;

  case 2:
    interpolatorType = mitk::ImageMappingInterpolator::NearestNeighbor;
    break;

  case 3:
    interpolatorType = mitk::ImageMappingInterpolator::WSinc_Hamming;
    break;

  case 4:
    interpolatorType = mitk::ImageMappingInterpolator::WSinc_Welch;
    break;

  default:
    interpolatorType = mitk::ImageMappingInterpolator::Linear;
  }

  int position = 0;
  if (parsedArgs.count("mapping-type"))
  {
    position = us::any_cast<int>(parsedArgs["mapping-type"]);
  }

  mitk::GridInterpolationPositionType gridPosition;
  switch (position)
  {
  case 0:
    gridPosition = mitk::GridInterpolationPositionType::OriginAligned;
    break;

  case 1:
    gridPosition = mitk::GridInterpolationPositionType::CenterAligned;
    break;

  case 2:
    gridPosition = mitk::GridInterpolationPositionType::SameSize;
    break;

  default:
    gridPosition = mitk::GridInterpolationPositionType::OriginAligned;
  }

  mitk::Image::Pointer tmpImage = mitk::TransformationOperation::ResampleMask(image, spacing, interpolatorType, gridPosition);
  mitk::IOUtil::Save(tmpImage, outputFilename);


  return EXIT_SUCCESS;
}
