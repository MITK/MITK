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

void mitk::CheckRestMaskTimeSteps(const RestTimeSelection& selection, const DataNode* imageNode,
  const DataNode* maskNode, const std::vector<TimeStepType>& imageTimeSteps, const std::string& path)
{
  const auto* imageTime = imageNode->GetData()->GetTimeGeometry();
  const auto* maskTime = maskNode->GetData()->GetTimeGeometry();

  // Numbers written as JSON writes them, which does not depend on the locale.
  const auto ms = [](TimePointType timePoint) { return nlohmann::json(timePoint).dump() + " ms"; };

  for (const auto timeStep : imageTimeSteps)
  {
    const auto begin = imageTime->TimeStepToTimePoint(timeStep);
    const bool hasData = maskTime->IsValidTimePoint(begin);

    if (selection.All)
    {
      if (!hasData)
      {
        ThrowRestError("NO_DATA_AT_TIME_POINT", "Unprocessable Content", "\"" + maskNode->GetName() + "\" has no data at " +
          ms(begin) + ", a time point of \"" + imageNode->GetName() + "\". Its time_steps say which time points it covers.", 422, path);
      }

      // The statistics take the mask where the time step of the image begins,
      // and a further time step of the mask within it would go unmeasured, as
      // all but the first of a dynamic segmentation on a static image.
      const auto maskStep = maskTime->TimePointToTimeStep(begin);
      const auto end = imageTime->GetTimeBounds(timeStep)[1];

      if (maskStep + 1 < maskTime->CountTimeSteps() && maskTime->GetTimeBounds(maskStep + 1)[0] < end)
      {
        ThrowRestError("NO_DATA_AT_TIME_POINT", "Unprocessable Content", "\"" + maskNode->GetName() +
          "\" has more than one time step within the time step of \"" + imageNode->GetName() + "\" that begins at " +
          ms(begin) + ", and the statistics would take only the one there. Their time_steps say which time points they cover.",
          422, path);
      }
    }
    else if (!hasData || maskTime->TimePointToTimeStep(begin) != maskTime->TimePointToTimeStep(selection.TimePoint))
    {
      ThrowRestError("NO_DATA_AT_TIME_POINT", "Unprocessable Content", "The statistics take \"" + maskNode->GetName() + "\" at " +
        ms(begin) + ", where the time step of \"" + imageNode->GetName() + "\" at " + ms(selection.TimePoint) + " begins, and \"" +
        maskNode->GetName() + "\" has " + (hasData ? "other data there than at " + ms(selection.TimePoint) : std::string("no data there")) +
        ". Its time_steps say which time points it covers.", 422, path);
    }
  }
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
