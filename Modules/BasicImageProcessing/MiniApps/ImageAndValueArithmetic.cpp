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

#include <mitkArithmeticOperation.h>

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

  parser.setTitle("File Converter");
  parser.setCategory("Basic Image Processing");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--","-");
  // Add command line argument names
  parser.addArgument("help", "h",mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("image", "i", mitkCommandLineParser::File, "Input file:", "Input File",us::Any(),false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("value", "v", mitkCommandLineParser::Float, "Input Value:", "Input Value", us::Any(), false);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Output file", us::Any(), false, false, false, mitkCommandLineParser::Output);

  parser.addArgument("as-double", "double", mitkCommandLineParser::Bool, "Result as double", "Result as double image type", false, true);
  parser.addArgument("image-right", "right", mitkCommandLineParser::Bool, "Image right (for example Value - Image)", "Image right (for example Value - Image)", false, true);

  parser.addArgument("add", "add", mitkCommandLineParser::Bool, "Add Left Image and Right Image", "Add Left Image and Right Image", us::Any(false), true);
  parser.addArgument("subtract", "sub", mitkCommandLineParser::Bool, "Subtract right image from left image", "Subtract right image from left image", us::Any(false), true);
  parser.addArgument("multiply", "multi", mitkCommandLineParser::Bool, "Multiply Left Image and Right Image", "Multiply Left Image and Right Image", us::Any(false), true);
  parser.addArgument("divide", "div", mitkCommandLineParser::Bool, "Divide Left Image by Right Image", "Divide Left Image by Right Image", us::Any(false), true);

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
    MITK_INFO << "Loaded data is not of type image";
    return 0;
  }

  double value = us::any_cast<float>(parsedArgs["value"]);
  bool resultAsDouble = ConvertToBool(parsedArgs, "as-double");
  MITK_INFO << "Output image as double: " << resultAsDouble;

  mitk::Image::Pointer tmpImage = image->Clone();
  if (ConvertToBool(parsedArgs, "image-right"))
  {
    if (ConvertToBool(parsedArgs, "add"))
    {
      MITK_INFO << " Start Doing Operation: ADD()";
      tmpImage = mitk::ArithmeticOperation::Add(value, tmpImage, resultAsDouble);
    }
    if (ConvertToBool(parsedArgs, "subtract"))
    {
      MITK_INFO << " Start Doing Operation: SUB()";
      tmpImage = mitk::ArithmeticOperation::Subtract(value, tmpImage, resultAsDouble);
    }
    if (ConvertToBool(parsedArgs, "multiply"))
    {
      MITK_INFO << " Start Doing Operation: MULT()";
      tmpImage = mitk::ArithmeticOperation::Multiply(value, tmpImage, resultAsDouble);
    }
    if (ConvertToBool(parsedArgs, "divide"))
    {
      MITK_INFO << " Start Doing Operation: DIV()";
      tmpImage = mitk::ArithmeticOperation::Divide(value, tmpImage, resultAsDouble);
    }
  }
  else {
    if (ConvertToBool(parsedArgs, "add"))
    {
      MITK_INFO << " Start Doing Operation: ADD()";
      tmpImage = mitk::ArithmeticOperation::Add(tmpImage, value, resultAsDouble);
    }
    if (ConvertToBool(parsedArgs, "subtract"))
    {
      MITK_INFO << " Start Doing Operation: SUB()";
      tmpImage = mitk::ArithmeticOperation::Subtract(tmpImage, value, resultAsDouble);
    }
    if (ConvertToBool(parsedArgs, "multiply"))
    {
      MITK_INFO << " Start Doing Operation: MULT()";
      tmpImage = mitk::ArithmeticOperation::Multiply(tmpImage, value, resultAsDouble);
    }
    if (ConvertToBool(parsedArgs, "divide"))
    {
      MITK_INFO << " Start Doing Operation: DIV()";
      tmpImage = mitk::ArithmeticOperation::Divide(tmpImage, value, resultAsDouble);
    }

  }

  mitk::IOUtil::Save(tmpImage, outputFilename);

  return EXIT_SUCCESS;
}
