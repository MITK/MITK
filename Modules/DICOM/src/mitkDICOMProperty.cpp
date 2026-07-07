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
  const auto& baseProp = matches.begin()->second;
  const auto* dicomProp =
    dynamic_cast<const mitk::DICOMProperty*>(baseProp.GetPointer());
  if (nullptr != dicomProp)
  {
    return dicomProp->GetValue(0, 0, true, true);
  }
  // Not a TemporoSpatialStringProperty (e.g. a uniform-value StringProperty
  // produced for a tag whose value is constant across all slices): fall back
  // to the generic string accessor so the helper works for any property kind
  // an IPropertyProvider may carry, not only the DICOM reader's output.
  return baseProp->GetValueAsString();
}
