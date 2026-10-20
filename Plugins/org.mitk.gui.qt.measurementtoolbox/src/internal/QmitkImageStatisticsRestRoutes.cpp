/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkImageStatisticsRestRoutes.h"

#include <mitkDataStorage.h>
#include <mitkErrorResponse.h>
#include <mitkImageStatisticsCalculator.h>
#include <mitkImageStatisticsConstants.h>
#include <mitkIRestServerService.h>
#include <mitkLabelSetImage.h>
#include <mitkMultiLabelMaskGenerator.h>
#include <mitkRestGuiThread.h>
#include <mitkRestTime.h>

#include <cmath>

using mitk::CallInGuiThread;
using mitk::ErrorResponse;
using mitk::RestException;
using mitk::RestRequest;
using mitk::RestResponse;
using mitk::RestRoute;

namespace
{
  [[noreturn]] void Throw(const std::string& code, const std::string& title, const std::string& detail, int status, const std::string& path)
  {
    throw RestException(ErrorResponse::Create(code, title, detail, status, path));
  }

  mitk::DataNode::Pointer FindNode(const mitk::IRestServerService& server, const std::string& uid, const std::string& path)
  {
    auto node = server.FindNode(uid);

    if (node.IsNull())
      throw RestException(ErrorResponse::NodeNotFound(uid, path));

    return node;
  }

  /** The image a segmentation was created for: its source in the data storage. */
  mitk::DataNode::Pointer FindReferenceImage(const mitk::DataStorage* dataStorage, const mitk::DataNode* segmentation)
  {
    const auto sources = dataStorage->GetSources(segmentation, nullptr, true);

    for (const auto& source : *sources)
    {
      if (dynamic_cast<const mitk::Image*>(source->GetData()) != nullptr)
        return source;
    }

    return nullptr;
  }

  /**
   * The time steps of the data that the selection covers.
   *
   * \throw RestException with status 422 if the data has no time step at the selected time point.
   */
  std::vector<mitk::TimeStepType> GetTimeSteps(const mitk::RestTimeSelection& selection, const mitk::DataNode* node, const std::string& path)
  {
    auto timeSteps = selection.GetTimeSteps(node->GetData());

    if (timeSteps.empty())
    {
      Throw("NO_DATA_AT_TIME_POINT", "Unprocessable Content", "\"" + node->GetName() + "\" has no data at " +
        std::to_string(selection.TimePoint) + " ms. Its time_steps say which time points it covers.", 422, path);
    }

    return timeSteps;
  }

  /** Adds the measured values of a region; images without scalar pixels have only voxels and volume. */
  void AddValues(nlohmann::json& json, const mitk::ImageStatisticsContainer::ImageStatisticsObject& values)
  {
    using Constants = mitk::ImageStatisticsConstants;

    json["voxels"] = values.GetValueConverted<mitk::ImageStatisticsContainer::VoxelCountType>(Constants::NUMBEROFVOXELS());

    if (values.HasStatistic(Constants::VOLUME()))
      json["volume_ml"] = std::round(values.GetValueConverted<double>(Constants::VOLUME()) / 10.0) / 100.0;

    if (!values.HasStatistic(Constants::MEAN()))
      return;

    json["mean"] = values.GetValueConverted<double>(Constants::MEAN());
    json["median"] = values.GetValueConverted<double>(Constants::MEDIAN());
    json["standard_deviation"] = values.GetValueConverted<double>(Constants::STANDARDDEVIATION());
    json["min"] = values.GetValueConverted<double>(Constants::MINIMUM());
    json["max"] = values.GetValueConverted<double>(Constants::MAXIMUM());
  }
}

std::vector<RestRoute> CreateImageStatisticsRoutes(mitk::IRestServerService* server)
{
  std::vector<RestRoute> routes;

  RestRoute statistics;
  statistics.Method = "GET";
  statistics.Path = "/statistics";
  statistics.Summary = "Measure the grey values of an image, in each label of a segmentation or as a whole";
  statistics.Description =
    "As the Image Statistics view measures them. Query parameters: image, the uid of an image; mask (optional), the "
    "uid of a segmentation of it; timepoint_ms (optional), a time point in milliseconds, or all for each time step "
    "of the image; the selected time point without it. With a mask, image is optional and defaults to the image the "
    "segmentation belongs to, its parent in the Data Manager. Returns {data: {image: {uid, name, dimensions, "
    "spacing_mm, time_steps: [{min_timepoint_ms, max_timepoint_ms}]}, mask: {uid, name} or null, timepoint_ms, "
    "statistics: [{value, name, group, voxels, volume_ml, mean, median, standard_deviation, min, max}]}}: one entry "
    "per label, or one for the whole image without value, name and group. With all, each entry has time_steps: "
    "[{min_timepoint_ms, max_timepoint_ms, voxels, ...}] instead of the values. Grey values are in the image's "
    "unit, Hounsfield units for CT; empty labels have none. Status 422 if the image or mask has no data at the "
    "time point.";
  statistics.Handler = [server](const RestRequest& request) {
    const auto imageUid = request.GetQueryParam("image");
    const auto maskUid = request.GetQueryParam("mask");

    if (!imageUid.has_value() && !maskUid.has_value())
      throw RestException(ErrorResponse::InvalidRequest("Name an image, a mask or both: ?image=<uid>&mask=<uid>.", request.Path));

    auto imageNode = imageUid.has_value() ? FindNode(*server, *imageUid, request.Path) : mitk::DataNode::Pointer();
    const auto maskNode = maskUid.has_value() ? FindNode(*server, *maskUid, request.Path) : mitk::DataNode::Pointer();
    const auto dataStorage = server->GetDataStorage();

    if (dataStorage.IsNull())
      throw RestException(ErrorResponse::DataStorageNotAvailable(request.Path));

    mitk::Image::ConstPointer image;
    mitk::MultiLabelSegmentation::ConstPointer segmentation;
    auto labels = nlohmann::json::array();
    mitk::RestTimeSelection selection;
    std::vector<mitk::TimeStepType> timeSteps;

    CallInGuiThread(*server, [&]() {
      if (maskNode.IsNotNull())
      {
        auto* mask = dynamic_cast<mitk::MultiLabelSegmentation*>(maskNode->GetData());

        if (mask == nullptr)
          Throw("NOT_A_SEGMENTATION", "Unprocessable Content", "The mask \"" + maskNode->GetName() + "\" is no segmentation.", 422, request.Path);

        segmentation = mask;

        for (const auto& label : mask->GetLabels())
        {
          const auto value = label->GetValue();
          labels.push_back({ { "value", value }, { "name", label->GetName() }, { "group", mask->GetGroupIndexOfLabel(value) } });
        }

        if (imageNode.IsNull())
          imageNode = FindReferenceImage(dataStorage, maskNode);

        if (imageNode.IsNull())
        {
          Throw("REFERENCE_NOT_FOUND", "Unprocessable Content", "The segmentation \"" + maskNode->GetName() +
            "\" belongs to no image. Name one with the query parameter image.", 422, request.Path);
        }
      }

      image = dynamic_cast<const mitk::Image*>(imageNode->GetData());

      if (image.IsNull())
        Throw("NOT_AN_IMAGE", "Unprocessable Content", "\"" + imageNode->GetName() + "\" is no image.", 422, request.Path);

      selection = mitk::RestTimeSelection::FromRequest(request);
      timeSteps = GetTimeSteps(selection, imageNode, request.Path);

      if (maskNode.IsNotNull())
        GetTimeSteps(selection, maskNode, request.Path);
    });

    // Measured here instead of the GUI thread, as the Image Statistics view does in a worker.
    auto calculator = mitk::ImageStatisticsCalculator::New();
    calculator->SetInputImage(image);

    if (segmentation.IsNotNull())
    {
      auto maskGenerator = mitk::MultiLabelMaskGenerator::New();
      maskGenerator->SetMultiLabelSegmentation(segmentation);
      calculator->SetMask(maskGenerator);
    }

    const mitk::ImageStatisticsContainer* measured = nullptr;

    try
    {
      measured = calculator->GetStatistics();
    }
    catch (const itk::ExceptionObject& e)
    {
      Throw("STATISTICS_FAILED", "Unprocessable Content", e.GetDescription(), 422, request.Path);
    }

    // Values of a region at a time step; empty labels have no statistics of their own.
    const auto addValues = [measured](nlohmann::json& json, mitk::ImageStatisticsContainer::LabelValueType value, mitk::TimeStepType timeStep) {
      if (measured->StatisticsExist(value, timeStep))
      {
        AddValues(json, measured->GetStatistics(value, timeStep));
      }
      else
      {
        json["voxels"] = 0;
        json["volume_ml"] = 0.0;
      }
    };

    const auto addEntry = [&](nlohmann::json entry, mitk::ImageStatisticsContainer::LabelValueType value) {
      if (selection.All)
      {
        auto steps = nlohmann::json::array();

        for (const auto timeStep : timeSteps)
        {
          auto step = mitk::DescribeRestTimeStep(image, timeStep);
          addValues(step, value, timeStep);
          steps.push_back(std::move(step));
        }

        entry["time_steps"] = std::move(steps);
      }
      else
      {
        addValues(entry, value, timeSteps.front());
      }

      return entry;
    };

    auto entries = nlohmann::json::array();

    if (segmentation.IsNull())
    {
      entries.push_back(addEntry(nlohmann::json::object(), mitk::ImageStatisticsContainer::NO_MASK_LABEL_VALUE));
    }
    else
    {
      for (const auto& label : labels)
        entries.push_back(addEntry(label, label["value"].get<mitk::MultiLabelSegmentation::LabelValueType>()));
    }

    auto dimensions = nlohmann::json::array();

    for (unsigned int i = 0; i < image->GetDimension(); ++i)
      dimensions.push_back(image->GetDimension(i));

    const auto spacing = image->GetGeometry()->GetSpacing();

    nlohmann::json mask = nullptr;

    if (maskNode.IsNotNull())
      mask = { { "uid", server->GetNodeUid(maskNode) }, { "name", maskNode->GetName() } };

    return RestResponse::FromData({
      { "image", {
        { "uid", server->GetNodeUid(imageNode) },
        { "name", imageNode->GetName() },
        { "dimensions", dimensions },
        { "spacing_mm", { spacing[0], spacing[1], spacing[2] } },
        { "time_steps", mitk::DescribeRestTimeSteps(image) } } },
      { "mask", mask },
      { "timepoint_ms", selection.ToJson() },
      { "statistics", entries }
    });
  };
  routes.push_back(statistics);

  return routes;
}
