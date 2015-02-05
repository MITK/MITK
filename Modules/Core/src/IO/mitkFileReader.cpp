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


#include "mitkFileReader.h"

mitk::FileReader::FileReader() :
m_CanReadFromMemory (false),
m_ReadFromMemory (false)
{
}

mitk::FileReader::~FileReader()
{
}
bool mitk::FileReader::CanReadFromMemory(  )
{
  return m_CanReadFromMemory;
}

void mitk::FileReader::SetReadFromMemory( bool read )
{
  m_ReadFromMemory = read;
}
bool mitk::FileReader::GetReadFromMemory(  )
{
  return m_ReadFromMemory;
}
void mitk::FileReader::SetMemoryBuffer(const char* dataArray, unsigned int size)
{
  m_MemoryBuffer = dataArray;
  m_MemorySize   = size;
}

