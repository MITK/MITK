/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarFigureSerializer.h"
#include "mitkPlanarFigure.h"

#include <mitkIOUtil.h>

#include <itksys/SystemTools.hxx>

MITK_REGISTER_SERIALIZER(PlanarFigureSerializer)

mitk::PlanarFigureSerializer::PlanarFigureSerializer()
{
}

mitk::PlanarFigureSerializer::~PlanarFigureSerializer()
{
}

std::string mitk::PlanarFigureSerializer::Serialize()
{
  const auto *pf = dynamic_cast<const PlanarFigure *>(m_Data.GetPointer());
  if (pf == nullptr)
  {
    MITK_ERROR << " Object at " << (const void *)this->m_Data
               << " is not an mitk::PlanarFigure. Cannot serialize as PlanarFigure.";
    return "";
  }

  std::string filename(this->GetUniqueFilenameInWorkingDirectory());
  filename += "_";
  filename += m_FilenameHint;
  filename += ".pf";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    mitk::IOUtil::Save(pf, fullname);
  }
  catch (std::exception &e)
  {
    MITK_ERROR << " Error serializing object at " << (const void *)this->m_Data << " to " << fullname << ": "
               << e.what();
    return "";
  }
  return filename;
}
