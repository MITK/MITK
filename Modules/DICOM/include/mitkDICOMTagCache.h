/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMTagCache_h
#define mitkDICOMTagCache_h

#include "itkObjectFactory.h"
#include "mitkCommon.h"

#include "mitkDICOMTag.h"
#include "mitkDICOMDatasetAccess.h"
#include "mitkDICOMEnums.h"

#include "mitkDICOMDatasetAccessingImageFrameInfo.h"
#include "MitkDICOMExports.h"

namespace mitk
{

  /**
    \ingroup DICOMModule
    \brief ...
  */
  class MITKDICOM_EXPORT DICOMTagCache : public itk::Object
  {
    public:
      mitkClassMacroItkParent( DICOMTagCache, itk::Object );

      typedef std::list<DICOMDatasetFinding> FindingsListType;

      /**
      \brief Define the list of files that were scanned to populate the cache.
      */
      virtual void SetInputFiles(const StringList& filenames);

      virtual DICOMDatasetFinding GetTagValue(DICOMImageFrameInfo* frame, const DICOMTag& tag) const = 0;

      virtual FindingsListType GetTagValue(DICOMImageFrameInfo* frame, const DICOMTagPath& path) const = 0;

      /**
      \brief Retrieve a result list for file-by-file tag access.
      */
      virtual DICOMDatasetAccessingImageFrameList GetFrameInfoList() const = 0;

    protected:

      StringList m_InputFilenames;
      DICOMTagCache();
      DICOMTagCache(const DICOMTagCache&);
      ~DICOMTagCache() override;
  };
}

#endif
