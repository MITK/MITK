/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMGenericTagCache_h
#define mitkDICOMGenericTagCache_h

#include "mitkDICOMTagCache.h"
#include "mitkDICOMGenericImageFrameInfo.h"

namespace mitk
{

  /**
    \ingroup DICOMModule
    \brief Generic tag cache implementation.
  */
  class MITKDICOM_EXPORT DICOMGenericTagCache : public DICOMTagCache
  {
    public:

      mitkClassMacro(DICOMGenericTagCache, DICOMTagCache);
      itkFactorylessNewMacro( DICOMGenericTagCache );
      itkCloneMacro(Self);

      DICOMDatasetFinding GetTagValue(DICOMImageFrameInfo* frame, const DICOMTag& tag) const override;

      FindingsListType GetTagValue(DICOMImageFrameInfo* frame, const DICOMTagPath& path) const override;

      DICOMDatasetAccessingImageFrameList GetFrameInfoList() const override;

      void AddFrameInfo(DICOMDatasetAccessingImageFrameInfo* info);
      void Reset();

  protected:

      DICOMGenericTagCache();
      ~DICOMGenericTagCache() override;

      DICOMDatasetAccessingImageFrameList m_ScanResult;

    private:
      DICOMGenericTagCache(const DICOMGenericTagCache&);
  };
}

#endif
