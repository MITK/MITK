/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPropertyNameHelper.h>
#include <iomanip>
#include <mitkPropertyList.h>
#include <sstream>

std::string mitk::GeneratePropertyNameForDICOMTag(unsigned int group, unsigned int element)
{
  std::ostringstream nameStream;
  nameStream << "DICOM."
    << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << group << std::nouppercase << "."
    << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << element;

  return nameStream.str();
};

bool mitk::GetDICOMPropertyValue(unsigned int group,
                                 unsigned int element,
                                 mitk::PropertyList const *propertyList,
                                 std::string &propertyValue)
{
  propertyValue = "";

  const BaseProperty *prop = propertyList->GetProperty(mitk::GeneratePropertyNameForDICOMTag(group, element).c_str());

  if (prop != nullptr)
  { // May not be a string property, so use the generic value access; this also
    // resolves the TemporoSpatialStringProperty that DICOM-tag properties use,
    // which PropertyList::GetStringProperty would silently miss.
    propertyValue = prop->GetValueAsString();
  }

  return !propertyValue.empty();
};

bool mitk::GetBackwardsCompatibleDICOMPropertyValue(unsigned int group,
                                                    unsigned int element,
                                                    std::string const &backwardsCompatiblePropertyName,
                                                    mitk::PropertyList const *propertyList,
                                                    std::string &propertyValue)
{
  if (mitk::GetDICOMPropertyValue(group, element, propertyList, propertyValue))
  {
    return true;
  }

  // Fall back to the old property naming style for backwards compatibility.
  return propertyList->GetStringProperty(backwardsCompatiblePropertyName.c_str(), propertyValue);
};
