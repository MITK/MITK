/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMGDCMTagCache_h
#define mitkDICOMGDCMTagCache_h

#include "mitkDICOMTagCache.h"

#include <set>
#include <memory>

#include <gdcmScanner.h>

namespace mitk
{

  /**
    \ingroup DICOMModule
    \brief Tag cache implementation used by the DICOMGDCMTagScanner.
  */
  class MITKDICOM_EXPORT DICOMGDCMTagCache : public DICOMTagCache
  {
    public:

      mitkClassMacro(DICOMGDCMTagCache, DICOMTagCache);
      itkFactorylessNewMacro( DICOMGDCMTagCache );
      itkCloneMacro(Self);

      DICOMDatasetFinding GetTagValue(DICOMImageFrameInfo* frame, const DICOMTag& tag) const override;

      FindingsListType GetTagValue(DICOMImageFrameInfo* frame, const DICOMTagPath& path) const override;

      DICOMDatasetAccessingImageFrameList GetFrameInfoList() const override;

      void InitCache(const std::set<DICOMTag>& scannedTags, const std::shared_ptr<gdcm::Scanner>& scanner, const StringList& inputFiles);

      const gdcm::Scanner& GetScanner() const;

  protected:

      DICOMGDCMTagCache();
      ~DICOMGDCMTagCache() override;

      std::set<DICOMTag> m_ScannedTags;

      std::shared_ptr<gdcm::Scanner> m_Scanner;

      DICOMDatasetAccessingImageFrameList m_ScanResult;

    private:
      DICOMGDCMTagCache(const DICOMGDCMTagCache&);
  };
}

#endif
