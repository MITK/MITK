/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentationTaskListSerializer.h"
#include "mitkSegmentationTaskList.h"

#include <mitkIOUtil.h>

MITK_REGISTER_SERIALIZER(SegmentationTaskListSerializer)

mitk::SegmentationTaskListSerializer::SegmentationTaskListSerializer()
{
}

mitk::SegmentationTaskListSerializer::~SegmentationTaskListSerializer()
{
}

std::string mitk::SegmentationTaskListSerializer::Serialize()
{
  auto segmentationTaskList = dynamic_cast<const SegmentationTaskList*>(m_Data.GetPointer());

  if (nullptr == segmentationTaskList)
  {
    MITK_ERROR << "Object at " << (const void*)this->m_Data << " is not an mitk::SegmentationTaskList. Cannot serialize as MITK Segmentation Task List.";
    return "";
  }

  auto filename = this->GetUniqueFilenameInWorkingDirectory();
  filename += "_" + m_FilenameHint + ".json";

  std::string path = m_WorkingDirectory;
  path += IOUtil::GetDirectorySeparator() + filename;

  try
  {
    IOUtil::Save(segmentationTaskList, path);
  }
  catch (std::exception& e)
  {
    MITK_ERROR << "Error serializing object at " << (const void*)this->m_Data << " to " << path << ": " << e.what();
    return "";
  }

  return filename;
}
