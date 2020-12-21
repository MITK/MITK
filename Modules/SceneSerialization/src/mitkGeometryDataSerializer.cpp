/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGeometryDataSerializer.h"
#include "mitkGeometry3D.h"
#include "mitkGeometryData.h"
#include "mitkIOUtil.h"

MITK_REGISTER_SERIALIZER(GeometryDataSerializer)

mitk::GeometryDataSerializer::GeometryDataSerializer()
{
}

mitk::GeometryDataSerializer::~GeometryDataSerializer()
{
}

std::string mitk::GeometryDataSerializer::Serialize()
{
  // Verify good input data type
  const auto *ps = dynamic_cast<const GeometryData *>(m_Data.GetPointer());
  if (ps == nullptr)
  {
    MITK_ERROR << " Object at " << (const void *)this->m_Data << " is not an mitk::GeometryData. Cannot serialize...";
    return "";
  }

  // Construct the full filename to store the geometry
  std::string filename(this->GetUniqueFilenameInWorkingDirectory());
  filename += "_";
  filename += m_FilenameHint;
  filename += ".mitkgeometry";

  std::string fullname(m_WorkingDirectory);
  fullname += IOUtil::GetDirectorySeparator();
  fullname += filename;

  try
  {
    IOUtil::Save(ps, fullname);
    // in case of success, return only the relative filename part
    return filename;
  }
  catch (const std::exception &e)
  {
    MITK_ERROR << "Unable to serialize GeometryData object: " << e.what();
  }

  // when failed, return empty string
  return "";
}
