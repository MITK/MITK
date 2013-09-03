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


#include "mitkIFileReader.h"

mitk::IFileReader::~IFileReader()
{

}

std::string mitk::IFileReader::PROP_EXTENSION()
{
  static std::string s = "org.mitk.services.FileReader.Extension";
  return s;
}

std::string mitk::IFileReader::PROP_DESCRIPTION()
{
  static std::string s = "org.mitk.services.FileReader.Description";
  return s;
}

std::string mitk::IFileReader::PROP_IS_LEGACY()
{
  static std::string s = "org.mitk.services.FileReader.IsLegacy";
  return s;
}

/**
* @brief Used to indicate that an image should be read as an binary image.
*/
std::string mitk::IFileReader::OPTION_READ_AS_BINARY()
{
  static std::string s = "org.mitk.services.FileReader.Option.ReadAsBinary";
  return s;
}

std::string mitk::IFileReader::OPTION_READ_MULTIPLE_FILES()
{
  static std::string s = "org.mitk.services.FileReader.Option.ReadMultipleFiles";
  return s;
}
