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

std::string mitk::IFileReader::PROP_DESCRIPTION()
{
  static std::string s = "org.mitk.IFileReader.description";
  return s;
}

std::string mitk::IFileReader::PROP_MIMETYPE()
{
  static std::string s = "org.mitk.IFileReader.mimetype";
  return s;
}
