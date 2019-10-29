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
  parser.addArgument("number-of-bands", "bands", mitkCommandLineParser::Int, "Numbers of pyramid levels", "Number of pyramid levels", us::Any(), false);

  parser.addArgument("wavelet", "w", mitkCommandLineParser::Int, "0: Shannon, 1: Simocelli, 2: Vow, 3: Held", "0: Shannon, 1: Simocelli, 2: Vow, 3: Held", us::Any(), false);
  parser.addArgument("border-condition", "border", mitkCommandLineParser::Int, "0: Constant, 1: Periodic, 2: Zero Flux Neumann", "0: Constant, 1: Periodic, 2: Zero Flux Neumann", us::Any(), false);

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

  int levels = us::any_cast<int>(parsedArgs["number-of-levels"]);
  int bands = us::any_cast<int>(parsedArgs["number-of-bands"]);

  mitk::BorderCondition condition = mitk::BorderCondition::Constant;
  mitk::WaveletType waveletType = mitk::WaveletType::Held;
  switch (us::any_cast<int>(parsedArgs["wavelet"]))
  {
  case 0:
    waveletType = mitk::WaveletType::Shannon;
    break;
  case 1:
    waveletType = mitk::WaveletType::Simoncelli;
    break;
  case 2:
    waveletType = mitk::WaveletType::Vow;
    break;
  case 3:
    waveletType = mitk::WaveletType::Held;
    break;
  default:
    waveletType = mitk::WaveletType::Shannon;
    break;
  }
  switch (us::any_cast<int>(parsedArgs["border-condition"]))
  {
  case 0:
    condition = mitk::BorderCondition::Constant;
    break;
  case 1:
    condition = mitk::BorderCondition::Periodic;
    break;
  case 2:
    condition = mitk::BorderCondition::ZeroFluxNeumann;
    break;
  default:
    condition = mitk::BorderCondition::Constant;
    break;
  }

  std::vector<mitk::Image::Pointer> results = mitk::TransformationOperation::WaveletForward(image, levels, bands, condition, waveletType);
  unsigned int level = 0;
  for (auto image : results)
  {
    std::string name = outputFilename + us::Any(level).ToString() + outputExtension;
    MITK_INFO << "Saving to " << name;
    mitk::IOUtil::Save(image, name);
    ++level;
  }

  return EXIT_SUCCESS;
}
