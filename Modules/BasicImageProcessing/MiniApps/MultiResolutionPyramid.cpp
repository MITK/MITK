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

static bool ConvertToBool(std::map<std::string, us::Any> &data, std::string name)
{
  if (!data.count(name))
  {
    return false;
  }
  try {
    return us::any_cast<bool>(data[name]);
  }
  catch (us::BadAnyCastException &)
  {
    return false;
  }
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Multi-Resolution Pyramid");
  parser.setCategory("Basic Image Processing");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--", "-");
  // Add command line argument names
  parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("image", "i", mitkCommandLineParser::File, "Input image:", "Input Image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Output Mask", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("output-extension", "e", mitkCommandLineParser::File, "Output file:", "Output Mask", us::Any(), false, false, false, mitkCommandLineParser::Output);

  parser.addArgument("number-of-levels", "levels", mitkCommandLineParser::Int, "Numbers of pyramid levels", "Number of pyramid levels", us::Any(), false);
  parser.addArgument("as-double", "double", mitkCommandLineParser::Bool, "Result Image as Type Double", "Result Image as Type Double", us::Any(false), true);

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
  std::string outputExtension = us::any_cast<std::string>(parsedArgs["output-extension"]);

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

  int levels = us::any_cast<float>(parsedArgs["number-of-levels"]);
  bool asDouble = ConvertToBool(parsedArgs, "as-double");

  std::vector<mitk::Image::Pointer> results = mitk::TransformationOperation::MultiResolution(image, levels, asDouble);
  unsigned int level = 1;
  for (auto image : results)
  {
    std::string name = outputFilename + us::Any(levels - level).ToString() + outputExtension;
    mitk::IOUtil::Save(image, name);
    ++level;
  }

  return EXIT_SUCCESS;
}
