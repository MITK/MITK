/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkFileReader.h"

mitk::FileReader::FileReader() : m_CanReadFromMemory(false), m_ReadFromMemory(false)
{
}

mitk::FileReader::~FileReader()
{
}
bool mitk::FileReader::CanReadFromMemory()
{
  return m_CanReadFromMemory;
}

void mitk::FileReader::SetReadFromMemory(bool read)
{
  m_ReadFromMemory = read;
}
bool mitk::FileReader::GetReadFromMemory()
{
  return m_ReadFromMemory;
}
void mitk::FileReader::SetMemoryBuffer(const char *dataArray, unsigned int size)
{
  m_MemoryBuffer = dataArray;
  m_MemorySize = size;
}
