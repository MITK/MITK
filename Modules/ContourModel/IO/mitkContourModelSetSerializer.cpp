/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourModelSetSerializer.h"
#include "mitkContourModelSetWriter.h"

#include <itksys/SystemTools.hxx>

MITK_REGISTER_SERIALIZER(ContourModelSetSerializer)

mitk::ContourModelSetSerializer::ContourModelSetSerializer()
{
}

mitk::ContourModelSetSerializer::~ContourModelSetSerializer()
{
}

std::string mitk::ContourModelSetSerializer::Serialize()
{
  const auto *contourSet = dynamic_cast<const ContourModelSet *>(m_Data.GetPointer());
  if (!contourSet)
  {
    MITK_ERROR << " Object at " << (const void *)this->m_Data
               << " is not an mitk::ContourModelSet. Cannot serialize as contour model set.";
    return "";
  }

  std::string filename(this->GetUniqueFilenameInWorkingDirectory());
  filename += "_";
  filename += m_FilenameHint;
  filename += ".cnt_set";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    ContourModelSetWriter writer;
    writer.SetOutputLocation(fullname);
    writer.SetInput(contourSet);
    writer.Write();
  }
  catch ( const std::exception &e )
  {
    MITK_ERROR << " Error serializing object at " << (const void *)this->m_Data << " to " << fullname << ": "
               << e.what();
    return "";
  }

  return filename;
}
