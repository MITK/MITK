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

////////////////// Filenames etc. //////////////////

const char* mitk::FileReaderAbstract::GetFileName() const
{
  return m_FileName;
}

void mitk::FileReaderAbstract::SetFileName(const char* aFileName)
{
 m_FileName = aFileName;
}

const char* mitk::FileReaderAbstract::GetFilePrefix() const
{
  return m_FilePrefix;
}

void mitk::FileReaderAbstract::SetFilePrefix(const char* aFilePrefix)
{
  m_FilePrefix = aFilePrefix;
}

const char* mitk::FileReaderAbstract::GetFilePattern() const
{
  return m_FilePattern;
}

void mitk::FileReaderAbstract::SetFilePattern(const char* aFilePattern)
{
  m_FilePattern = aFilePattern;
}

////////////////// Memory Reading //////////////////

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
