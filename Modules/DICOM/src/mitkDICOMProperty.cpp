/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMProperty.h>

#include <mitkIPropertyProvider.h>


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

std::map<std::string, mitk::BaseProperty::ConstPointer>
mitk::GetPropertyByDICOMTagPath(const mitk::IPropertyProvider* provider, const mitk::DICOMTagPath& path)
{
  std::map<std::string, mitk::BaseProperty::ConstPointer> result;

  if (nullptr == provider)
  {
    return result;
  }

  for (const auto& key : provider->GetPropertyKeys())
  {
    const DICOMTagPath propPath = PropertyNameToDICOMTagPath(key);
    if (!propPath.IsEmpty() && path.Equals(propPath))
    {
      auto baseProp = provider->GetConstProperty(key);
      if (baseProp.IsNotNull())
      {
        result.emplace(key, baseProp);
      }
    }
  }
  return result;
}

std::string
mitk::GetFirstDICOMValueAsString(const mitk::IPropertyProvider* provider, const mitk::DICOMTagPath& path)
{
  const auto matches = GetPropertyByDICOMTagPath(provider, path);
  if (matches.empty())
  {
    return {};
  }
  const auto* dicomProp =
    dynamic_cast<const mitk::DICOMProperty*>(matches.begin()->second.GetPointer());
  if (nullptr == dicomProp)
  {
    return {};
  }
  return dicomProp->GetValue(0, 0, true, true);
}
