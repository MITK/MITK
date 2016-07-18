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

#include "mitkCommandLineParser.h"
#include "mitkIOUtil.h"
#include <mitkImageCast.h>
#include <typeinfo>

#define CONVERT_IMAGE(TYPE, DIM) itk::Image<TYPE, DIM>::Pointer itkImage = itk::Image<TYPE, DIM>::New();    \
  MITK_INFO << "Data Type for Conversion: "<< typeid(TYPE).name();                                    \
  mitk::CastToItkImage(image, itkImage);                                                              \
  mitk::CastToMitkImage(itkImage, outputImage)

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Image Type Converter");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("");
  parser.setContributor("MBI");

  parser.setArgumentPrefix("--","-");
  // Add command line argument names
  parser.addArgument("help", "h",mitkCommandLineParser::Bool, "Help:", "Show this help text");
  parser.addArgument("input", "i", mitkCommandLineParser::InputDirectory, "Input file:", "Input file",us::Any(),false);
  parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output file:", "Output file", us::Any(), false);
  parser.addArgument("type", "t", mitkCommandLineParser::OutputFile, "Type definition:", "Define Scalar data type: int, uint, short, ushort, char, uchar, float, double", us::Any(), false);

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // Show a help message
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    std::cout << parser.helpText();
    return EXIT_SUCCESS;
  }

  std::string inputName = us::any_cast<string>(parsedArgs["input"]);
  std::string outputName = us::any_cast<string>(parsedArgs["output"]);
  std::string type = us::any_cast<string>(parsedArgs["type"]);

  mitk::Image::Pointer image = mitk::IOUtil::LoadImage(inputName);
  mitk::Image::Pointer outputImage = mitk::Image::New();

  if (type.compare("int") == 0)
  {
    CONVERT_IMAGE(int, 3);
  }
  else if (type.compare("uint") == 0)
  {
    CONVERT_IMAGE(unsigned int, 3);
  }
  else if (type.compare("char") == 0)
  {
    CONVERT_IMAGE(char, 3);
  }
  else if (type.compare("uchar") == 0)
  {
    CONVERT_IMAGE(unsigned char, 3);
  }
  else if (type.compare("short") == 0)
  {
    CONVERT_IMAGE(short, 3);
  }
  else if (type.compare("ushort") == 0)
  {
    CONVERT_IMAGE(unsigned short, 3);
  }
  else if (type.compare("float") == 0)
  {
    CONVERT_IMAGE(float, 3);
  }
  else if (type.compare("double") == 0)
  {
    CONVERT_IMAGE(double, 3);
  }
  else
  {
    CONVERT_IMAGE(double, 3);
  }


  mitk::IOUtil::SaveImage(outputImage, outputName);

  return EXIT_SUCCESS;
}