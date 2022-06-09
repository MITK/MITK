/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentationTaskSerializer.h"
#include "mitkSegmentationTask.h"

#include <mitkIOUtil.h>

MITK_REGISTER_SERIALIZER(SegmentationTaskSerializer)

mitk::SegmentationTaskSerializer::SegmentationTaskSerializer()
{
}

mitk::SegmentationTaskSerializer::~SegmentationTaskSerializer()
{
}

std::string mitk::SegmentationTaskSerializer::Serialize()
{
  auto segmentationTask = dynamic_cast<const SegmentationTask*>(m_Data.GetPointer());

  if (nullptr == segmentationTask)
  {
    MITK_ERROR << "Object at " << (const void*)this->m_Data << " is not an mitk::SegmentationTask. Cannot serialize as MITK Segmentation Task.";
    return "";
  }

  auto filename = this->GetUniqueFilenameInWorkingDirectory();
  filename += "_" + m_FilenameHint + ".json";

  std::string path = m_WorkingDirectory;
  path += IOUtil::GetDirectorySeparator() + filename;

  try
  {
    IOUtil::Save(segmentationTask, path);
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Error serializing object at " << (const void*)this->m_Data << " to " << path << ": " << e.what();
    return "";
  }

  return filename;
}
