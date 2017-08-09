/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY
{
}
 without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkPropertyNameHelper.h"
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

bool mitk::GetBackwardsCompatibleDICOMProperty(unsigned int group,
                                               unsigned int element,
                                               std::string const &backwardsCompatiblePropertyName,
                                               mitk::PropertyList const *propertyList,
                                               std::string &propertyValue)
{
  propertyValue = "";

  BaseProperty *prop = propertyList->GetProperty(mitk::GeneratePropertyNameForDICOMTag(group, element).c_str());

  if (prop)
  { // may not be a string property so use the generic access.
    propertyValue = prop->GetValueAsString();
  }

  if (!propertyValue.empty() || propertyList->GetStringProperty(backwardsCompatiblePropertyName.c_str(), propertyValue))
  // 2nd part is for backwards compatibility with the old property naming style
  {
    return true;
  }

  return false;
};
