/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestImageStatistics.h>

#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <mitkImageStatisticsConstants.h>
#include <mitkRestHelpers.h>

mitk::DataNodeHandle mitk::FindRestReferenceImage(const IRestServerService& server, const DataStorage* dataStorage, const DataNode* mask)
{
  const auto sources = dataStorage->GetSources(mask, nullptr, true);

  for (const auto& source : *sources)
  {
    if (dynamic_cast<const Image*>(source->GetData()) != nullptr)
      return server.FindNode(server.GetNodeUid(source));
  }

  return nullptr;
}

std::vector<mitk::TimeStepType> mitk::GetRestTimeSteps(const RestTimeSelection& selection, const DataNode* node, const std::string& path)
{
  auto timeSteps = selection.GetTimeSteps(node->GetData());

  if (timeSteps.empty())
  {
    // Written as JSON writes the number, which does not depend on the locale.
    ThrowRestError("NO_DATA_AT_TIME_POINT", "Unprocessable Content", "\"" + node->GetName() + "\" has no data at " +
      nlohmann::json(selection.TimePoint).dump() + " ms. Its time_steps say which time points it covers.", 422, path);
  }

  return timeSteps;
}

void mitk::AddRestStatisticsValues(nlohmann::json& json, const ImageStatisticsContainer::ImageStatisticsObject& values)
{
  using Constants = ImageStatisticsConstants;

  json["voxels"] = values.GetValueConverted<ImageStatisticsContainer::VoxelCountType>(Constants::NUMBEROFVOXELS());

  if (values.HasStatistic(Constants::VOLUME()))
    json["volume_ml"] = values.GetValueConverted<double>(Constants::VOLUME()) / 1000.0;

  if (!values.HasStatistic(Constants::MEAN()))
    return;

  json["mean"] = values.GetValueConverted<double>(Constants::MEAN());
  json["median"] = values.GetValueConverted<double>(Constants::MEDIAN());
  json["standard_deviation"] = values.GetValueConverted<double>(Constants::STANDARDDEVIATION());
  json["min"] = values.GetValueConverted<double>(Constants::MINIMUM());
  json["max"] = values.GetValueConverted<double>(Constants::MAXIMUM());
}
