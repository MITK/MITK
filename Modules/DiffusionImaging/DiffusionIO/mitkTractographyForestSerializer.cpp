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

#include "mitkTractographyForestSerializer.h"
#include "mitkTractographyForest.h"
#include "mitkTractographyForestWriter.h"
#include <mitkTractographyForest.h>

#include <itksys/SystemTools.hxx>
#include <mitkIOUtil.h>


MITK_REGISTER_SERIALIZER(TractographyForestSerializer)


mitk::TractographyForestSerializer::TractographyForestSerializer()
{
}


mitk::TractographyForestSerializer::~TractographyForestSerializer()
{
}


std::string mitk::TractographyForestSerializer::Serialize()
{
  const TractographyForest* fb = dynamic_cast<const TractographyForest*>( m_Data.GetPointer() );
  if (fb == nullptr)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::TractographyForest. Cannot serialize as TractographyForest.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".rf";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    mitk::IOUtil::Save(fb,fullname);
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

