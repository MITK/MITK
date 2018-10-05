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
#include <mitkDICOMTag.h>
#include <mitkIDICOMTagsOfInterest.h>
#include <mitkLabel.h>
#include <mitkPropertyList.h>
#include <mitkPropertyNameHelper.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <MitkMultilabelExports.h>

// us
#include <usGetModuleContext.h>
#include <usModuleContext.h>

#include <vtkSmartPointer.h>

namespace mitk
{
  struct MITKMULTILABEL_EXPORT DICOMSegmentationPropertyHandler
  {
    static PropertyList::Pointer GetDICOMSegmentationProperties(PropertyList *referencedPropertyList)
    {
      PropertyList::Pointer propertyList = PropertyList::New();

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

      // Check if original image is a DICOM image; if so, store relevant DICOM Tags into the PropertyList of new
      // segmentation image
      bool parentIsDICOM = false;

      for (const auto &element : *(referencedPropertyList->GetMap()))
      {
        if (element.first.find("DICOM") == 0)
        {
          parentIsDICOM = true;
          break;
        }
      }

      if (!parentIsDICOM)
        return propertyList;

      //====== Patient information ======

      // Add DICOM Tag (0010,0010) patient's name; default "No Name"
      SetReferenceDICOMProperty(referencedPropertyList, propertyList, DICOMTag(0x0010, 0x0010), "NO NAME");
      // Add DICOM Tag (0010,0020) patient id; default "No Name"
      SetReferenceDICOMProperty(referencedPropertyList, propertyList, DICOMTag(0x0010, 0x0020), "NO NAME");
      // Add DICOM Tag (0010,0030) patient's birth date; no default
      SetReferenceDICOMProperty(referencedPropertyList, propertyList, DICOMTag(0x0010, 0x0030));
      // Add DICOM Tag (0010,0040) patient's sex; default "U" (Unknown)
      SetReferenceDICOMProperty(referencedPropertyList, propertyList, DICOMTag(0x0010, 0x0040), "U");

      //====== General study ======

      // Add DICOM Tag (0020,000D) Study Instance UID; no default --> MANDATORY!
      SetReferenceDICOMProperty(referencedPropertyList, propertyList, DICOMTag(0x0020, 0x000D));
      // Add DICOM Tag (0080,0020) Study Date; no default (think about "today")
      SetReferenceDICOMProperty(referencedPropertyList, propertyList, DICOMTag(0x0080, 0x0020));
      // Add DICOM Tag (0008,0050) Accession Number; no default
      SetReferenceDICOMProperty(referencedPropertyList, propertyList, DICOMTag(0x0008, 0x0050));
      // Add DICOM Tag (0008,1030) Study Description; no default
      SetReferenceDICOMProperty(referencedPropertyList, propertyList, DICOMTag(0x0008, 0x1030));

      //====== Reference DICOM data ======

      // Add reference file paths to referenced DICOM data
      BaseProperty::Pointer dcmFilesProp = referencedPropertyList->GetProperty("files");
      if (dcmFilesProp.IsNotNull())
        propertyList->SetProperty("referenceFiles", dcmFilesProp);

      return propertyList;
    }

    static void GetDICOMSegmentProperties(Label *label)
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

    static void SetReferenceDICOMProperty(PropertyList *referencedPropertyList,
                                          PropertyList *propertyList,
                                          const DICOMTag &tag,
                                          const std::string &defaultString = "")
    {
      std::string tagString = GeneratePropertyNameForDICOMTag(tag.GetGroup(), tag.GetElement());

      // Get DICOM property from referenced image
      BaseProperty::Pointer originalProperty = referencedPropertyList->GetProperty(tagString.c_str());

      // if property exists, copy the informtaion to the segmentation
      if (originalProperty.IsNotNull())
        propertyList->SetProperty(tagString.c_str(), originalProperty);
      else // use the default value, if there is one
      {
        if (!defaultString.empty())
          propertyList->SetProperty(tagString.c_str(), TemporoSpatialStringProperty::New(defaultString).GetPointer());
      }
    }
  };
}
