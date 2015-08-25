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

#include "mitkLabelSetImageSerializer.h"
#include "mitkLabelSetImage.h"

#include <itksys/SystemTools.hxx>

#include <mitkIOUtil.h>

MITK_REGISTER_SERIALIZER(LabelSetImageSerializer)

  mitk::LabelSetImageSerializer::LabelSetImageSerializer()
{
}

mitk::LabelSetImageSerializer::~LabelSetImageSerializer()
{
}

std::string mitk::LabelSetImageSerializer::Serialize()
{
  const LabelSetImage* image = dynamic_cast<const LabelSetImage*>( m_Data.GetPointer() );
  if (image == NULL)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
      << " is not an mitk::LabelSetImage. Cannot serialize as LabelSetImage.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".nrrd";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    mitk::IOUtil::Save(const_cast<LabelSetImage*>(image), fullname);
    //    LabelSetImageWriter::Pointer writer = LabelSetImageWriter::New();
    //    writer->SetFileName(fullname);
    //    writer->SetInput(const_cast<LabelSetImage*>(image));
    //    writer->Write();
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