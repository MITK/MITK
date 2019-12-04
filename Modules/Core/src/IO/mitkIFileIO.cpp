/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIFileIO.h"

namespace mitk
{
  IFileIO::~IFileIO() {}
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
