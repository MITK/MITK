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
{
}

mitk::DICOMGDCMTagScanner
::DICOMGDCMTagScanner(const DICOMGDCMTagScanner& other)
:DICOMTagCache(other)
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
  assert(frame);

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

  if ( m_ScannedTags.find(tag) != m_ScannedTags.end() )
  {
    if ( std::find( m_InputFilenames.begin(), m_InputFilenames.end(), frame->Filename ) != m_InputFilenames.end() )
    {
      // precondition of gdcm::Scanner::GetValue() fulfilled
      return m_GDCMScanner.GetValue( frame->Filename.c_str(), gdcm::Tag( tag.GetGroup(), tag.GetElement() ) );
    }
    else
    {
      // callers are required to tell us about the filenames they are interested in
      // this is a helpful reminder for them to inform us
      std::stringstream errorstring;
      errorstring << "Invalid call to DICOMGDCMTagScanner::GetTagValue( "
        << "'" << frame->Filename << "', frame " << frame->FrameNo
        << " ). Filename was never mentioned before!";
      MITK_ERROR << errorstring.str();
      throw std::invalid_argument(errorstring.str());
    }
  }
  else
  {
    // callers are required to tell us about the tags they are interested in
    // this is a helpful reminder for them to inform us
    std::stringstream errorstring;
    errorstring << "Invalid call to DICOMGDCMTagScanner::GetTagValue( ";
    tag.Print(errorstring);
    errorstring << " ). Tag was never mentioned before!";
    MITK_ERROR << errorstring.str();
    throw std::invalid_argument(errorstring.str());
  }
}

void
mitk::DICOMGDCMTagScanner
::AddTag(const DICOMTag& tag)
{
  m_ScannedTags.insert(tag);
  m_GDCMScanner.AddTag( gdcm::Tag(tag.GetGroup(), tag.GetElement()) ); // also a set, duplicate calls to AddTag don't hurt
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
