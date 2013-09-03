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


#include "mitkIFileWriter.h"

mitk::IFileWriter::~IFileWriter()
{

}

std::string mitk::IFileWriter::PROP_EXTENSION()
{
  static std::string s = "org.mitk.services.FileWriter.Extension";
  return s;
}

std::string mitk::IFileWriter::PROP_BASEDATA_TYPE()
{
  static std::string s = "org.mitk.services.FileWriter.BasedataType";
  return s;
}

std::string mitk::IFileWriter::PROP_DESCRIPTION()
{
  static std::string s = "org.mitk.services.FileWriter.Description";
  return s;
}

std::string mitk::IFileWriter::PROP_IS_LEGACY()
{
  static std::string s = "org.mitk.services.FileWriter.IsLegacy";
  return s;
}
