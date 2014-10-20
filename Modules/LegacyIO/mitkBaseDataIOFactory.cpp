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

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include "mitkBaseDataIOFactory.h"

#include "mitkIOUtil.h"

namespace mitk
{

std::vector<BaseData::Pointer> BaseDataIO::LoadBaseDataFromFile(const std::string path, const std::string /*filePrefix*/,
                                                                const std::string /*filePattern*/, bool /*series*/)
{
  return IOUtil::Load(path);
}

} // end namespace itk
