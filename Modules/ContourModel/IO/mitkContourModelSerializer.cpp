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
  const ContourModel* contour = dynamic_cast<const ContourModel*>( m_Data.GetPointer() );
  if (!contour)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::ContourModel. Cannot serialize as contour model.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".cnt";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    ContourModelWriter writer;
    writer.SetOutputLocation( fullname );
    writer.SetInput( const_cast<ContourModel*>( contour ) );
    writer.Write();
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

