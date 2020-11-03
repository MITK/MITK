/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef DICOMQIPROPERTYHELPER_H_
#define DICOMQIPROPERTYHELPER_H_

#include <mitkDICOMTag.h>
#include <mitkBaseData.h>

#include <MitkDICOMQIExports.h>

namespace mitk
{
  class MITKDICOMQI_EXPORT DICOMQIPropertyHelper
  {
  public:
    static void DeriveDICOMSourceProperties(const BaseData *sourceDICOMImage, BaseData *derivedDICOMImage);

  private:
    static void AdoptReferenceDICOMProperty(PropertyList *referencedPropertyList,
      PropertyList *propertyList,
      const DICOMTag &tag,
      const std::string &defaultString = "");
    //-------------
  };
}
#endif
