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
  parser.addArgument("input", "i", mitkCommandLineParser::File, "Input file:", "Input File",us::Any(),false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Output file", us::Any(), false, false, false, mitkCommandLineParser::Output);

  parser.addArgument("as-double", "double", mitkCommandLineParser::Bool, "Result as double", "Result as double image type", false, true);

  parser.addArgument("tan", "tan", mitkCommandLineParser::Bool, "Calculate tan operation", "Calculate tan operation", us::Any(false), true);
  parser.addArgument("atan", "atan", mitkCommandLineParser::Bool, "Calculate atan operation", "Calculate atan operation", us::Any(false), true);
  parser.addArgument("cos", "cos", mitkCommandLineParser::Bool, "Calculate cos operation", "Calculate cos operation", us::Any(false), true);
  parser.addArgument("acos", "acos", mitkCommandLineParser::Bool, "Calculate acos operation", "Calculate acos operation", us::Any(false), true);
  parser.addArgument("sin", "sin", mitkCommandLineParser::Bool, "Calculate sin operation", "Calculate sin operation", us::Any(false), true);
  parser.addArgument("asin", "asin", mitkCommandLineParser::Bool, "Calculate asin operation", "Calculate asin operation", us::Any(false), true);
  parser.addArgument("square", "square", mitkCommandLineParser::Bool, "Calculate square operation", "Calculate square operation", us::Any(false), true);
  parser.addArgument("sqrt", "sqrt", mitkCommandLineParser::Bool, "Calculate sqrt operation", "Calculate sqrt operation", us::Any(false), true);
  parser.addArgument("abs", "abs", mitkCommandLineParser::Bool, "Calculate abs operation", "Calculate abs operation", us::Any(false), true);
  parser.addArgument("exp", "exp", mitkCommandLineParser::Bool, "Calculate exp operation", "Calculate exp operation", us::Any(false), true);
  parser.addArgument("expneg", "expneg", mitkCommandLineParser::Bool, "Calculate expneg operation", "Calculate expneg operation", us::Any(false), true);
  parser.addArgument("log10", "log10", mitkCommandLineParser::Bool, "Calculate log10 operation", "Calculate log10 operation", us::Any(false), true);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
      return EXIT_FAILURE;

  // Show a help message
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string inputFilename = us::any_cast<std::string>(parsedArgs["input"]);
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

  bool resultAsDouble = ConvertToBool(parsedArgs, "as-double");
  MITK_INFO << "Output image as double: " << resultAsDouble;

  mitk::Image::Pointer tmpImage = image->Clone();

  if (ConvertToBool(parsedArgs, "tan"))
  {
    MITK_INFO << " Start Doing Operation: TAN()";
    tmpImage = mitk::ArithmeticOperation::Tan(tmpImage, resultAsDouble);
  }
  if (ConvertToBool(parsedArgs, "atan"))
  {
    MITK_INFO << " Start Doing Operation: ATAN()";
    tmpImage = mitk::ArithmeticOperation::Atan(tmpImage, resultAsDouble);
  }
  if (ConvertToBool(parsedArgs, "cos"))
  {
    MITK_INFO << " Start Doing Operation: COS()";
    tmpImage = mitk::ArithmeticOperation::Cos(tmpImage, resultAsDouble);
  }
  if (ConvertToBool(parsedArgs, "acos"))
  {
    MITK_INFO << " Start Doing Operation: ACOS()";
    tmpImage = mitk::ArithmeticOperation::Acos(tmpImage, resultAsDouble);
  }
  if (ConvertToBool(parsedArgs, "sin"))
  {
    MITK_INFO << " Start Doing Operation: SIN()";
    tmpImage = mitk::ArithmeticOperation::Sin(tmpImage, resultAsDouble);
  }
  if (ConvertToBool(parsedArgs, "asin"))
  {
    MITK_INFO << " Start Doing Operation: ASIN()";
    tmpImage = mitk::ArithmeticOperation::Asin(tmpImage, resultAsDouble);
  }
  if (ConvertToBool(parsedArgs, "square"))
  {
    MITK_INFO << " Start Doing Operation: SQUARE()";
    tmpImage = mitk::ArithmeticOperation::Square(tmpImage, resultAsDouble);
  }
  if (ConvertToBool(parsedArgs, "sqrt"))
  {
    MITK_INFO << " Start Doing Operation: SQRT()";
    tmpImage = mitk::ArithmeticOperation::Sqrt(tmpImage, resultAsDouble);
  }
  if (ConvertToBool(parsedArgs, "abs"))
  {
    MITK_INFO << " Start Doing Operation: ABS()";
    tmpImage = mitk::ArithmeticOperation::Abs(tmpImage, resultAsDouble);
  }
  if (ConvertToBool(parsedArgs, "exp"))
  {
    MITK_INFO << " Start Doing Operation: EXP()";
    tmpImage = mitk::ArithmeticOperation::Exp(tmpImage, resultAsDouble);
  }
  if (ConvertToBool(parsedArgs, "expneg"))
  {
    MITK_INFO << " Start Doing Operation: EXPNEG()";
    tmpImage = mitk::ArithmeticOperation::ExpNeg(tmpImage, resultAsDouble);
  }
  if (ConvertToBool(parsedArgs, "log10"))
  {
    MITK_INFO << " Start Doing Operation: LOG10()";
    tmpImage = mitk::ArithmeticOperation::Log10(tmpImage, resultAsDouble);
  }

  mitk::IOUtil::Save(tmpImage, outputFilename);

  return EXIT_SUCCESS;
}
