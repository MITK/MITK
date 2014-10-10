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
  const ContourModelSet* contourSet = dynamic_cast<const ContourModelSet*>( m_Data.GetPointer() );
  if (!contourSet)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::ContourModelSet. Cannot serialize as contour model set.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".cnt_set";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    ContourModelSetWriter writer;
    writer.SetOutputLocation( fullname );
    writer.SetInput( const_cast<ContourModelSet*>( contourSet ) );
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

