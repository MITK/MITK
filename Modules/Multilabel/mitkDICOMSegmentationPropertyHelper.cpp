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
  void DICOMSegmentationPropertyHelper::DeriveDICOMSegmentationProperties(LabelSetImage* dicomSegImage)
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

    // Set DICOM properties for each label
    // Iterate over all layers
    for (unsigned int layer = 0; layer < dicomSegImage->GetNumberOfLayers(); ++layer)
    {
      // Iterate over all labels
      const LabelSet *labelSet = dicomSegImage->GetLabelSet(layer);
      auto labelIter = labelSet->IteratorConstBegin();
      // Ignore background label
      ++labelIter;

      for (; labelIter != labelSet->IteratorConstEnd(); ++labelIter)
      {
        Label::Pointer label = labelIter->second;
        SetDICOMSegmentProperties(label);
      }
    }
  }

  void DICOMSegmentationPropertyHelper::SetDICOMSegmentProperties(Label *label)
  {
    PropertyList::Pointer propertyList = PropertyList::New();

    AnatomicalStructureColorPresets::Category category;
    AnatomicalStructureColorPresets::Type type;
    auto presets = vtkSmartPointer<AnatomicalStructureColorPresets>::New();
    presets->LoadPreset();

    for (const auto &preset : presets->GetCategoryPresets())
    {
      auto presetOrganName = preset.first;
      if (label->GetName().compare(presetOrganName) == 0)
      {
        category = preset.second;
        break;
      }
    }

    for (const auto &preset : presets->GetTypePresets())
    {
      auto presetOrganName = preset.first;
      if (label->GetName().compare(presetOrganName) == 0)
      {
        type = preset.second;
        break;
      }
    }

    //------------------------------------------------------------
    // Add Segment Sequence tags (0062, 0002)
    // Segment Number:Identification number of the segment.The value of Segment Number(0062, 0004) shall be unique
    // within the Segmentation instance in which it is created
    label->SetProperty(DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_NUMBER_PATH()).c_str(),
      TemporoSpatialStringProperty::New(std::to_string(label->GetValue())));

    // Segment Label: User-defined label identifying this segment.
    label->SetProperty(DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_LABEL_PATH()).c_str(),
      TemporoSpatialStringProperty::New(label->GetName()));

    // Segment Algorithm Type: Type of algorithm used to generate the segment. AUTOMATIC SEMIAUTOMATIC MANUAL
    label->SetProperty(DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_ALGORITHM_TYPE_PATH()).c_str(),
      TemporoSpatialStringProperty::New("SEMIAUTOMATIC"));
    //------------------------------------------------------------
    // Add Segmented Property Category Code Sequence tags (0062, 0003): Sequence defining the general category of this
    // segment.
    // (0008,0100) Code Value
    if (!category.codeValue.empty())
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_VALUE_PATH()).c_str(),
        TemporoSpatialStringProperty::New(category.codeValue));

    // (0008,0102) Coding Scheme Designator
    if (!category.codeScheme.empty())
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_SCHEME_PATH()).c_str(),
        TemporoSpatialStringProperty::New(category.codeScheme));

    // (0008,0104) Code Meaning
    if (!category.codeName.empty())
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_CATEGORY_CODE_MEANING_PATH()).c_str(),
        TemporoSpatialStringProperty::New(category.codeName));
    //------------------------------------------------------------
    // Add Segmented Property Type Code Sequence (0062, 000F): Sequence defining the specific property type of this
    // segment.
    // (0008,0100) Code Value
    if (!type.codeValue.empty())
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_VALUE_PATH()).c_str(),
        TemporoSpatialStringProperty::New(type.codeValue));

    // (0008,0102) Coding Scheme Designator
    if (!type.codeScheme.empty())
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_SCHEME_PATH()).c_str(),
        TemporoSpatialStringProperty::New(type.codeScheme));

    // (0008,0104) Code Meaning
    if (!type.codeName.empty())
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_TYPE_CODE_MEANING_PATH()).c_str(),
        TemporoSpatialStringProperty::New(type.codeName));
    //------------------------------------------------------------
    // Add Segmented Property Type Modifier Code Sequence (0062,0011): Sequence defining the modifier of the property
    // type of this segment.
    // (0008,0100) Code Value
    if (!type.modifier.codeValue.empty())
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_VALUE_PATH()).c_str(),
        TemporoSpatialStringProperty::New(type.modifier.codeValue));

    // (0008,0102) Coding Scheme Designator
    if (!type.modifier.codeScheme.empty())
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_SCHEME_PATH()).c_str(),
        TemporoSpatialStringProperty::New(type.modifier.codeScheme));

    // (0008,0104) Code Meaning
    if (!type.modifier.codeName.empty())
      label->SetProperty(
        DICOMTagPathToPropertyName(DICOMSegmentationConstants::SEGMENT_MODIFIER_CODE_MEANING_PATH()).c_str(),
        TemporoSpatialStringProperty::New(type.modifier.codeName));
  }
}
