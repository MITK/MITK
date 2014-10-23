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

#include "mitkQBallImageSerializer.h"
#include "mitkQBallImage.h"
#include "mitkNrrdQBallImageWriter.h"

#include <itksys/SystemTools.hxx>


MITK_REGISTER_SERIALIZER(QBallImageSerializer)


mitk::QBallImageSerializer::QBallImageSerializer()
{
}


mitk::QBallImageSerializer::~QBallImageSerializer()
{
}


std::string mitk::QBallImageSerializer::Serialize()
{
  const QBallImage* image = dynamic_cast<const QBallImage*>( m_Data.GetPointer() );
  if (image == NULL)
  {
    MITK_ERROR << " Object at " << (const void*) this->m_Data
              << " is not an mitk::NrrdQBallImage. Cannot serialize as NrrdQBallImage.";
    return "";
  }

  std::string filename( this->GetUniqueFilenameInWorkingDirectory() );
  filename += "_";
  filename += m_FilenameHint;
  filename += ".qbi";

  std::string fullname(m_WorkingDirectory);
  fullname += "/";
  fullname += itksys::SystemTools::ConvertToOutputPath(filename.c_str());

  try
  {
    NrrdQBallImageWriter writer;
    writer.SetOutputLocation(fullname);
    writer.SetInput(const_cast<QBallImage*>(image));
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

