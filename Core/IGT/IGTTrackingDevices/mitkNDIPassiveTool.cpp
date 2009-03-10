/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkNDIPassiveTool.h"
#include <iostream>
#include <fstream>


mitk::NDIPassiveTool::NDIPassiveTool() 
: InternalTrackingTool(), 
m_SROMData(NULL), 
m_SROMDataLength(0), 
m_TrackingPriority(Dynamic), 
m_PortHandle() 
{
}


mitk::NDIPassiveTool::~NDIPassiveTool()
{
  if (m_SROMData != NULL)
  {
    delete m_SROMData;
    m_SROMData = NULL;
  }
}


bool mitk::NDIPassiveTool::LoadSROMFile(const char* filename)
{
  if (filename == NULL)
    return false;
  if (filename[0] == '\0')
    return false;

  std::basic_ifstream<char> file;
  file.open(filename, std::ios::in | std::ios::binary); // open the file
  if (file.is_open() == false)
    return false;

  file.seekg (0, std::ios::end);                // get the length of the file
  unsigned int newLength = file.tellg();
  file.seekg (0, std::ios::beg);
  char* newData = new char [newLength];         // create a buffer to store the srom file
  file.read(newData, newLength);                // read the file into the buffer
  file.close();
  if (file.fail() == true)                      // reading of data unsuccessful?
  {
    delete newData;
    return false;
  }
  if (m_SROMData != NULL)                       // reading was successful, delete old data
    delete m_SROMData;
  m_SROMDataLength = newLength;                 // set member variables to new values
  m_SROMData = (unsigned char*) newData;
  this->Modified();
  return true;
}

const unsigned char* mitk::NDIPassiveTool::GetSROMData() const
{
  return m_SROMData;
}

unsigned int mitk::NDIPassiveTool::GetSROMDataLength() const
{
  return m_SROMDataLength;
}
