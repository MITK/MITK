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

mitk::DICOMGDCMTagScanner::DICOMGDCMTagScanner()
{
}

mitk::DICOMGDCMTagScanner::DICOMGDCMTagScanner( const DICOMGDCMTagScanner& other )
: DICOMTagCache( other )
{
}

mitk::DICOMGDCMTagScanner::~DICOMGDCMTagScanner()
{
}

mitk::DICOMDatasetFinding mitk::DICOMGDCMTagScanner::GetTagValue( DICOMImageFrameInfo* frame, const DICOMTag& tag ) const
{
  assert( frame );

  for ( auto frameIter = m_ScanResult.cbegin(); frameIter != m_ScanResult.cend(); ++frameIter )
  {
    if ( ( *frameIter )->GetFrameInfo().IsNotNull() && ( *( ( *frameIter )->GetFrameInfo() ) == *frame ) )
    {
      return (*frameIter)->GetTagValueAsString(tag);
    }
  }

  if ( m_ScannedTags.find( tag ) != m_ScannedTags.cend() )
  {
    if ( std::find( m_InputFilenames.cbegin(), m_InputFilenames.cend(), frame->Filename )
         != m_InputFilenames.cend() )
    {
      // precondition of gdcm::Scanner::GetValue() fulfilled
      const char* value = m_GDCMScanner.GetValue( frame->Filename.c_str(), gdcm::Tag( tag.GetGroup(), tag.GetElement() ) );
      DICOMDatasetFinding result;
      if (value)
      {
        result.isValid = true;
        result.value = value;
      }
      return result;
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
      throw std::invalid_argument( errorstring.str() );
    }
  }
  else
  {
    // callers are required to tell us about the tags they are interested in
    // this is a helpful reminder for them to inform us
    std::stringstream errorstring;
    errorstring << "Invalid call to DICOMGDCMTagScanner::GetTagValue( ";
    tag.Print( errorstring );
    errorstring << " ). Tag was never mentioned before!";
    MITK_ERROR << errorstring.str();
    throw std::invalid_argument( errorstring.str() );
  }
}

void mitk::DICOMGDCMTagScanner::AddTag( const DICOMTag& tag )
{
  m_ScannedTags.insert( tag );
  m_GDCMScanner.AddTag(
    gdcm::Tag( tag.GetGroup(), tag.GetElement() ) ); // also a set, duplicate calls to AddTag don't hurt
}

void mitk::DICOMGDCMTagScanner::AddTags( const DICOMTagList& tags )
{
  for ( auto tagIter = tags.cbegin(); tagIter != tags.cend(); ++tagIter )
  {
    this->AddTag( *tagIter );
  }
}

void mitk::DICOMGDCMTagScanner::SetInputFiles( const StringList& filenames )
{
  m_InputFilenames = filenames;
}


void mitk::DICOMGDCMTagScanner::Scan()
{
  // TODO integrate push/pop locale??
  m_GDCMScanner.Scan( m_InputFilenames );

  m_ScanResult.clear();
  m_ScanResult.reserve( m_InputFilenames.size() );

  for ( auto inputIter = m_InputFilenames.cbegin(); inputIter != m_InputFilenames.cend(); ++inputIter )
  {
    m_ScanResult.push_back( DICOMGDCMImageFrameInfo::New( DICOMImageFrameInfo::New( *inputIter, 0 ),
                                                          m_GDCMScanner.GetMapping( inputIter->c_str() ) ) );
  }
}

mitk::DICOMGDCMImageFrameList mitk::DICOMGDCMTagScanner::GetFrameInfoList() const
{
  return m_ScanResult;
}
