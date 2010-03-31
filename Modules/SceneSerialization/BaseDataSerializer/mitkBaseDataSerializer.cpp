/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include <Poco/TemporaryFile.h>
#include <Poco/Path.h>

#include "mitkBaseDataSerializer.h"

mitk::BaseDataSerializer::BaseDataSerializer()
: m_FilenameHint("unnamed")
, m_WorkingDirectory("")
{
}

mitk::BaseDataSerializer::~BaseDataSerializer()
{
}

std::string mitk::BaseDataSerializer::Serialize()
{
  MITK_INFO << this->GetNameOfClass() 
           << " is asked to serialize an object " << (const void*) this->m_Data
           << " into a directory " << m_WorkingDirectory
           << " using a filename hint " << m_FilenameHint;

  return "";
}

std::string mitk::BaseDataSerializer::GetUniqueFilenameInWorkingDirectory()
{
  Poco::Path newname( Poco::TemporaryFile::tempName() );
  return newname.getFileName();
}

