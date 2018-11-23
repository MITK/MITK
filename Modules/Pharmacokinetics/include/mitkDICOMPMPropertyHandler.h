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

#ifndef DICOMSEGMENTATIONPROPERTYHANDLER_H_
#define DICOMSEGMENTATIONPROPERTYHANDLER_H_

#include <mitkDICOMTag.h>
//#include <mitkLabel.h>
//#include <mitkLabelSetImage.h>

#include <MitkPharmacokineticsExports.h>

namespace mitk
{
  class MITKPHARMACOKINETICS_EXPORT DICOMPMPropertyHandler
  {
  public:
    static PropertyList::Pointer GetDICOMPMProperties(PropertyList *referencedPropertyList);
    static void SetReferenceDICOMProperty(PropertyList *referencedPropertyList,
                                          PropertyList *propertyList,
                                          const DICOMTag &tag,
                                          const std::string &defaultString = "");
  };
} // namespace mitk
#endif
