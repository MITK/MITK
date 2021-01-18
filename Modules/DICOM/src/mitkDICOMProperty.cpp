/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMProperty.h"


mitk::BaseProperty::Pointer
mitk::GetDICOMPropertyForDICOMValuesFunctor(const mitk::DICOMCachedValueLookupTable& cacheLookupTable)
{
  const auto& lookupTable = cacheLookupTable.GetLookupTable();
  mitk::DICOMProperty::Pointer prop = mitk::DICOMProperty::New();

  for (const auto &element : lookupTable)
  {
    prop->SetValue(element.second.TimePoint, element.second.SliceInTimePoint, element.second.Value);
  }

  return prop.GetPointer();
}

std::map< std::string, mitk::BaseProperty::Pointer> mitk::GetPropertyByDICOMTagPath(const mitk::PropertyList* list, const mitk::DICOMTagPath& path)
{
  std::map< std::string, mitk::BaseProperty::Pointer> result;

  for (const auto& iter : *(list->GetMap()))
  {
    DICOMTagPath propPath = PropertyNameToDICOMTagPath(iter.first);
    if (!propPath.IsEmpty() && path.Equals(propPath))
    {
      result.insert(iter);
    }
  }
  return result;
};

std::map< std::string, mitk::BaseProperty::Pointer> mitk::GetPropertyByDICOMTagPath(const mitk::BaseData* data, const mitk::DICOMTagPath& path)
{
  std::map< std::string, mitk::BaseProperty::Pointer> result;

  if (data)
  {
    result = GetPropertyByDICOMTagPath(data->GetPropertyList(), path);
  }

  return result;
};
