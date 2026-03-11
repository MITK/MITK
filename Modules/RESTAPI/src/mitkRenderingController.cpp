/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRenderingController.h"
#include "mitkRenderWindowBridge.h"
#include "mitkErrorResponse.h"
#include <mitkDataStorage.h>
#include <mitkException.h>
#include <mitkRenderingManager.h>
#include <mitkStepper.h>
#include <mitkTimeNavigationController.h>

#include <algorithm>
#include <limits>
#include <optional>
#include <vector>

namespace mitk
{

RenderingController::RenderingController(DataStorageBridge& bridge)
  : m_Bridge(bridge)
{
}

void RenderingController::SetDispatcher(StorageThreadDispatcherBase* dispatcher)
{
  m_Dispatcher = dispatcher;
}

void RenderingController::SetRenderWindowBridge(RenderWindowBridge* bridge)
{
  m_RenderWindowBridge = bridge;
}

void RenderingController::HandlePOST_update(const httplib::Request& req, httplib::Response& res)
{
  auto type = RenderingManager::REQUEST_UPDATE_ALL;

  if (!req.body.empty())
  {
    try
    {
      const auto body = nlohmann::json::parse(req.body);
      if (body.contains("type"))
      {
        const auto typeStr = body["type"].get<std::string>();
        if (typeStr == "all")
        {
          type = RenderingManager::REQUEST_UPDATE_ALL;
        }
        else if (typeStr == "2d")
        {
          type = RenderingManager::REQUEST_UPDATE_2DWINDOWS;
        }
        else if (typeStr == "3d")
        {
          type = RenderingManager::REQUEST_UPDATE_3DWINDOWS;
        }
        else
        {
          const auto error = ErrorResponse::InvalidRequest(
            "Invalid type '" + typeStr + "'. Must be 'all', '2d', or '3d'.", req.path);
          this->SendErrorResponse(res, 400, error);
          return;
        }
      }
    }
    catch (const nlohmann::json::exception&)
    {
      const auto error = ErrorResponse::InvalidRequest("Invalid JSON body.", req.path);
      this->SendErrorResponse(res, 400, error);
      return;
    }
  }

  try
  {
    this->Dispatch([type]() {
      RenderingManager::GetInstance()->RequestUpdateAll(type);
    });
    res.status = 204;
  }
  catch (const mitk::Exception& e)
  {
    const auto error = ErrorResponse::RenderingError(
      std::string("Rendering operation failed: ") + e.what(), req.path);
    this->SendErrorResponse(res, 422, error);
  }
  catch (const std::exception& e)
  {
    const auto error = ErrorResponse::InternalError(
      std::string("Unexpected error during rendering: ") + e.what(), req.path);
    this->SendErrorResponse(res, 500, error);
  }
}

void RenderingController::HandlePOST_reinit(const httplib::Request& req, httplib::Response& res)
{
  if (!m_Bridge.HasDataStorage())
  {
    const auto error = ErrorResponse::DataStorageNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  // Parse optional body for node-scoped reinit.
  std::vector<std::string> uids;

  if (!req.body.empty())
  {
    try
    {
      const auto body = nlohmann::json::parse(req.body);
      if (body.contains("uids"))
      {
        const auto& uidsJson = body["uids"];
        if (!uidsJson.is_array() || uidsJson.empty())
        {
          const auto error = ErrorResponse::InvalidRequest(
            "'uids' must be a non-empty array of strings.", req.path);
          this->SendErrorResponse(res, 400, error);
          return;
        }
        for (const auto& item : uidsJson)
        {
          if (!item.is_string())
          {
            const auto error = ErrorResponse::InvalidRequest(
              "'uids' must be a non-empty array of strings.", req.path);
            this->SendErrorResponse(res, 400, error);
            return;
          }
          uids.push_back(item.get<std::string>());
        }
      }
    }
    catch (const nlohmann::json::exception&)
    {
      const auto error = ErrorResponse::InvalidRequest("Invalid JSON body.", req.path);
      this->SendErrorResponse(res, 400, error);
      return;
    }
  }

  if (!uids.empty())
  {
    // Node-scoped reinit: fit views to the bounding geometry of the specified nodes.
    // Pre-dispatch: validate all UIDs and capture node pointers (no cloning).
    std::vector<DataNode::ConstPointer> nodes;
    nodes.reserve(uids.size());
    for (const auto& uid : uids)
    {
      const auto node = m_Bridge.FindDataNode(uid);
      if (node == nullptr)
      {
        this->SendErrorResponse(res, 404, ErrorResponse::NodeNotFound(uid, req.path));
        return;
      }
      nodes.push_back(node);
    }

    // Dispatch: validate geometry and compute bounding box on the UI thread.
    const auto dataStorage = m_Bridge.GetDataStorage();
    std::optional<std::pair<int, nlohmann::json>> dispatchError;

    try
    {
      this->Dispatch([dataStorage, nodes, uids, &req, &dispatchError]()
      {
        auto nodeSet = DataStorage::ConstSetOfObjects::New();
        for (std::size_t i = 0; i < nodes.size(); ++i)
        {
          const auto data = nodes[i]->GetData();
          if (data == nullptr)
          {
            dispatchError = {422, ErrorResponse::NoData(uids[i], req.path)};
            return;
          }
          if (data->GetTimeGeometry() == nullptr)
          {
            dispatchError = {422, ErrorResponse::NoGeometry(uids[i], req.path)};
            return;
          }
          nodeSet->InsertElement(nodeSet->Size(), nodes[i]);
        }

        const auto geometry = dataStorage->ComputeBoundingGeometry3D(nodeSet);
        if (geometry == nullptr)
        {
          // Defensive: should not happen after per-node validation above.
          dispatchError = {500, ErrorResponse::InternalError(
            "Failed to compute bounding geometry for the specified nodes.", req.path)};
          return;
        }
        RenderingManager::GetInstance()->InitializeViews(
          geometry, RenderingManager::REQUEST_UPDATE_ALL, true);
      });
    }
    catch (const mitk::Exception& e)
    {
      const auto error = ErrorResponse::RenderingError(
        std::string("Rendering operation failed: ") + e.what(), req.path);
      this->SendErrorResponse(res, 422, error);
      return;
    }
    catch (const std::exception& e)
    {
      const auto error = ErrorResponse::InternalError(
        std::string("Unexpected error during rendering: ") + e.what(), req.path);
      this->SendErrorResponse(res, 500, error);
      return;
    }

    if (dispatchError.has_value())
    {
      this->SendErrorResponse(res, dispatchError->first, dispatchError->second);
      return;
    }
    res.status = 204;
  }
  else
  {
    // Global reinit: fit all views to the bounding box of all visible data.
    const auto dataStorage = m_Bridge.GetDataStorage();

    try
    {
      this->Dispatch([dataStorage]() {
        RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
      });
      res.status = 204;
    }
    catch (const mitk::Exception& e)
    {
      const auto error = ErrorResponse::RenderingError(
        std::string("Rendering operation failed: ") + e.what(), req.path);
      this->SendErrorResponse(res, 422, error);
    }
    catch (const std::exception& e)
    {
      const auto error = ErrorResponse::InternalError(
        std::string("Unexpected error during rendering: ") + e.what(), req.path);
      this->SendErrorResponse(res, 500, error);
    }
  }
}

void RenderingController::HandleGET_selectedPosition(const httplib::Request& req, httplib::Response& res)
{
  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasPositionGetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  // Read position via the bridge (dispatches to UI thread internally).
  Point3D pos;
  pos.Fill(0.0);
  try
  {
    pos = m_RenderWindowBridge->GetSelectedPosition();
  }
  catch (const std::exception& e)
  {
    const auto error = ErrorResponse::InternalError(
      std::string("Failed to read crosshair position: ") + e.what(), req.path);
    this->SendErrorResponse(res, 500, error);
    return;
  }

  // Read bounds from the TimeNavigationController's input world time geometry
  // (i.e. the geometry established by reinit). Dispatched separately.
  bool hasBounds = false;
  double worldMin[3] = {0.0, 0.0, 0.0};
  double worldMax[3] = {0.0, 0.0, 0.0};

  try
  {
    this->Dispatch([&hasBounds, &worldMin, &worldMax]()
    {
      auto* const tnc = RenderingManager::GetInstance()->GetTimeNavigationController();
      if (tnc == nullptr)
        return;

      const auto tg = tnc->GetInputWorldTimeGeometry();
      if (tg == nullptr)
        return;

      const auto baseGeom = tg->GetGeometryForTimeStep(tnc->GetSelectedTimeStep());
      if (baseGeom.IsNull())
        return;

      for (int i = 0; i < 3; ++i)
      {
        worldMin[i] = std::numeric_limits<double>::max();
        worldMax[i] = std::numeric_limits<double>::lowest();
      }
      for (int cornerId = 0; cornerId < 8; ++cornerId)
      {
        const auto corner = baseGeom->GetCornerPoint(cornerId);
        for (int i = 0; i < 3; ++i)
        {
          worldMin[i] = std::min(worldMin[i], corner[i]);
          worldMax[i] = std::max(worldMax[i], corner[i]);
        }
      }
      hasBounds = true;
    });
  }
  catch (const std::exception& e)
  {
    const auto error = ErrorResponse::InternalError(
      std::string("Failed to read scene bounds: ") + e.what(), req.path);
    this->SendErrorResponse(res, 500, error);
    return;
  }

  nlohmann::json response;
  response["position"] = {pos[0], pos[1], pos[2]};

  if (hasBounds)
  {
    response["bounds"]["min"] = {worldMin[0], worldMin[1], worldMin[2]};
    response["bounds"]["max"] = {worldMax[0], worldMax[1], worldMax[2]};
  }
  else
  {
    response["bounds"]["min"] = nullptr;
    response["bounds"]["max"] = nullptr;
  }

  res.status = 200;
  res.set_content(response.dump(), "application/json");
}

void RenderingController::HandlePUT_selectedPosition(const httplib::Request& req, httplib::Response& res)
{
  if (req.body.empty())
  {
    const auto error = ErrorResponse::InvalidRequest("Request body is required.", req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  nlohmann::json body;
  try
  {
    body = nlohmann::json::parse(req.body);
  }
  catch (const nlohmann::json::exception&)
  {
    const auto error = ErrorResponse::InvalidRequest("Invalid JSON body.", req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (!body.contains("position"))
  {
    const auto error = ErrorResponse::InvalidRequest("'position' field is required.", req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  const auto& posJson = body["position"];
  if (!posJson.is_array() || posJson.size() != 3)
  {
    const auto error = ErrorResponse::InvalidRequest(
      "'position' must be an array of exactly 3 numbers.", req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  for (const auto& v : posJson)
  {
    if (!v.is_number())
    {
      const auto error = ErrorResponse::InvalidRequest(
        "'position' must be an array of exactly 3 numbers.", req.path);
      this->SendErrorResponse(res, 400, error);
      return;
    }
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasPositionSetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  Point3D newPos;
  newPos[0] = posJson[0].get<double>();
  newPos[1] = posJson[1].get<double>();
  newPos[2] = posJson[2].get<double>();

  try
  {
    m_RenderWindowBridge->SetSelectedPosition(newPos);
    res.status = 204;
  }
  catch (const std::exception& e)
  {
    const auto error = ErrorResponse::InternalError(
      std::string("Failed to set crosshair position: ") + e.what(), req.path);
    this->SendErrorResponse(res, 500, error);
  }
}

void RenderingController::HandleGET_selectedTime(const httplib::Request& req, httplib::Response& res)
{
  int timestep = 0;
  double timepointMs = 0.0;
  double minTimepointMs = 0.0;
  double maxTimepointMs = 0.0;
  int steps = 0;

  try
  {
    this->Dispatch([&timestep, &timepointMs, &minTimepointMs, &maxTimepointMs, &steps]()
    {
      auto* const tnc = RenderingManager::GetInstance()->GetTimeNavigationController();
      if (tnc == nullptr)
      {
        return;
      }

      timestep = static_cast<int>(tnc->GetSelectedTimeStep());
      timepointMs = tnc->GetSelectedTimePoint();

      const auto tg = tnc->GetInputWorldTimeGeometry();
      if (nullptr != tg)
      {
        steps = static_cast<int>(tg->CountTimeSteps());
        minTimepointMs = tg->GetMinimumTimePoint();
        maxTimepointMs = tg->GetMaximumTimePoint();
      }
      else
      {
        const auto* stepper = tnc->GetStepper();
        if (stepper != nullptr)
        {
          steps = static_cast<int>(stepper->GetSteps());
        }
      }
    });
  }
  catch (const mitk::Exception& e)
  {
    const auto error = ErrorResponse::InternalError(
      std::string("Failed to read time navigation state: ") + e.what(), req.path);
    this->SendErrorResponse(res, 500, error);
    return;
  }
  catch (const std::exception& e)
  {
    const auto error = ErrorResponse::InternalError(
      std::string("Failed to read time navigation state: ") + e.what(), req.path);
    this->SendErrorResponse(res, 500, error);
    return;
  }

  nlohmann::json response;
  response["timepoint_ms"] = timepointMs;
  response["timestep"] = timestep;
  response["bounds"]["min_timepoint_ms"] = minTimepointMs;
  response["bounds"]["max_timepoint_ms"] = maxTimepointMs;
  response["bounds"]["steps"] = steps;

  res.status = 200;
  res.set_content(response.dump(), "application/json");
}

void RenderingController::HandlePUT_selectedTime(const httplib::Request& req, httplib::Response& res)
{
  if (req.body.empty())
  {
    const auto error = ErrorResponse::InvalidRequest("Request body is required.", req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  nlohmann::json body;
  try
  {
    body = nlohmann::json::parse(req.body);
  }
  catch (const nlohmann::json::exception&)
  {
    const auto error = ErrorResponse::InvalidRequest("Invalid JSON body.", req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  const bool hasTimepointMs = body.contains("timepoint_ms");
  const bool hasTimestep = body.contains("timestep");

  if (hasTimepointMs && hasTimestep)
  {
    const auto error = ErrorResponse::InvalidRequest(
      "Provide exactly one of 'timepoint_ms' or 'timestep', not both.", req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (!hasTimepointMs && !hasTimestep)
  {
    const auto error = ErrorResponse::InvalidRequest(
      "Either 'timepoint_ms' (number) or 'timestep' (integer) is required.", req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (hasTimepointMs && !body["timepoint_ms"].is_number())
  {
    const auto error = ErrorResponse::InvalidRequest(
      "'timepoint_ms' must be a number.", req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (hasTimestep && !body["timestep"].is_number_integer())
  {
    const auto error = ErrorResponse::InvalidRequest(
      "'timestep' must be an integer.", req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (hasTimestep && body["timestep"].get<int>() < 0)
  {
    const auto error = ErrorResponse::InvalidRequest(
      "'timestep' must be a non-negative integer.", req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  try
  {
    if (hasTimestep)
    {
      const auto ts = static_cast<unsigned int>(body["timestep"].get<int>());
      this->Dispatch([ts]()
      {
        auto* const tnc = RenderingManager::GetInstance()->GetTimeNavigationController();
        if (tnc == nullptr)
        {
          return;
        }
        tnc->GetStepper()->SetPos(ts);
        tnc->SendTime();
      });
    }
    else
    {
      const double tp = body["timepoint_ms"].get<double>();
      this->Dispatch([tp]()
      {
        auto* const tnc = RenderingManager::GetInstance()->GetTimeNavigationController();
        if (tnc == nullptr)
        {
          return;
        }
        const auto tg = tnc->GetInputWorldTimeGeometry();
        unsigned int ts = 0;
        if (nullptr != tg)
        {
          ts = static_cast<unsigned int>(tg->TimePointToTimeStep(tp));
        }
        tnc->GetStepper()->SetPos(ts);
        tnc->SendTime();
      });
    }
    res.status = 204;
  }
  catch (const mitk::Exception& e)
  {
    const auto error = ErrorResponse::RenderingError(
      std::string("Failed to set time navigation state: ") + e.what(), req.path);
    this->SendErrorResponse(res, 422, error);
  }
  catch (const std::exception& e)
  {
    const auto error = ErrorResponse::InternalError(
      std::string("Failed to set time navigation state: ") + e.what(), req.path);
    this->SendErrorResponse(res, 500, error);
  }
}

void RenderingController::HandleGET_screenshot(const httplib::Request& req, httplib::Response& res)
{
  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasScreenshotProvider())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  // Parse format parameter (default: png).
  ScreenshotFormat format = ScreenshotFormat::Png;
  if (req.has_param("format"))
  {
    const auto formatStr = req.get_param_value("format");
    if (formatStr == "png")
    {
      format = ScreenshotFormat::Png;
    }
    else if (formatStr == "jpeg")
    {
      format = ScreenshotFormat::Jpeg;
    }
    else
    {
      const auto error = ErrorResponse::InvalidRequest(
        "Invalid format '" + formatStr + "'. Must be 'png' or 'jpeg'.", req.path);
      this->SendErrorResponse(res, 400, error);
      return;
    }
  }

  const std::string contentType = (format == ScreenshotFormat::Jpeg) ? "image/jpeg" : "image/png";

  // Parse optional width/height — both must be given together.
  const bool hasWidth = req.has_param("width");
  const bool hasHeight = req.has_param("height");

  if (hasWidth != hasHeight)
  {
    const auto error = ErrorResponse::InvalidRequest(
      "Both 'width' and 'height' must be provided together.", req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  std::optional<std::pair<int, int>> size;
  if (hasWidth)
  {
    int width = 0;
    int height = 0;
    try
    {
      width = std::stoi(req.get_param_value("width"));
      height = std::stoi(req.get_param_value("height"));
    }
    catch (const std::exception&)
    {
      const auto error = ErrorResponse::InvalidRequest(
        "'width' and 'height' must be integers.", req.path);
      this->SendErrorResponse(res, 400, error);
      return;
    }

    if (width <= 0 || height <= 0)
    {
      const auto error = ErrorResponse::InvalidRequest(
        "'width' and 'height' must be positive integers.", req.path);
      this->SendErrorResponse(res, 400, error);
      return;
    }

    size = {width, height};
  }

  // Take screenshot — the bridge dispatches to the UI thread internally.
  try
  {
    const auto imageData = m_RenderWindowBridge->TakeScreenshot(size, format);
    res.status = 200;
    res.set_content(
      reinterpret_cast<const char*>(imageData.data()),
      imageData.size(),
      contentType);
  }
  catch (const std::exception& e)
  {
    const auto error = ErrorResponse::InternalError(
      std::string("Screenshot capture failed: ") + e.what(), req.path);
    this->SendErrorResponse(res, 500, error);
  }
}

void RenderingController::Dispatch(std::function<void()> task) const
{
  auto dispatcher = m_Dispatcher.Lock();
  if (dispatcher.IsNull())
  {
    task();
  }
  else
  {
    dispatcher->Execute(std::move(task));
  }
}

void RenderingController::SendErrorResponse(httplib::Response& res, int status, const nlohmann::json& error)
{
  res.status = status;
  res.set_content(error.dump(), "application/json");
}

}
