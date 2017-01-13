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

#include <mitkGlobalImageFeaturesParameter.h>


void mitk::cl::GlobalImageFeaturesParameter::AddParameter(mitkCommandLineParser &parser)
{
  parser.addArgument("image", "i", mitkCommandLineParser::InputImage, "Input Image", "Path to the input image file", us::Any(), false);
  parser.addArgument("mask", "m", mitkCommandLineParser::InputImage, "Input Mask", "Path to the mask Image that specifies the area over for the statistic (Values = 1)", us::Any(), false);
  parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output text file", "Path to output file. The output statistic is appended to this file.", us::Any(), false);

}


void mitk::cl::GlobalImageFeaturesParameter::ParseParameter(std::map<std::string, us::Any> parsedArgs)
{
  imagePath = parsedArgs["image"].ToString();
  maskPath = parsedArgs["mask"].ToString();
  outputPath = parsedArgs["output"].ToString();
}