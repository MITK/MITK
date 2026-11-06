/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMultiLabelNrrdMetaData.h>

#include <mitkItkImageIO.h>
#include <mitkMultiLabelIOHelper.h>

#include <algorithm>

std::vector<std::string> mitk::GetMultiLabelNrrdMetaDataKeys()
{
  return { "org.mitk.uid", "NRRD.space", "NRRD.kinds", PROPERTY_NAME_TIMEGEOMETRY_TYPE,
           PROPERTY_NAME_TIMEGEOMETRY_TIMEPOINTS, "ITK.InputFilterName", "org.mitk.multilabel.", "MITK.IO.",
           "modality" };
}

std::vector<std::string> mitk::GetLegacyLabelSetNrrdMetaDataKeys()
{
  auto keys = GetMultiLabelNrrdMetaDataKeys();
  keys.insert(keys.end(), { "layers", "layer.", "label.", "org.mitk.label." });
  return keys;
}

mitk::PropertyList::Pointer mitk::ExtractSegmentationPropertiesFromNrrdHeader(const itk::MetaDataDictionary& dictionary,
                                                                              const std::string& mimeTypeName,
                                                                              const std::vector<std::string>& metaDataKeys)
{
  auto properties = ItkImageIO::ExtractMetaDataAsPropertyList(dictionary, mimeTypeName, metaDataKeys);

  for (const auto& name : properties->GetPropertyKeys())
  {
    const auto isMetaData = [&name](const std::string& key) { return name.compare(0, key.size(), key) == 0; };
    if (std::any_of(metaDataKeys.begin(), metaDataKeys.end(), isMetaData))
      properties->RemoveProperty(name);
  }

  return properties;
}
