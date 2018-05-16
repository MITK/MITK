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

#ifndef mitkDICOMGenericTagCache_h
#define mitkDICOMGenericTagCache_h

#include "mitkDICOMTagCache.h"
#include "mitkDICOMGenericImageFrameInfo.h"

namespace mitk
{

  /**
    \ingroup DICOMReaderModule
    \brief Generic tag cache implementation.
  */
  class MITKDICOMREADER_EXPORT DICOMGenericTagCache : public DICOMTagCache
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
