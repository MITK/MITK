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

#include "mitkBaseDataSerializer.h"
#include "mitkStandardFileLocations.h"
#include <itksys/SystemTools.hxx>

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
  // tmpname
  static unsigned long count = 0;
  unsigned long n = count++;
  std::ostringstream name;
  for (int i = 0; i < 6; ++i)
  {
    name << char('a' + (n % 26));
    n /= 26;
  }
  std::string myname;
  myname.append(name.str());
  return myname;
}

