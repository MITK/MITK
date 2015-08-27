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

#include "mitkExampleDataStructureSerializer.h"
#include "mitkExampleDataStructure.h"
#include "mitkIOUtil.h"

#include <itksys/SystemTools.hxx>


MITK_REGISTER_SERIALIZER(ExampleDataStructureSerializer)


mitk::ExampleDataStructureSerializer::ExampleDataStructureSerializer()
{
}


mitk::ExampleDataStructureSerializer::~ExampleDataStructureSerializer()
{
}


std::string mitk::ExampleDataStructureSerializer::Serialize()
{
  const ExampleDataStructure* exData = dynamic_cast<const ExampleDataStructure*>( m_Data.GetPointer() );
  if (exData == NULL)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::ExampleDataStructure. Cannot serialize as ExampleDataStructure.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".txt";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    mitk::IOUtil::Save(exData, fullname);
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

