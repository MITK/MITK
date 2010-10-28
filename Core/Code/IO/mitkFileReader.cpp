/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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

