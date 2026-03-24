/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMPMPropertyHelper_h
#define mitkDICOMPMPropertyHelper_h

#include <mitkDICOMTag.h>
#include <mitkBaseData.h>

#include <MitkDICOMQIExports.h>

namespace mitk
{
  class MITKDICOMQI_EXPORT DICOMPMPropertyHelper
  {
  public:
    static void DeriveDICOMPMProperties(BaseData *derivedDICOMImage);

  };
} // namespace mitk
#endif
