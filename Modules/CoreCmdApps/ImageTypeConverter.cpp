/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCommandLineParser.h>
#include <mitkIOUtil.h>
#include <mitkImageCast.h>
#include <typeinfo>

#define CONVERT_IMAGE(TYPE, DIM)                                                                      \
  {                                                                                                   \
    MITK_INFO << "Data Type for Conversion: " << typeid(TYPE).name();                                 \
    itk::Image<TYPE, DIM>::Pointer itkImage = itk::Image<TYPE, DIM>::New();                           \
    mitk::CastToItkImage(image, itkImage);                                                            \
    mitk::CastToMitkImage(itkImage, outputImage);                                                     \
  }

#define CONVERT_IMAGE_TYPE(TYPE)                                                                      \
  {                                                                                                   \
    unsigned int dimension = image->GetDimension();                                                   \
    MITK_INFO << "Image Dimension is: " << dimension;                                                 \
    switch (dimension) {                                                                              \
      case 2 : CONVERT_IMAGE( TYPE , 2);                                                              \
               break;                                                                                 \
      case 3 : CONVERT_IMAGE( TYPE , 3);                                                              \
               break;                                                                                 \
      default: MITK_ERROR << "This tool doesn't support a dimension of "<<dimension<<".";             \
               outputImage = nullptr;                                                                 \
               break;                                                                                 \
    };                                                                                                \
  }


int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Image Type Converter");
  parser.setCategory("Basic Image Processing");
  parser.setDescription("Converts the scalar pixel type of a 2D or 3D image to a given type.");
  parser.setContributor("German Cancer Research Center (DKFZ)");

  parser.setArgumentPrefix("--","-");
  // Add command line argument names
  parser.addArgument("input", "i", mitkCommandLineParser::File, "Input file:", "Input file",us::Any(),false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file:", "Output file", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("type", "t", mitkCommandLineParser::String, "Type definition:", "Target scalar pixel type: int, uint, short, ushort, char, uchar, float, double, or none to write no output", us::Any(), false);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string inputName = us::any_cast<std::string>(parsedArgs["input"]);
  std::string outputName = us::any_cast<std::string>(parsedArgs["output"]);
  std::string type = us::any_cast<std::string>(parsedArgs["type"]);

  try
  {
    mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(inputName);
    mitk::Image::Pointer outputImage = mitk::Image::New();

    if (type.compare("int") == 0) {
      CONVERT_IMAGE_TYPE(int);
    }
    else if (type.compare("uint") == 0)
    {
      CONVERT_IMAGE_TYPE(unsigned int);
    }
    else if (type.compare("char") == 0)
    {
      CONVERT_IMAGE_TYPE(char);
    }
    else if (type.compare("uchar") == 0)
    {
      CONVERT_IMAGE_TYPE(unsigned char);
    }
    else if (type.compare("short") == 0)
    {
      CONVERT_IMAGE_TYPE(short);
    }
    else if (type.compare("ushort") == 0)
    {
      CONVERT_IMAGE_TYPE(unsigned short);
    }
    else if (type.compare("float") == 0)
    {
      CONVERT_IMAGE_TYPE(float);
    }
    else if (type.compare("double") == 0)
    {
      CONVERT_IMAGE_TYPE(double);
    }
    else if (type.compare("none") == 0)
    {
      MITK_INFO << " No conversion performed";
      return EXIT_SUCCESS;
    }
    else
    {
      MITK_ERROR << "Unknown scalar data type \"" << type << "\". Supported types: int, uint, short, ushort, char, uchar, float, double, none";
      return EXIT_FAILURE;
    }

    if (outputImage.IsNull())
      return EXIT_FAILURE;

    mitk::IOUtil::Save(outputImage, outputName);
    return EXIT_SUCCESS;
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    MITK_ERROR << "Unexpected error encountered.";
    return EXIT_FAILURE;
  }
}
