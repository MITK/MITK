/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSurfaceSerializer.h"

#include <mitkIOUtil.h>

MITK_REGISTER_SERIALIZER(SurfaceSerializer)

mitk::SurfaceSerializer::SurfaceSerializer()
{
}

mitk::SurfaceSerializer::~SurfaceSerializer()
{
}

std::string mitk::SurfaceSerializer::Serialize()
{
  const auto *surface = dynamic_cast<const Surface *>(m_Data.GetPointer());
  if (!surface)
  {
    MITK_ERROR << " Object at " << (const void *)this->m_Data
               << " is not an mitk::Surface. Cannot serialize as surface.";
    return "";
  }

  // VTK XML PolyData holds a single time step without its time, and several
  // time steps would be split into files the scene cannot reference. VTKHDF
  // holds all time steps with their times. Surfaces without time, like 3D
  // images, stay VTK XML PolyData, which older versions of MITK can read.
  std::string filename(this->GetUniqueFilenameInWorkingDirectory());
  filename += "_";
  filename += m_FilenameHint;
  filename += surface->GetTimeGeometry()->IsValidAtEveryTimePoint() ? ".vtp" : ".vtkhdf";

  std::string fullname(m_WorkingDirectory);
  fullname += IOUtil::GetDirectorySeparator();
  fullname += filename;

  try
  {
    IOUtil::Save(surface, fullname);
  }
  catch (std::exception &e)
  {
    MITK_ERROR << " Error serializing object at " << (const void *)this->m_Data << " to " << fullname << ": "
               << e.what();
    return "";
  }

  return filename;
}
