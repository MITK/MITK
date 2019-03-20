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

#ifndef DICOMSEGMENTATIONPROPERTYHELPER_H_
#define DICOMSEGMENTATIONPROPERTYHELPER_H_

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
