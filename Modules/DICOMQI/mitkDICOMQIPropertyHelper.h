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
