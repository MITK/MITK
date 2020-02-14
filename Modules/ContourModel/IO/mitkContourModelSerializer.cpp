/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourModelSerializer.h"
#include "mitkContourModelWriter.h"

#include <itksys/SystemTools.hxx>

MITK_REGISTER_SERIALIZER(ContourModelSerializer)

mitk::ContourModelSerializer::ContourModelSerializer()
{
}

mitk::ContourModelSerializer::~ContourModelSerializer()
{
}

std::string mitk::ContourModelSerializer::Serialize()
{
  const auto *contour = dynamic_cast<const ContourModel *>(m_Data.GetPointer());
  if (!contour)
  {
    MITK_ERROR << " Object at " << (const void *)this->m_Data
               << " is not an mitk::ContourModel. Cannot serialize as contour model.";
    return "";
  }

  std::string filename(this->GetUniqueFilenameInWorkingDirectory());
  filename += "_";
  filename += m_FilenameHint;
  filename += ".cnt";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    ContourModelWriter writer;
    writer.SetOutputLocation(fullname);
    writer.SetInput(contour);
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
