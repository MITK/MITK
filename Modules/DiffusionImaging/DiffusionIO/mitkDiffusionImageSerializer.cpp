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

#include "mitkDiffusionImageSerializer.h"
#include "mitkDiffusionImage.h"
#include "mitkNrrdDiffusionImageWriter.h"

#include <itksys/SystemTools.hxx>


MITK_REGISTER_SERIALIZER(DiffusionImageSerializer)


mitk::DiffusionImageSerializer::DiffusionImageSerializer()
{
}


mitk::DiffusionImageSerializer::~DiffusionImageSerializer()
{
}


std::string mitk::DiffusionImageSerializer::Serialize()
{
  const DiffusionImage<short>* image = dynamic_cast<const DiffusionImage<short>*>( m_Data.GetPointer() );
  if (image == NULL)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::DiffusionImage. Cannot serialize as DiffusionImage.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".dwi";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    NrrdDiffusionImageWriter writer;
    writer.SetOutputLocation(fullname);
    writer.SetInput(const_cast<DiffusionImage<short>*>(image));
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

