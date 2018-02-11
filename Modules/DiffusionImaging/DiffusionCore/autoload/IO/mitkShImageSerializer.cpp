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

#include "mitkShImageSerializer.h"
#include "mitkShImage.h"
#include "mitkShImageWriter.h"

#include <itksys/SystemTools.hxx>


MITK_REGISTER_SERIALIZER(ShImageSerializer)


mitk::ShImageSerializer::ShImageSerializer()
{
}


mitk::ShImageSerializer::~ShImageSerializer()
{
}


std::string mitk::ShImageSerializer::Serialize()
{
  const ShImage* image = dynamic_cast<const ShImage*>( m_Data.GetPointer() );
  if (image == nullptr)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::NrrdShImage. Cannot serialize as NrrdShImage.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".shi";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    mitk::ShImageWriter writer;
    writer.SetOutputLocation(fullname);
    writer.SetInput(image);
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

