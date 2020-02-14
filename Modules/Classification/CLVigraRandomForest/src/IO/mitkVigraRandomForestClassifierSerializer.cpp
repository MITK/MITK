/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkVigraRandomForestClassifierSerializer.h"
#include "mitkIOUtil.h"
#include "mitkVigraRandomForestClassifier.h"

#include <itksys/SystemTools.hxx>

MITK_REGISTER_SERIALIZER(VigraRandomForestClassifierSerializer)

mitk::VigraRandomForestClassifierSerializer::VigraRandomForestClassifierSerializer()
{
}

mitk::VigraRandomForestClassifierSerializer::~VigraRandomForestClassifierSerializer()
{
}

std::string mitk::VigraRandomForestClassifierSerializer::Serialize()
{
  const mitk::VigraRandomForestClassifier* data = dynamic_cast<const mitk::VigraRandomForestClassifier*>( m_Data.GetPointer() );
  if (!data)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::ContourModelSet. Cannot serialize as contour model set.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".hdf5";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    mitk::IOUtil::Save(const_cast<mitk::VigraRandomForestClassifier*>( data ),fullname);
  }
  catch ( const std::exception& e )
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

