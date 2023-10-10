/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkROISerializer.h"

#include <mitkROI.h>
#include <mitkIOUtil.h>

MITK_REGISTER_SERIALIZER(ROISerializer)

mitk::ROISerializer::ROISerializer()
{
}

mitk::ROISerializer::~ROISerializer()
{
}

std::string mitk::ROISerializer::Serialize()
{
  auto roi = dynamic_cast<const ROI*>(m_Data.GetPointer());

  if (nullptr == roi)
  {
    MITK_ERROR << "Object at " << (const void*)this->m_Data << " is not an mitk::ROI. Cannot serialize as MITK ROI.";
    return "";
  }

  auto filename = this->GetUniqueFilenameInWorkingDirectory();
  filename += "_" + m_FilenameHint + ".json";

  std::string path = m_WorkingDirectory;
  path += IOUtil::GetDirectorySeparator() + filename;

  try
  {
    IOUtil::Save(roi, path);
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Error serializing object at " << (const void*)this->m_Data << " to " << path << ": " << e.what();
    return "";
  }

  return filename;
}
