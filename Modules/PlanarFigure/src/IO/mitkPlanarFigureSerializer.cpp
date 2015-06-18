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

#include "mitkPlanarFigureSerializer.h"
#include "mitkPlanarFigure.h"
#include "mitkPlanarFigureWriter.h"

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
  const PlanarFigure* pf = dynamic_cast<const PlanarFigure*>( m_Data.GetPointer() );
  if (pf == nullptr)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::PlanarFigure. Cannot serialize as PlanarFigure.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".pf";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    PlanarFigureWriter::Pointer writer = PlanarFigureWriter::New();
    writer->SetFileName(fullname);
    writer->SetInput(const_cast<PlanarFigure*>(pf));
    writer->Write();
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

