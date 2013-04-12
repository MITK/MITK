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

const std::string mitk::FileReaderAbstract::GetFileName() const
{
  return m_FileName;
}

void mitk::FileReaderAbstract::SetFileName(const std::string aFileName)
{
 m_FileName = aFileName;
}

const std::string mitk::FileReaderAbstract::GetFilePrefix() const
{
  return m_FilePrefix;
}

void mitk::FileReaderAbstract::SetFilePrefix(const std::string aFilePrefix)
{
  m_FilePrefix = aFilePrefix;
}

const std::string mitk::FileReaderAbstract::GetFilePattern() const
{
  return m_FilePattern;
}

void mitk::FileReaderAbstract::SetFilePattern(const std::string aFilePattern)
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
void mitk::FileReaderAbstract::SetMemoryBuffer(const std::string dataArray, unsigned int size)
{
  m_MemoryBuffer = dataArray;
  m_MemorySize   = size;
}

////////////////// µS related Getters //////////////////

int mitk::FileReaderAbstract::GetPriority()
{
  return m_Priority;
}
