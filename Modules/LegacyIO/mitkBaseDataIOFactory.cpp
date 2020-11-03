/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifdef _MSC_VER
#pragma warning(disable : 4786)
#endif

#include "mitkBaseDataIOFactory.h"

#include "mitkIOUtil.h"

namespace mitk
{
  std::vector<BaseData::Pointer> BaseDataIO::LoadBaseDataFromFile(const std::string path,
                                                                  const std::string /*filePrefix*/,
                                                                  const std::string /*filePattern*/,
                                                                  bool /*series*/)
  {
    return IOUtil::Load(path);
  }

} // end namespace itk
