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


#include "mitkFrameOfReferenceUIDManager.h"

std::vector<std::string> mitk::FrameOfReferenceUIDManager::m_FrameOfReferenceUIDs;

mitk::FrameOfReferenceUIDManager::FrameOfReferenceUIDManager()
{
  if(m_FrameOfReferenceUIDs.size()==0)
    m_FrameOfReferenceUIDs.push_back("");
}

mitk::FrameOfReferenceUIDManager::~FrameOfReferenceUIDManager()
{
  m_FrameOfReferenceUIDs.clear();
}

unsigned int mitk::FrameOfReferenceUIDManager::AddFrameOfReferenceUID(const char* forUID)
{
  std::string forUIDstring=forUID;
  unsigned int i;
  size_t size;
  size = m_FrameOfReferenceUIDs.size();
  std::vector<std::string>::iterator it;
  for(i=0, it=m_FrameOfReferenceUIDs.begin(); i<size; ++i,++it)
  {
    if(*it==forUIDstring)
      return i;
  }
//itkGenericOutputMacro(<<"adding uid "<<forUIDstring<< " as id "<<i);
  m_FrameOfReferenceUIDs.push_back(forUIDstring);
  return i;
}

const char* mitk::FrameOfReferenceUIDManager::GetFrameOfReferenceUID(unsigned int geometryFrameOfReferenceID)
{
  if(geometryFrameOfReferenceID >= m_FrameOfReferenceUIDs.size())
    return NULL;
  return m_FrameOfReferenceUIDs[geometryFrameOfReferenceID].c_str();
}

static mitk::FrameOfReferenceUIDManager AddStandardFOR;
