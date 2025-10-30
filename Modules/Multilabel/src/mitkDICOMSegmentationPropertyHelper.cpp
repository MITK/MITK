/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkAnatomicalStructureColorPresets.h>
#include <mitkDICOMSegmentationConstants.h>
#include <mitkIDICOMTagsOfInterest.h>
#include <mitkPropertyNameHelper.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkPropertyList.h>

#include "mitkDICOMSegmentationPropertyHelper.h"

#include <vtkSmartPointer.h>

#include <vtkSmartPointer.h>

namespace mitk
{
  void DICOMSegmentationPropertyHelper::DeriveDICOMSegmentationProperties(MultiLabelSegmentation* dicomSegImage)
  {
    PropertyList::Pointer propertyList = dicomSegImage->GetPropertyList();

    // Add DICOM Tag (0008, 0060) Modality "SEG"
    propertyList->SetProperty(GeneratePropertyNameForDICOMTag(0x0008, 0x0060).c_str(),
      TemporoSpatialStringProperty::New("SEG"));
    // Add DICOM Tag (0008,103E) Series Description
    propertyList->SetProperty(GeneratePropertyNameForDICOMTag(0x0008, 0x103E).c_str(),
      TemporoSpatialStringProperty::New("MITK Segmentation"));
    // Add DICOM Tag (0070,0084) Content Creator Name
    propertyList->SetProperty(GeneratePropertyNameForDICOMTag(0x0070, 0x0084).c_str(),
      TemporoSpatialStringProperty::New("MITK"));
    // Add DICOM Tag (0012, 0071) Clinical Trial Series ID
    propertyList->SetProperty(GeneratePropertyNameForDICOMTag(0x0012, 0x0071).c_str(),
      TemporoSpatialStringProperty::New("Session 1"));
    // Add DICOM Tag (0012,0050) Clinical Trial Time Point ID
    propertyList->SetProperty(GeneratePropertyNameForDICOMTag(0x0012, 0x0050).c_str(),
      TemporoSpatialStringProperty::New("0"));
    // Add DICOM Tag (0012, 0060) Clinical Trial Coordinating Center Name
    propertyList->SetProperty(GeneratePropertyNameForDICOMTag(0x0012, 0x0060).c_str(),
      TemporoSpatialStringProperty::New("Unknown"));
  }

}
