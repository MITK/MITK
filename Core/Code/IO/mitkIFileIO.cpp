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

#include "mitkIFileIO.h"

namespace mitk {

IFileIO::~IFileIO()
{
}

std::string mitk::IFileIO::PROP_DESCRIPTION()
{
  static std::string s = "org.mitk.IFileIO.description";
  return s;
}

std::string mitk::IFileIO::PROP_MIMETYPE()
{
  static std::string s = "org.mitk.IFileIO.mimetype";
  return s;
}

}
