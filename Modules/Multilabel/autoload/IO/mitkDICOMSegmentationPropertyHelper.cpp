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
#include <mitkDICOMProperty.h>
#include <mitkDICOMTag.h>
#include <mitkIDICOMTagsOfInterest.h>
#include <mitkLabel.h>
#include <mitkPropertyList.h>
#include <mitkPropertyNameHelper.h>
#include <mitkStringProperty.h>

#include "MitkMultilabelIOExports.h"

// us
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk
{
  struct MITKMULTILABELIO_EXPORT DICOMSegmentationPropertyHandler
  {
    static PropertyList::Pointer GetDICOMSegmentationProperties(PropertyList *referencedPropertyList)
    {
      PropertyList::Pointer propertyList = PropertyList::New();

      // Add DICOM Tag (0008, 0060) Modality "SEG"
      propertyList->SetProperty(GeneratePropertyNameForDICOMTag(0x0008, 0x0060).c_str(), StringProperty::New("SEG"));
      // Add DICOM Tag (0008,103E) Series Description
      propertyList->SetProperty(GeneratePropertyNameForDICOMTag(0x0008, 0x103E).c_str(),
                                StringProperty::New("MITK Segmentation"));

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
      AnatomicalStructureColorPresets *anatomicalStructureColorPresets = AnatomicalStructureColorPresets::New();
      anatomicalStructureColorPresets->LoadPreset();

      for (const auto &preset : anatomicalStructureColorPresets->GetCategoryPresets())
      {
        auto presetOrganName = preset.first;
        if (label->GetName().compare(presetOrganName) == 0)
        {
          category = preset.second;
          break;
        }
      }

      for (const auto &preset : anatomicalStructureColorPresets->GetTypePresets())
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
      DICOMTagPath segmentSequencePath = DICOMTagPath().AddElement(0x0062, 0x0002);

      // Segment Number:Identification number of the segment.The value of Segment Number(0062, 0004) shall be unique
      // within the Segmentation instance in which it is created
      DICOMTagPath segmentNumberPath = DICOMTagPath(segmentSequencePath).AddElement(0x0062, 0x0004);
      label->SetProperty(DICOMTagPathToPropertyName(segmentNumberPath).c_str(),
                         StringProperty::New(std::to_string(label->GetValue())));

      // Segment Label: User-defined label identifying this segment.
      DICOMTagPath segmentLabelPath = DICOMTagPath(segmentSequencePath).AddElement(0x0062, 0x0005);
      label->SetProperty(DICOMTagPathToPropertyName(segmentLabelPath).c_str(), StringProperty::New(label->GetName()));

      // Segment Algorithm Type: Type of algorithm used to generate the segment. AUTOMATIC SEMIAUTOMATIC MANUAL
      DICOMTagPath segmentAlgorithmTypePath = DICOMTagPath(segmentSequencePath).AddElement(0x0062, 0x0008);
      label->SetProperty(DICOMTagPathToPropertyName(segmentAlgorithmTypePath).c_str(),
                         StringProperty::New("SEMIAUTOMATIC"));
      //------------------------------------------------------------
      // Add Segmented Property Category Code Sequence tags (0062, 0003): Sequence defining the general category of this
      // segment.
      DICOMTagPath segmentedPropertyCategorySequencePath = DICOMTagPath(segmentSequencePath).AddElement(0x0062, 0x0003);
      // (0008,0100) Code Value
      DICOMTagPath segmentCategoryCodeValuePath =
        DICOMTagPath(segmentedPropertyCategorySequencePath).AddElement(0x008, 0x0100);
      if (!category.codeValue.empty())
        label->SetProperty(DICOMTagPathToPropertyName(segmentCategoryCodeValuePath).c_str(),
                           StringProperty::New(category.codeValue));

      // (0008,0102) Coding Scheme Designator
      DICOMTagPath segmentCategoryCodeSchemePath =
        DICOMTagPath(segmentedPropertyCategorySequencePath).AddElement(0x008, 0x0102);
      if (!category.codeScheme.empty())
        label->SetProperty(DICOMTagPathToPropertyName(segmentCategoryCodeSchemePath).c_str(),
                           StringProperty::New(category.codeScheme));

      // (0008,0104) Code Meaning
      DICOMTagPath segmentCategoryCodeMeaningPath =
        DICOMTagPath(segmentedPropertyCategorySequencePath).AddElement(0x008, 0x0104);
      if (!category.codeName.empty())
        label->SetProperty(DICOMTagPathToPropertyName(segmentCategoryCodeMeaningPath).c_str(),
                           StringProperty::New(category.codeName));
      //------------------------------------------------------------
      // Add Segmented Property Type Code Sequence (0062, 000F): Sequence defining the specific property type of this
      // segment.
      DICOMTagPath segmentedPropertyTypeSequencePath = DICOMTagPath(segmentSequencePath).AddElement(0x0062, 0x000F);

      // (0008,0100) Code Value
      DICOMTagPath segmentTypeCodeValuePath = DICOMTagPath(segmentedPropertyTypeSequencePath).AddElement(0x008, 0x0100);
      if (!type.codeValue.empty())
        label->SetProperty(DICOMTagPathToPropertyName(segmentTypeCodeValuePath).c_str(),
                           StringProperty::New(type.codeValue));

      // (0008,0102) Coding Scheme Designator
      DICOMTagPath segmentTypeCodeSchemePath =
        DICOMTagPath(segmentedPropertyTypeSequencePath).AddElement(0x008, 0x0102);
      if (!type.codeScheme.empty())
        label->SetProperty(DICOMTagPathToPropertyName(segmentTypeCodeSchemePath).c_str(),
                           StringProperty::New(type.codeScheme));

      // (0008,0104) Code Meaning
      DICOMTagPath segmentTypeCodeMeaningPath =
        DICOMTagPath(segmentedPropertyTypeSequencePath).AddElement(0x008, 0x0104);
      if (!type.codeName.empty())
        label->SetProperty(DICOMTagPathToPropertyName(segmentTypeCodeMeaningPath).c_str(),
                           StringProperty::New(type.codeName));
      //------------------------------------------------------------
      // Add Segmented Property Type Modifier Code Sequence (0062,0011): Sequence defining the modifier of the property
      // type of this segment.
      DICOMTagPath segmentedPropertyModifierSequencePath =
        DICOMTagPath(segmentedPropertyTypeSequencePath).AddElement(0x0062, 0x0011);
      // (0008,0100) Code Value
      DICOMTagPath segmentModifierCodeValuePath =
        DICOMTagPath(segmentedPropertyModifierSequencePath).AddElement(0x008, 0x0100);
      if (!type.modifier.codeValue.empty())
        label->SetProperty(DICOMTagPathToPropertyName(segmentModifierCodeValuePath).c_str(),
                           StringProperty::New(type.modifier.codeValue));

      // (0008,0102) Coding Scheme Designator
      DICOMTagPath segmentModifierCodeSchemePath =
        DICOMTagPath(segmentedPropertyModifierSequencePath).AddElement(0x008, 0x0102);
      if (!type.modifier.codeScheme.empty())
        label->SetProperty(DICOMTagPathToPropertyName(segmentModifierCodeSchemePath).c_str(),
                           StringProperty::New(type.modifier.codeScheme));

      // (0008,0104) Code Meaning
      DICOMTagPath segmentModifierCodeMeaningPath =
        DICOMTagPath(segmentedPropertyModifierSequencePath).AddElement(0x008, 0x0104);
      if (!type.modifier.codeName.empty())
        label->SetProperty(DICOMTagPathToPropertyName(segmentModifierCodeMeaningPath).c_str(),
                           StringProperty::New(type.modifier.codeName));

      //============================TODO: Not here:-)
      IDICOMTagsOfInterest *toiService = nullptr;

      std::vector<us::ServiceReference<IDICOMTagsOfInterest>> toiRegisters =
        us::GetModuleContext()->GetServiceReferences<IDICOMTagsOfInterest>();
      if (!toiRegisters.empty())
      {
        if (toiRegisters.size() > 1)
          MITK_WARN << "Multiple DICOM tags of interest services found. Using just one.";
        toiService = us::GetModuleContext()->GetService<IDICOMTagsOfInterest>(toiRegisters.front());
      }

      if (toiService != nullptr)
      {
        toiService->AddTagOfInterest(segmentSequencePath);

        toiService->AddTagOfInterest(segmentNumberPath);
        toiService->AddTagOfInterest(segmentLabelPath);
        toiService->AddTagOfInterest(segmentAlgorithmTypePath);

        toiService->AddTagOfInterest(segmentedPropertyCategorySequencePath);
        toiService->AddTagOfInterest(segmentCategoryCodeValuePath);
        toiService->AddTagOfInterest(segmentCategoryCodeSchemePath);
        toiService->AddTagOfInterest(segmentCategoryCodeMeaningPath);

        toiService->AddTagOfInterest(segmentedPropertyTypeSequencePath);
        toiService->AddTagOfInterest(segmentTypeCodeValuePath);
        toiService->AddTagOfInterest(segmentTypeCodeSchemePath);
        toiService->AddTagOfInterest(segmentTypeCodeMeaningPath);

        toiService->AddTagOfInterest(segmentedPropertyModifierSequencePath);
        toiService->AddTagOfInterest(segmentModifierCodeValuePath);
        toiService->AddTagOfInterest(segmentModifierCodeSchemePath);
        toiService->AddTagOfInterest(segmentModifierCodeMeaningPath);
      }
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
          propertyList->SetProperty(tagString.c_str(), StringProperty::New(defaultString).GetPointer());
      }
    }
  };
}
