/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
  catch ( const us::BadAnyCastException & )
  {
    return false;
  }
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Laplacian of Gaussian");
  parser.setCategory("Basic Image Processing");
  parser.setDescription("");
  parser.setContributor("German Cancer Research Center (DKFZ)");

  parser.setArgumentPrefix("--", "-");
  // Add command line argument names
  parser.addArgument("help", "h", mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("image", "i", mitkCommandLineParser::File, "Input image:", "Input Image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Output Mask", us::Any(), false, false, false, mitkCommandLineParser::Output);

  parser.addArgument("sigma", "s", mitkCommandLineParser::Float, "Sigma for Gaussian", "Sigma for Gaussian", us::Any(), false);
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

  double sigma = us::any_cast<float>(parsedArgs["sigma"]);



  bool asDouble = ConvertToBool(parsedArgs, "as-double");
  mitk::Image::Pointer tmpImage = mitk::TransformationOperation::LaplacianOfGaussian(image, sigma, asDouble);
  mitk::IOUtil::Save(tmpImage, outputFilename);


  return EXIT_SUCCESS;
}
