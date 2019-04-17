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

#include <mitkMaskCleaningOperation.h>


int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Mask  Outlier filtering");
  parser.setCategory("Basic Image Processing");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--","-");
  // Add command line argument names
  parser.addArgument("help", "h",mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("image", "i", mitkCommandLineParser::File, "Input image:", "Input Image",us::Any(),false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mask", "m", mitkCommandLineParser::File, "Input mask:", "Input Mask", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Output Mask", us::Any(), false, false, false, mitkCommandLineParser::Output);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
      return EXIT_FAILURE;

  // Show a help message
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string inputFilename = us::any_cast<std::string>(parsedArgs["image"]);
  std::string maskFilename = us::any_cast<std::string>(parsedArgs["mask"]);
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


  auto maskNodes = mitk::IOUtil::Load(maskFilename);
  if (maskNodes.size() == 0)
  {
    MITK_INFO << "No Mask Loaded";
    return 0;
  }
  mitk::Image::Pointer mask = dynamic_cast<mitk::Image*>(maskNodes[0].GetPointer());

  if (image.IsNull())
  {
    MITK_INFO << "Loaded data (mask) is not of type image";
    return 0;
  }

  mitk::Image::Pointer tmpImage = mitk::MaskCleaningOperation::MaskOutlierFiltering(image, mask);

  mitk::IOUtil::Save(tmpImage, outputFilename);

  return EXIT_SUCCESS;
}
