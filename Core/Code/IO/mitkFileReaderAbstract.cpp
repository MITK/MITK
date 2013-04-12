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


#include "mitkFileReaderAbstract.h"


mitk::FileReaderAbstract::FileReaderAbstract() :
m_CanReadFromMemory (false),
m_ReadFromMemory (false)
{
}

mitk::FileReaderAbstract::~FileReaderAbstract()
{
}
bool mitk::FileReaderAbstract::CanReadFromMemory(  )
{
  return m_CanReadFromMemory;
}

void mitk::FileReaderAbstract::SetReadFromMemory( bool read )
{
  m_ReadFromMemory = read;
}
bool mitk::FileReaderAbstract::GetReadFromMemory(  )
{
  return m_ReadFromMemory;
}
void mitk::FileReaderAbstract::SetMemoryBuffer(const char* dataArray, unsigned int size)
{
  m_MemoryBuffer = dataArray;
  m_MemorySize   = size;
}
