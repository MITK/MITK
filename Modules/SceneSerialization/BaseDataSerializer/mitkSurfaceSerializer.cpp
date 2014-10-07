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

#include "mitkSurfaceSerializer.h"

#include "mitkIOUtil.h"

MITK_REGISTER_SERIALIZER(SurfaceSerializer)

mitk::SurfaceSerializer::SurfaceSerializer()
{
}

mitk::SurfaceSerializer::~SurfaceSerializer()
{
}

std::string mitk::SurfaceSerializer::Serialize()
{
  const Surface* surface = dynamic_cast<const Surface*>( m_Data.GetPointer() );
  if (!surface)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::Surface. Cannot serialize as surface.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".vtp";

  std::string fullname(m_WorkingDirectory);
  fullname += IOUtil::GetDirectorySeparator();
  fullname += filename;

  try
  {
    IOUtil::Save(surface, fullname);
  }
  catch (std::exception& e)
  {
    MITK_ERROR << " Error serializing object at " << (const void*) this->m_Data
              << " to "
              << fullname
              << ": "
              << e.what();
    return "";
  }

  return filename;
}

