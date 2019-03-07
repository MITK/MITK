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

#include <mitkIDICOMTagsOfInterest.h>
#include <mitkPropertyNameHelper.h>
#include <mitkTemporoSpatialStringProperty.h>
#include <mitkPropertyList.h>

#include "mitkDICOMQIPropertyHelper.h"

namespace mitk
{
  void DICOMQIPropertyHelper::DeriveDICOMSourceProperties(const BaseData *sourceDICOMImage, BaseData *derivedDICOMImage)
  {
    // Check if original image is a DICOM image; if so, store relevant DICOM Tags into the PropertyList of new
    // segmentation/PM image

    PropertyList::Pointer sourcePropertyList = sourceDICOMImage->GetPropertyList();
    bool parentIsDICOM = false;

    for (const auto &element : *(sourcePropertyList->GetMap()))
    {
      if (element.first.find("DICOM") == 0)
      {
        parentIsDICOM = true;
        break;
      }
    }

    if (!parentIsDICOM)
      return;


    PropertyList::Pointer propertyList = derivedDICOMImage->GetPropertyList();

    //====== Patient information ======
    // Add DICOM Tag (0010,0010) patient's name; default "No Name"
    AdoptReferenceDICOMProperty(sourcePropertyList, propertyList, DICOMTag(0x0010, 0x0010), "NO NAME");
    // Add DICOM Tag (0010,0020) patient id; default "No Name"
    AdoptReferenceDICOMProperty(sourcePropertyList, propertyList, DICOMTag(0x0010, 0x0020), "NO NAME");
    // Add DICOM Tag (0010,0030) patient's birth date; no default
    AdoptReferenceDICOMProperty(sourcePropertyList, propertyList, DICOMTag(0x0010, 0x0030));
    // Add DICOM Tag (0010,0040) patient's sex; default "U" (Unknown)
    AdoptReferenceDICOMProperty(sourcePropertyList, propertyList, DICOMTag(0x0010, 0x0040), "U");

    //====== General study ======
    // Add DICOM Tag (0020,000D) Study Instance UID; no default --> MANDATORY!
    AdoptReferenceDICOMProperty(sourcePropertyList, propertyList, DICOMTag(0x0020, 0x000D));
    // Add DICOM Tag (0080,0020) Study Date; no default (think about "today")
    AdoptReferenceDICOMProperty(sourcePropertyList, propertyList, DICOMTag(0x0080, 0x0020));
    // Add DICOM Tag (0008,0050) Accession Number; no default
    AdoptReferenceDICOMProperty(sourcePropertyList, propertyList, DICOMTag(0x0008, 0x0050));
    // Add DICOM Tag (0008,1030) Study Description; no default
    AdoptReferenceDICOMProperty(sourcePropertyList, propertyList, DICOMTag(0x0008, 0x1030));


    //====== Reference DICOM data ======
    // Add reference file paths to referenced DICOM data
    BaseProperty::Pointer dcmFilesProp = sourcePropertyList->GetProperty("files");
    if (dcmFilesProp.IsNotNull())
      propertyList->SetProperty("referenceFiles", dcmFilesProp);
  }

  void DICOMQIPropertyHelper::AdoptReferenceDICOMProperty(PropertyList *referencedPropertyList,
    PropertyList *propertyList,
    const DICOMTag &tag,
    const std::string &defaultString)
  {
    std::string tagString = GeneratePropertyNameForDICOMTag(tag.GetGroup(), tag.GetElement());

    // Get DICOM property from referenced image
    BaseProperty::Pointer originalProperty = referencedPropertyList->GetProperty(tagString.c_str());
	
    // if property exists, copy the informtaion to the derived image
    if (originalProperty.IsNotNull())
      propertyList->SetProperty(tagString.c_str(), originalProperty);
    else // use the default value, if there is one
    {
      if (!defaultString.empty())
        propertyList->SetProperty(tagString.c_str(), TemporoSpatialStringProperty::New(defaultString).GetPointer());
    }
  }
}
