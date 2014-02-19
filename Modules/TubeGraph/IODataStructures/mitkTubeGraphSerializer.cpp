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

#include "mitkTubeGraphSerializer.h"

#include "mitkTubeGraph.h"
#include "mitkTubeGraphWriter.h"

#include <itksys/SystemTools.hxx>


MITK_REGISTER_SERIALIZER(TubeGraphSerializer)

mitk::TubeGraphSerializer::TubeGraphSerializer()
{
}

mitk::TubeGraphSerializer::~TubeGraphSerializer()
{
}

std::string mitk::TubeGraphSerializer::Serialize()
{
  const TubeGraph* tubeGraph = dynamic_cast<const mitk::TubeGraph*>( m_Data.GetPointer() );
  if (tubeGraph == NULL)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
      << " is not an mitk::TubeGraph. Cannot serialize as TubeGraph.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".tsf";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    TubeGraphWriter::Pointer writer = TubeGraphWriter::New();
    writer->SetFileName(fullname);
    writer->SetInput(const_cast<mitk::TubeGraph*>(tubeGraph));
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
