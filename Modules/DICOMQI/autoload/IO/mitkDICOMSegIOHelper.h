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

#ifndef MITKDICOMSegIOHelper_H
#define MITKDICOMSegIOHelper_H

#include <mitkIDICOMTagsOfInterest.h>
#include <mitkDICOMDatasetAccessingImageFrameInfo.h>

#include "MitkDICOMQIIOExports.h"

namespace mitk {
  class BaseData;

  typedef std::vector<mitk::DICOMDatasetAccess::FindingsListType> FindingsListVectorType;
  typedef BaseData* BaseDataPointer;

  mitk::IDICOMTagsOfInterest* GetDicomTagsOfInterestService();

  FindingsListVectorType ExtractPathsOfInterest(const DICOMTagPathList& pathsOfInterest, const DICOMDatasetAccessingImageFrameList& frames);

  void SetProperties(BaseDataPointer image, const FindingsListVectorType& findings);
}

#endif
