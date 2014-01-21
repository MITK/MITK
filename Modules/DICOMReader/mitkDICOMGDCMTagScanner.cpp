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

#include "mitkDICOMGDCMTagScanner.h"

mitk::DICOMGDCMTagScanner
::DICOMGDCMTagScanner()
:itk::Object()
{
}

mitk::DICOMGDCMTagScanner
::DICOMGDCMTagScanner(const DICOMGDCMTagScanner& other)
:itk::Object()
,DICOMTagCache(other)
{
}

mitk::DICOMGDCMTagScanner
::~DICOMGDCMTagScanner()
{
}

std::string
mitk::DICOMGDCMTagScanner
::GetTagValue(DICOMImageFrameInfo* frame, const DICOMTag& tag) const
{
  // TODO inefficient. if (m_InputFrameList.contains(frame)) return frame->GetTagValueAsString(tag);
  for(DICOMGDCMImageFrameList::const_iterator frameIter = m_ScanResult.begin();
      frameIter != m_ScanResult.end();
      ++frameIter)
  {
    if ( (*frameIter)->GetFrameInfo().IsNotNull() &&
         (*((*frameIter)->GetFrameInfo()) == *frame )
       )
    {
      return (*frameIter)->GetTagValueAsString(tag);
    }

  }

  return "";
}

void
mitk::DICOMGDCMTagScanner
::AddTag(const DICOMTag& tag)
{
  // TODO check for duplicates?
  m_GDCMScanner.AddTag( gdcm::Tag(tag.GetGroup(), tag.GetElement()) );
}

void
mitk::DICOMGDCMTagScanner
::AddTags(const DICOMTagList& tags)
{
  for(DICOMTagList::const_iterator tagIter = tags.begin();
      tagIter != tags.end();
      ++tagIter)
  {
    this->AddTag(*tagIter);
  }
}

void
mitk::DICOMGDCMTagScanner
::SetInputFiles(const StringList& filenames)
{
  m_InputFilenames = filenames;
}


void
mitk::DICOMGDCMTagScanner
::Scan()
{
  // TODO integrate push/pop locale??
  m_GDCMScanner.Scan( m_InputFilenames );

  m_ScanResult.clear();
  for (StringList::const_iterator inputIter = m_InputFilenames.begin();
       inputIter != m_InputFilenames.end();
       ++inputIter)
  {
    m_ScanResult.push_back( DICOMGDCMImageFrameInfo::New( DICOMImageFrameInfo::New(*inputIter, 0), m_GDCMScanner.GetMapping(inputIter->c_str()) ) );
  }
}

mitk::DICOMGDCMImageFrameList
mitk::DICOMGDCMTagScanner
::GetFrameInfoList() const
{
  return m_ScanResult;
}
