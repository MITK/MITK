/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIFileWriter.h"

namespace mitk
{
  IFileWriter::~IFileWriter() {}
  std::string IFileWriter::PROP_BASEDATA_TYPE()
  {
    static std::string s = "org.mitk.IFileWriter.basedatatype";
    return s;
  }
}
