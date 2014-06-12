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

#include "mitkIMimeType.h"

namespace mitk {

IMimeType::~IMimeType()
{
}

std::string IMimeType::PROP_ID()
{
  static std::string s = "org.mitk.IMimeType.id";
  return s;
}

std::string IMimeType::PROP_CATEGORY()
{
  static std::string s = "org.mitk.IMimeType.category";
  return s;
}

std::string IMimeType::PROP_EXTENSIONS()
{
  static std::string s = "org.mitk.IMimeType.extensions";
  return s;
}

std::string IMimeType::PROP_DESCRIPTION()
{
  static std::string s = "org.mitk.IMimeType.description";
  return s;
}

}
