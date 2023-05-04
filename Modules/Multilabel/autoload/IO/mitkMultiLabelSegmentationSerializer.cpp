/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMultiLabelSegmentationSerializer.h"
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
  const auto *image = dynamic_cast<const LabelSetImage *>(m_Data.GetPointer());
  if (image == nullptr)
  {
    MITK_ERROR << " Object at " << (const void *)this->m_Data
               << " is not an mitk::LabelSetImage. Cannot serialize as LabelSetImage.";
    return "";
  }

  std::string filename(this->GetUniqueFilenameInWorkingDirectory());
  filename += "_";
  filename += m_FilenameHint;
  filename += ".nrrd";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    mitk::IOUtil::Save(image, fullname);
    //    LabelSetImageWriter::Pointer writer = LabelSetImageWriter::New();
    //    writer->SetFileName(fullname);
    //    writer->SetInput(const_cast<LabelSetImage*>(image));
    //    writer->Write();
  }
  catch (std::exception &e)
  {
    MITK_ERROR << " Error serializing object at " << (const void *)this->m_Data << " to " << fullname << ": "
               << e.what();
    return "";
  }
  return filename;
}
