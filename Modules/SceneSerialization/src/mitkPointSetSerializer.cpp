/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPointSetSerializer.h"
#include "mitkIOUtil.h"

MITK_REGISTER_SERIALIZER(PointSetSerializer)

mitk::PointSetSerializer::PointSetSerializer()
{
}

mitk::PointSetSerializer::~PointSetSerializer()
{
}

std::string mitk::PointSetSerializer::Serialize()
{
  const auto *ps = dynamic_cast<const PointSet *>(m_Data.GetPointer());
  if (ps == nullptr)
  {
    MITK_ERROR << " Object at " << (const void *)this->m_Data
               << " is not an mitk::PointSet. Cannot serialize as pointset.";
    return "";
  }

  std::string filename(this->GetUniqueFilenameInWorkingDirectory());
  filename += "_";
  filename += m_FilenameHint;
  filename += ".mps";

  std::string fullname(m_WorkingDirectory);
  fullname += IOUtil::GetDirectorySeparator();
  fullname += filename;

  try
  {
    IOUtil::Save(ps, fullname);
  }
  catch (std::exception &e)
  {
    MITK_ERROR << " Error serializing object at " << (const void *)this->m_Data << " to " << fullname << ": "
               << e.what();
    return "";
  }
  return filename;
}
