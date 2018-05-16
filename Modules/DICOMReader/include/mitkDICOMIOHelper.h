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

#ifndef MITKDICOMIOHelper_H
#define MITKDICOMIOHelper_H

#include "mitkDICOMDatasetAccessingImageFrameInfo.h"
#include "mitkIDICOMTagsOfInterest.h"

#include <MitkDICOMReaderExports.h>

namespace mitk
{
  class BaseData;

  typedef std::vector<mitk::DICOMDatasetAccess::FindingsListType> FindingsListVectorType;
  typedef BaseData *BaseDataPointer;

  MITKDICOMREADER_EXPORT mitk::IDICOMTagsOfInterest *GetDicomTagsOfInterestService();

  MITKDICOMREADER_EXPORT FindingsListVectorType ExtractPathsOfInterest(const DICOMTagPathList &pathsOfInterest,
                                                const DICOMDatasetAccessingImageFrameList &frames);

  MITKDICOMREADER_EXPORT void SetProperties(BaseDataPointer image, const FindingsListVectorType &findings);
}

#endif
