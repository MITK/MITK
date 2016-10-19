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

#include "mitkDICOMGDCMTagCache.h"
#include "mitkDICOMEnums.h"
#include "mitkDICOMGDCMImageFrameInfo.h"

mitk::DICOMGDCMTagCache::DICOMGDCMTagCache()
{
}

mitk::DICOMGDCMTagCache::~DICOMGDCMTagCache()
{
}

mitk::DICOMDatasetFinding mitk::DICOMGDCMTagCache::GetTagValue( DICOMImageFrameInfo* frame, const DICOMTag& tag ) const
{
  assert( frame );

  for ( auto frameIter = m_ScanResult.cbegin(); frameIter != m_ScanResult.cend(); ++frameIter )
  {
    if ( **frameIter == *frame )
    {
      return (*frameIter)->GetTagValueAsString(tag);
    }
  }

  if ( m_ScannedTags.find( tag ) != m_ScannedTags.cend() )
  {
    if ( std::find( m_InputFilenames.cbegin(), m_InputFilenames.cend(), frame->Filename )
         == m_InputFilenames.cend() )
    {
      // callers are required to tell us about the filenames they are interested in
      // this is a helpful reminder for them to inform us
      std::stringstream errorstring;
      errorstring << "Invalid call to DICOMGDCMTagCache::GetTagValue( "
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
    errorstring << "Invalid call to DICOMGDCMTagCache::GetTagValue( ";
    tag.Print( errorstring );
    errorstring << " ). Tag was never mentioned before!";
    MITK_ERROR << errorstring.str();
    throw std::invalid_argument( errorstring.str() );
  }

  return DICOMDatasetFinding();
}

mitk::DICOMDatasetAccess::FindingsListType
mitk::DICOMGDCMTagCache::GetTagValue(DICOMImageFrameInfo* frame, const DICOMTagPath& path) const
{
  FindingsListType result;
  if (path.Size() == 1 && path.IsExplicit())
  {
    result.push_back(this->GetTagValue(frame, path.GetFirstNode().tag));
  }
  return result;
}

mitk::DICOMDatasetAccessingImageFrameList mitk::DICOMGDCMTagCache::GetFrameInfoList() const
{
  return m_ScanResult;
}

void
mitk::DICOMGDCMTagCache::InitCache(const std::set<DICOMTag>& scannedTags, const std::shared_ptr<gdcm::Scanner>& scanner, const StringList& inputFiles)
{
  m_ScannedTags = scannedTags;
  m_InputFilenames = inputFiles;
  m_Scanner = scanner;

  m_ScanResult.clear();
  m_ScanResult.reserve(m_InputFilenames.size());

  for (auto inputIter = m_InputFilenames.cbegin(); inputIter != m_InputFilenames.cend(); ++inputIter)
  {
    m_ScanResult.push_back(DICOMGDCMImageFrameInfo::New(DICOMImageFrameInfo::New(*inputIter, 0),
      m_Scanner->GetMapping(inputIter->c_str())).GetPointer());
  }
}

const gdcm::Scanner&
mitk::DICOMGDCMTagCache::GetScanner() const
{
  return *(this->m_Scanner);
}
