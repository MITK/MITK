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

#include <mitkAnatomicalStructureColorPresets.h>
#include <mitkDICOMTag.h>
#include <mitkIDICOMTagsOfInterest.h>
#include <mitkPropertyList.h>
#include <mitkPropertyNameHelper.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <MitkDICOMQIExports.h>


#include <mitkDICOMPMPropertyHelper.h>

// us
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk
{
  void DICOMPMPropertyHelper::DeriveDICOMPMProperties(BaseData *derivedDICOMImage)
  {
    PropertyList::Pointer propertyList = derivedDICOMImage->GetPropertyList();
    
    // Add DICOM Tag (0008, 0060) Modality "PM"
    propertyList->SetProperty(GeneratePropertyNameForDICOMTag(0x0008, 0x0060).c_str(),
                              TemporoSpatialStringProperty::New("PM"));

    // Add DICOM Tag (0008,103E) Series Description
    propertyList->SetProperty(GeneratePropertyNameForDICOMTag(0x0008, 0x103E).c_str(),
                              TemporoSpatialStringProperty::New("MITK Parameter Map"));
    // Add DICOM Tag (0070,0084) Content Creator Name
    propertyList->SetProperty(GeneratePropertyNameForDICOMTag(0x0070, 0x0084).c_str(),
                              TemporoSpatialStringProperty::New("MITK"));




  }


 
} // namespace mitk



