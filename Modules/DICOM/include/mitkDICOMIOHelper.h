/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMIOHelper_h
#define mitkDICOMIOHelper_h

#include <mitkDICOMDatasetAccessingImageFrameInfo.h>
#include <mitkIDICOMTagsOfInterest.h>

#include <MitkDICOMExports.h>

namespace mitk
{
  class BaseData;

  namespace DICOMIOHelper
  {
    using FindingsListVectorType = std::vector<DICOMDatasetAccess::FindingsListType>;

    MITKDICOM_EXPORT IDICOMTagsOfInterest* GetTagsOfInterestService();

    MITKDICOM_EXPORT FindingsListVectorType ExtractPathsOfInterest(const DICOMTagPathList& pathsOfInterest,
      const DICOMDatasetAccessingImageFrameList& frames);

    MITKDICOM_EXPORT void SetProperties(BaseData* image, const FindingsListVectorType& findings);
  }
}

#endif
