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


#include "mitkFileReaderInterface.h"

const std::string mitk::FileReaderInterface::US_PRIORITY = "org.mitk.services.FileReader.Priority";
const std::string mitk::FileReaderInterface::US_EXTENSION = "org.mitk.services.FileReader.Extension";


mitk::FileReaderInterface::FileReaderInterface() :
m_CanReadFromMemory (false),
m_ReadFromMemory (false)
{
}

mitk::FileReaderInterface::~FileReaderInterface()
{
}
bool mitk::FileReaderInterface::CanReadFromMemory(  )
{
  return m_CanReadFromMemory;
}

void mitk::FileReaderInterface::SetReadFromMemory( bool read )
{
  m_ReadFromMemory = read;
}
bool mitk::FileReaderInterface::GetReadFromMemory(  )
{
  return m_ReadFromMemory;
}
void mitk::FileReaderInterface::SetMemoryBuffer(const char* dataArray, unsigned int size)
{
  m_MemoryBuffer = dataArray;
  m_MemorySize   = size;
}
