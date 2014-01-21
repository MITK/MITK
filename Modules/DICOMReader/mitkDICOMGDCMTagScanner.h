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

#ifndef mitkDICOMGDCMTagScanner_h
#define mitkDICOMGDCMTagScanner_h

#include "mitkDICOMTagCache.h"
#include "mitkDICOMEnums.h"

#include "mitkDICOMGDCMImageFrameInfo.h"

#include <gdcmScanner.h>

namespace mitk
{

  /**
    \ingroup DICOMReaderModule
    \brief ...
  */
  class DICOMReader_EXPORT DICOMGDCMTagScanner : public DICOMTagCache
  {
    public:

      mitkClassMacro( DICOMGDCMTagScanner, DICOMTagCache );
      itkNewMacro( DICOMGDCMTagScanner );

      virtual std::string GetTagValue(DICOMImageFrameInfo* frame, const DICOMTag& tag) const;

      virtual void AddTag(const DICOMTag& tag);
      virtual void AddTags(const DICOMTagList& tags);

      virtual void SetInputFiles(const StringList& filenames);

      virtual void Scan();

      virtual DICOMGDCMImageFrameList GetFrameInfoList() const;

      protected:

      DICOMGDCMTagScanner();
      DICOMGDCMTagScanner(const DICOMGDCMTagScanner&);
      virtual ~DICOMGDCMTagScanner();

      gdcm::Scanner m_GDCMScanner;
      StringList m_InputFilenames;
      DICOMGDCMImageFrameList m_ScanResult;
  };
}

#endif
