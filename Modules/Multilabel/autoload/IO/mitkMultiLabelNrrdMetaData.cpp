/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMultiLabelNrrdMetaData.h>

#include <mitkCoreServices.h>
#include <mitkIPropertyPersistence.h>
#include <mitkItkImageIO.h>
#include <mitkMultiLabelIOHelper.h>
#include <mitkPropertyPersistenceInfo.h>

#include <algorithm>

bool mitk::MultiLabelReservedKeys::Matches(const std::string& name) const
{
  if (exact.count(name) > 0)
    return true;

  return std::any_of(prefixes.begin(), prefixes.end(),
    [&name](const std::string& prefix) { return name.compare(0, prefix.size(), prefix) == 0; });
}

std::vector<std::string> mitk::MultiLabelReservedKeys::GetAll() const
{
  std::vector<std::string> result(exact.begin(), exact.end());
  result.insert(result.end(), prefixes.begin(), prefixes.end());
  return result;
}

const mitk::MultiLabelReservedKeys& mitk::GetMultiLabelReservedKeys()
{
  static const MultiLabelReservedKeys keys = {
    { "org.mitk.uid", "modality", "path", PROPERTY_NAME_TIMEGEOMETRY_TYPE, PROPERTY_NAME_TIMEGEOMETRY_TIMEPOINTS },
    { "NRRD.", "ITK.", "MITK.IO.", "org.mitk.multilabel." } };
  return keys;
}

const mitk::MultiLabelReservedKeys& mitk::GetLegacyLabelSetReservedKeys()
{
  static const MultiLabelReservedKeys keys = [] {
    auto result = GetMultiLabelReservedKeys();
    result.exact.insert("layers");
    result.prefixes.insert(result.prefixes.end(), { "layer.", "label.", "org.mitk.label." });
    return result;
  }();
  return keys;
}

mitk::PropertyList::Pointer mitk::ExtractSegmentationPropertiesFromNrrdHeader(const itk::MetaDataDictionary& dictionary,
                                                                              const std::string& mimeTypeName,
                                                                              const MultiLabelReservedKeys& reservedKeys)
{
  auto properties = ItkImageIO::ExtractMetaDataAsPropertyList(dictionary, mimeTypeName, reservedKeys.GetAll());
  RemoveReservedProperties(properties, reservedKeys);

  // ItkImageIO matches its default keys as prefixes, so it registers no persistence info for a
  // name that merely starts with an exact one; without an info, the next save would drop it.
  CoreServicePointer<IPropertyPersistence> persistence(CoreServices::GetPropertyPersistence());
  for (const auto& key : dictionary.GetKeys())
  {
    auto name = key;
    std::replace(name.begin(), name.end(), '_', '.');
    if (properties->GetConstProperty(name).IsNull() || !persistence->GetInfo(name, mimeTypeName, true).empty())
      continue;

    auto info = PropertyPersistenceInfo::New();
    info->SetNameAndKey(name, key);
    info->SetMimeTypeName(PropertyPersistenceInfo::ANY_MIMETYPE_NAME());
    persistence->AddInfo(info);
  }

  return properties;
}

void mitk::RemoveReservedProperties(PropertyList* properties, const MultiLabelReservedKeys& reservedKeys)
{
  for (const auto& name : properties->GetPropertyKeys())
  {
    if (reservedKeys.Matches(name))
      properties->RemoveProperty(name);
  }
}
