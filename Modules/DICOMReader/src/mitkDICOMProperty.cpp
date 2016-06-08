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

#include "mitkDICOMProperty.h"


mitk::BaseProperty::Pointer
mitk::GetDICOMPropertyForDICOMValuesFunctor(const mitk::DICOMCachedValueLookupTable& cacheLookupTable)
{
  const auto& lookupTable = cacheLookupTable.GetLookupTable();
  mitk::DICOMProperty::Pointer prop = mitk::DICOMProperty::New();

  for (auto element : lookupTable)
  {
    prop->SetValue(element.second.TimePoint, element.second.SliceInTimePoint, element.second.Value);
  }

  return prop.GetPointer();
}
