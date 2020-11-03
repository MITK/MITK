/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "mitkMAPRegistrationWrapperSerializer.h"
#include "mitkMAPRegistrationWrapperIO.h"

#include "mitkMAPRegistrationWrapper.h"

#include <itksys/SystemTools.hxx>


MITK_REGISTER_SERIALIZER(MAPRegistrationWrapperSerializer)


mitk::MAPRegistrationWrapperSerializer::MAPRegistrationWrapperSerializer()
{
}


mitk::MAPRegistrationWrapperSerializer::~MAPRegistrationWrapperSerializer()
{
}


std::string mitk::MAPRegistrationWrapperSerializer::Serialize()
{
  const mitk::MAPRegistrationWrapper* wrapper = dynamic_cast<const mitk::MAPRegistrationWrapper*>( m_Data.GetPointer() );
  if (wrapper == nullptr)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::MAPRegistrationWrapper. Cannot serialize as MAPRegistrationWrapper for MatchPoint registration objects.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".mapr";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    MAPRegistrationWrapperIO writer;
    writer.SetOutputLocation(fullname);
    writer.AbstractFileWriter::SetInput( dynamic_cast<const mitk::MAPRegistrationWrapper*>( wrapper ) );
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
