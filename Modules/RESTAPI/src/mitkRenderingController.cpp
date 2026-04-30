/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRenderingController.h>
#include <mitkRenderWindowBridge.h>
#include <mitkErrorResponse.h>
#include <mitkDataStorage.h>
#include <mitkException.h>
#include <mitkRenderingManager.h>
#include <mitkStepper.h>
#include <mitkTimeNavigationController.h>

#include <algorithm>
#include <functional>
#include <optional>
#include <set>
#include <vector>

namespace mitk
{

RenderingController::RenderingController(DataStorageBridge& bridge)
  : m_Bridge(bridge)
{
}

void RenderingController::SetDispatcher(StorageThreadDispatcherBase* dispatcher)
{
  std::lock_guard<std::mutex> lock(m_DispatcherMutex);
  m_Dispatcher = dispatcher;
}

void RenderingController::SetRenderWindowBridge(RenderWindowBridge* bridge)
{
  m_RenderWindowBridge = bridge;
}

void RenderingController::HandlePOST_update(const httplib::Request& req, httplib::Response& res) const
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

void RenderingController::HandlePOST_reinit(const httplib::Request& req, httplib::Response& res) const
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

void RenderingController::HandleGET_selectedPosition(const httplib::Request& req, httplib::Response& res) const
{
  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasPositionGetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  // Read position and bounds atomically via the bridge (single UI-thread dispatch).
  SelectedPositionInfo posInfo;
  try
  {
    posInfo = m_RenderWindowBridge->GetSelectedPosition();
  }
  catch (const RenderWindowBridgeNoEditorException& e)
  {
    const auto error = ErrorResponse::EditorNotActive(e.what(), req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }
  catch (const std::exception& e)
  {
    const auto error = ErrorResponse::InternalError(
      std::string("Failed to read crosshair position: ") + e.what(), req.path);
    this->SendErrorResponse(res, 500, error);
    return;
  }

  nlohmann::json response;
  response["position"] = {posInfo.position[0], posInfo.position[1], posInfo.position[2]};

  if (posInfo.bounds.has_value())
  {
    const auto& b = posInfo.bounds.value();
    response["bounds"]["min"] = {b.min[0], b.min[1], b.min[2]};
    response["bounds"]["max"] = {b.max[0], b.max[1], b.max[2]};
  }
  else
  {
    response["bounds"]["min"] = nullptr;
    response["bounds"]["max"] = nullptr;
  }

  res.status = 200;
  res.set_content(response.dump(), "application/json");
}

void RenderingController::HandlePUT_selectedPosition(const httplib::Request& req, httplib::Response& res) const
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
  catch (const RenderWindowBridgeNoEditorException& e)
  {
    const auto error = ErrorResponse::EditorNotActive(e.what(), req.path);
    this->SendErrorResponse(res, 503, error);
  }
  catch (const std::exception& e)
  {
    const auto error = ErrorResponse::InternalError(
      std::string("Failed to set crosshair position: ") + e.what(), req.path);
    this->SendErrorResponse(res, 500, error);
  }
}

void RenderingController::HandleGET_selectedTime(const httplib::Request& req, httplib::Response& res) const
{
  TimeStepType timestep = 0;
  double timepointMs = 0.0;
  double minTimepointMs = 0.0;
  double maxTimepointMs = 0.0;
  TimeStepType steps = 0;

  bool tncNull = false;
  try
  {
    this->Dispatch([&timestep, &timepointMs, &minTimepointMs, &maxTimepointMs, &steps, &tncNull]()
    {
      auto* const tnc = RenderingManager::GetInstance()->GetTimeNavigationController();
      if (tnc == nullptr)
      {
        tncNull = true;
        return;
      }

      timestep = tnc->GetSelectedTimeStep();
      timepointMs = tnc->GetSelectedTimePoint();

      const auto tg = tnc->GetInputWorldTimeGeometry();
      if (nullptr != tg)
      {
        steps = tg->CountTimeSteps();
        minTimepointMs = tg->GetMinimumTimePoint();
        maxTimepointMs = tg->GetMaximumTimePoint();
      }
      else
      {
        const auto* stepper = tnc->GetStepper();
        if (stepper != nullptr)
        {
          steps = stepper->GetSteps();
        }
      }
    });
  }
  catch (const std::exception& e)
  {
    const auto error = ErrorResponse::InternalError(
      std::string("Failed to read time navigation state: ") + e.what(), req.path);
    this->SendErrorResponse(res, 500, error);
    return;
  }

  if (tncNull)
  {
    const auto error = ErrorResponse::TimeNavigationNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
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

void RenderingController::HandlePUT_selectedTime(const httplib::Request& req, httplib::Response& res) const
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
    std::function<TimeStepType(TimeNavigationController*)> computeStep;
    if (hasTimestep)
    {
      const auto ts = static_cast<TimeStepType>(body["timestep"].get<int>());
      computeStep = [ts](TimeNavigationController*) { return ts; };
    }
    else
    {
      const double tp = body["timepoint_ms"].get<double>();
      computeStep = [tp](TimeNavigationController* tnc)
      {
        const auto tg = tnc->GetInputWorldTimeGeometry();
        TimeStepType ts = 0;
        if (nullptr != tg)
        {
          ts = tg->TimePointToTimeStep(tp);
        }
        return ts;
      };
    }

    bool tncNull = false;
    bool stepperNull = false;
    this->Dispatch([&computeStep, &tncNull, &stepperNull]()
    {
      auto* const tnc = RenderingManager::GetInstance()->GetTimeNavigationController();
      if (tnc == nullptr)
      {
        tncNull = true;
        return;
      }
      auto* const stepper = tnc->GetStepper();
      if (stepper == nullptr)
      {
        stepperNull = true;
        return;
      }
      stepper->SetPos(static_cast<unsigned int>(computeStep(tnc)));
      tnc->SendTime();
    });

    if (tncNull)
    {
      const auto error = ErrorResponse::TimeNavigationNotAvailable(req.path);
      this->SendErrorResponse(res, 503, error);
      return;
    }
    if (stepperNull)
    {
      const auto error = ErrorResponse::TimeStepperNotAvailable(req.path);
      this->SendErrorResponse(res, 500, error);
      return;
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

namespace
{
  nlohmann::json EditorInfoToJson(const mitk::EditorInfo& info, bool includeWindowList)
  {
    nlohmann::json j;
    j["alias"] = info.alias;
    j["plugin_id"] = info.pluginId;
    j["active"] = info.active;
    if (includeWindowList)
    {
      j["windows"] = nlohmann::json::array();
      for (const auto& n : info.windowNames)
        j["windows"].push_back(n);
    }
    return j;
  }

  bool IsValidStandardViewName(const std::string& v)
  {
    // Restricted to the six values CameraController::StandardView exposes.
    return v == "anterior" || v == "posterior" ||
           v == "left"     || v == "right"     ||
           v == "cranial"  || v == "caudal";
  }

  // Read a JSON array of exactly 3 numbers into `out`. Returns nullopt on OK,
  // or a diagnostic fragment (appended by the caller to the field name) on failure.
  std::optional<std::string> ReadPoint3D(const nlohmann::json& arr, mitk::Point3D& out)
  {
    if (!arr.is_array())
      return "must be an array of 3 numbers";
    if (arr.size() != 3)
      return "must have exactly 3 elements, got " + std::to_string(arr.size());
    for (std::size_t i = 0; i < 3; ++i)
    {
      if (!arr[i].is_number())
        return "element " + std::to_string(i) + " is not a number";
      out[i] = arr[i].get<double>();
    }
    return std::nullopt;
  }

  std::optional<std::string> ReadVector3D(const nlohmann::json& arr, mitk::Vector3D& out)
  {
    if (!arr.is_array())
      return "must be an array of 3 numbers";
    if (arr.size() != 3)
      return "must have exactly 3 elements, got " + std::to_string(arr.size());
    for (std::size_t i = 0; i < 3; ++i)
    {
      if (!arr[i].is_number())
        return "element " + std::to_string(i) + " is not a number";
      out[i] = arr[i].get<double>();
    }
    return std::nullopt;
  }

  /**
   * \brief Parse and validate a camera patch body for a given window kind.
   *
   * On success, fills `patch` and returns nullopt. On failure, returns the
   * HTTP 400 error payload (the caller already knows the status).
   *
   * Rules:
   * - Unknown top-level fields → 400.
   * - position/focal_point/view_up: array of 3 numbers if present.
   * - parallel_scale: number > 0, only for 2D windows.
   * - perspective_angle: number in (0, 180), only for 3D windows.
   * - standard_view: one of anterior/posterior/left/right/cranial/caudal.
   * - Empty body (no recognised field) → 400.
   */
  std::optional<std::string> ParseCameraPatch(
    const nlohmann::json& body, bool is3d, mitk::CameraPatch& patch)
  {
    static const std::set<std::string> knownFields = {
      "position", "focal_point", "view_up",
      "parallel_scale", "perspective_angle", "standard_view"
    };

    if (!body.is_object())
      return "Request body must be a JSON object.";

    for (auto it = body.begin(); it != body.end(); ++it)
    {
      if (!knownFields.count(it.key()))
        return "Unknown field '" + it.key() + "'.";
    }

    if (body.contains("position"))
    {
      mitk::Point3D p;
      if (const auto err = ReadPoint3D(body["position"], p))
        return "'position' " + *err + ".";
      patch.position = p;
    }
    if (body.contains("focal_point"))
    {
      mitk::Point3D p;
      if (const auto err = ReadPoint3D(body["focal_point"], p))
        return "'focal_point' " + *err + ".";
      patch.focalPoint = p;
    }
    if (body.contains("view_up"))
    {
      mitk::Vector3D v;
      if (const auto err = ReadVector3D(body["view_up"], v))
        return "'view_up' " + *err + ".";
      patch.viewUp = v;
    }
    if (body.contains("parallel_scale"))
    {
      if (is3d)
        return "'parallel_scale' is not applicable to the 3D window.";
      if (!body["parallel_scale"].is_number())
        return "'parallel_scale' must be a positive number.";
      const double s = body["parallel_scale"].get<double>();
      if (!(s > 0.0))
        return "'parallel_scale' must be a positive number.";
      patch.parallelScale = s;
    }
    if (body.contains("perspective_angle"))
    {
      if (!is3d)
        return "'perspective_angle' is only applicable to the 3D window.";
      if (!body["perspective_angle"].is_number())
        return "'perspective_angle' must be a number in (0, 180).";
      const double a = body["perspective_angle"].get<double>();
      if (!(a > 0.0 && a < 180.0))
        return "'perspective_angle' must be a number in (0, 180).";
      patch.perspectiveAngle = a;
    }
    if (body.contains("standard_view"))
    {
      if (!body["standard_view"].is_string())
        return "'standard_view' must be a string.";
      const auto v = body["standard_view"].get<std::string>();
      if (!IsValidStandardViewName(v))
        return "'standard_view' has unknown value '" + v + "'. "
               "Allowed: anterior, posterior, left, right, cranial, caudal.";
      patch.standardView = v;
    }

    // 'standard_view' programs the CameraController, while explicit pose fields
    // bypass it and write the raw vtkCamera. Combining them leaves the
    // controller's internal "standard view" memo inconsistent with the actual
    // pose, so we reject the combination outright. Scalar fields
    // (parallel_scale, perspective_angle) do not move the camera and remain
    // compatible with standard_view.
    if (patch.standardView && (patch.position || patch.focalPoint || patch.viewUp))
    {
      return "'standard_view' cannot be combined with 'position', 'focal_point', "
             "or 'view_up'. Send either a standard view or an explicit pose.";
    }

    if (!patch.position && !patch.focalPoint && !patch.viewUp &&
        !patch.parallelScale && !patch.perspectiveAngle && !patch.standardView)
    {
      return "Request body must set at least one camera field.";
    }

    return std::nullopt;
  }

  constexpr const char* ContentTypeFor(mitk::ScreenshotFormat f)
  {
    return (f == mitk::ScreenshotFormat::Jpeg) ? "image/jpeg" : "image/png";
  }

  /**
   * \brief Resolved query parameters for the screenshot endpoints.
   *
   * Shared between /rendering/screenshot, /rendering/editors/stdmulti/screenshot,
   * and /rendering/editors/stdmulti/windows/{name}/screenshot — the contract
   * is intentionally identical.
   */
  struct ScreenshotQueryParams
  {
    mitk::ScreenshotFormat format = mitk::ScreenshotFormat::Png;
    std::optional<std::pair<int, int>> size;
  };

  /**
   * \brief Thrown by ParseScreenshotQueryParams on invalid query parameters.
   *
   * Carries the human-readable detail string that the handler turns into a
   * 400 InvalidRequest response. Internal to this translation unit.
   */
  struct InvalidScreenshotRequest
  {
    std::string detail;
  };

  /**
   * \brief Parse `format` / `width` / `height` query parameters.
   *
   * \throws InvalidScreenshotRequest if any parameter is malformed.
   */
  ScreenshotQueryParams ParseScreenshotQueryParams(const httplib::Request& req)
  {
    ScreenshotQueryParams p;

    if (req.has_param("format"))
    {
      const auto formatStr = req.get_param_value("format");
      if (formatStr == "png")
        p.format = mitk::ScreenshotFormat::Png;
      else if (formatStr == "jpeg")
        p.format = mitk::ScreenshotFormat::Jpeg;
      else
        throw InvalidScreenshotRequest{
          "Invalid format '" + formatStr + "'. Must be 'png' or 'jpeg'."};
    }

    const bool hasWidth = req.has_param("width");
    const bool hasHeight = req.has_param("height");
    if (hasWidth != hasHeight)
      throw InvalidScreenshotRequest{
        "Both 'width' and 'height' must be provided together."};

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
        throw InvalidScreenshotRequest{"'width' and 'height' must be integers."};
      }

      if (width <= 0 || height <= 0)
        throw InvalidScreenshotRequest{
          "'width' and 'height' must be positive integers."};

      static constexpr int maxDimension = 8192;
      if (width > maxDimension || height > maxDimension)
        throw InvalidScreenshotRequest{
          "'width' and 'height' must not exceed " + std::to_string(maxDimension) + "."};

      p.size = {width, height};
    }

    return p;
  }

  nlohmann::json SliceStateToJson(const mitk::SliceState& s)
  {
    nlohmann::json j;
    j["step"] = s.step;
    j["position"] = {s.position[0], s.position[1], s.position[2]};
    j["bounds"]["steps"] = s.bounds.steps;
    if (s.bounds.hasPositions)
    {
      j["bounds"]["min_position"] = {s.bounds.minPosition[0], s.bounds.minPosition[1], s.bounds.minPosition[2]};
      j["bounds"]["max_position"] = {s.bounds.maxPosition[0], s.bounds.maxPosition[1], s.bounds.maxPosition[2]};
    }
    else
    {
      j["bounds"]["min_position"] = nullptr;
      j["bounds"]["max_position"] = nullptr;
    }
    return j;
  }

  /**
   * \brief Parse a selected-slice PUT body. Only `{"step": N}` is accepted
   *        for StdMulti. Presence of `position` → hint to use selected-position.
   *
   * On success, fills `step`. On failure, returns the 400 error detail string.
   */
  std::optional<std::string> ParseSliceStepBody(const nlohmann::json& body, unsigned int& step)
  {
    if (!body.is_object())
      return "Request body must be a JSON object.";

    // `position` is reserved for /rendering/selected-position — explicit hint.
    if (body.contains("position"))
    {
      return "'position' is not accepted on StdMulti selected-slice. "
             "Use PUT /rendering/selected-position to move by world coordinates.";
    }

    for (auto it = body.begin(); it != body.end(); ++it)
    {
      if (it.key() != "step")
        return "Unknown field '" + it.key() + "'.";
    }

    if (!body.contains("step"))
      return "'step' field is required.";

    const auto& s = body["step"];
    if (!s.is_number_integer())
      return "'step' must be a non-negative integer.";

    const auto raw = s.get<long long>();
    if (raw < 0)
      return "'step' must be a non-negative integer.";

    step = static_cast<unsigned int>(raw);
    return std::nullopt;
  }

  nlohmann::json CameraStateToJson(const mitk::CameraState& s)
  {
    nlohmann::json j;
    j["position"]    = {s.position[0],    s.position[1],    s.position[2]};
    j["focal_point"] = {s.focalPoint[0],  s.focalPoint[1],  s.focalPoint[2]};
    j["view_up"]     = {s.viewUp[0],      s.viewUp[1],      s.viewUp[2]};
    if (s.parallelScale.has_value())    j["parallel_scale"]    = *s.parallelScale;
    if (s.perspectiveAngle.has_value()) j["perspective_angle"] = *s.perspectiveAngle;
    return j;
  }
}

void RenderingController::HandleGET_editors(const httplib::Request& req, httplib::Response& res) const
{
  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasEditorListProvider())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  std::vector<EditorInfo> editors;
  try
  {
    editors = m_RenderWindowBridge->ListEditors();
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
    return;
  }

  nlohmann::json arr = nlohmann::json::array();
  for (const auto& ed : editors)
    arr.push_back(EditorInfoToJson(ed, /*includeWindowList=*/false));

  res.status = 200;
  res.set_content(arr.dump(), "application/json");
}

void RenderingController::HandleGET_stdmultiInfo(const httplib::Request& req, httplib::Response& res) const
{
  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasEditorListProvider())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  std::vector<EditorInfo> editors;
  try
  {
    editors = m_RenderWindowBridge->ListEditors();
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
    return;
  }

  for (const auto& ed : editors)
  {
    if (ed.alias == "stdmulti")
    {
      if (!ed.active)
      {
        const auto error = ErrorResponse::EditorNotActive(
          "StdMultiWidgetEditor is not open", req.path);
        this->SendErrorResponse(res, 503, error);
        return;
      }
      const auto j = EditorInfoToJson(ed, /*includeWindowList=*/true);
      res.status = 200;
      res.set_content(j.dump(), "application/json");
      return;
    }
  }

  // The alias list is authoritative; stdmulti missing means the provider is broken.
  const auto error = ErrorResponse::InternalError(
    "Editor list does not contain 'stdmulti'.", req.path);
  this->SendErrorResponse(res, 500, error);
}

void RenderingController::HandleGET_stdmultiWindows(const httplib::Request& req, httplib::Response& res) const
{
  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasStdMultiWindowListProvider())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  std::vector<WindowInfo> windows;
  try
  {
    windows = m_RenderWindowBridge->ListStdMultiWindows();
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
    return;
  }

  nlohmann::json arr = nlohmann::json::array();
  for (const auto& w : windows)
  {
    nlohmann::json wj;
    wj["name"] = w.name;
    wj["kind"] = WindowKindToString(w.kind);
    arr.push_back(wj);
  }

  res.status = 200;
  res.set_content(arr.dump(), "application/json");
}

void RenderingController::HandleGET_stdmultiWindow(const httplib::Request& req, httplib::Response& res) const
{
  const auto name = ReadRequiredPathParam(req, "name");

  if (!IsValidStdMultiWindowName(name))
  {
    const auto error = ErrorResponse::RenderWindowNotFound(name, req.path);
    this->SendErrorResponse(res, 404, error);
    return;
  }

  // Compose summary locally; the bridge already exposes enough state via the
  // windows-list provider, and no per-window "summary" callback is needed.
  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasStdMultiWindowListProvider())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  std::vector<WindowInfo> windows;
  try
  {
    windows = m_RenderWindowBridge->ListStdMultiWindows();
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
    return;
  }

  // The controller-side name validation already accepts only canonical names,
  // so an empty window list at this point means the editor reports no windows
  // — that is an editor state we also surface as RENDER_WINDOW_NOT_FOUND.
  const bool present = std::any_of(windows.begin(), windows.end(),
    [&](const WindowInfo& w) { return w.name == name; });
  if (!present)
  {
    const auto error = ErrorResponse::RenderWindowNotFound(name, req.path);
    this->SendErrorResponse(res, 404, error);
    return;
  }

  const bool is3d = IsStdMulti3dWindow(name);

  nlohmann::json j;
  j["name"] = name;
  j["kind"] = is3d ? "3d" : "2d";
  j["has_camera"] = true;
  j["has_selected_slice"] = !is3d;

  res.status = 200;
  res.set_content(j.dump(), "application/json");
}

void RenderingController::HandleGET_stdmultiCamera(const httplib::Request& req, httplib::Response& res) const
{
  const auto name = ReadRequiredPathParam(req, "name");

  if (!IsValidStdMultiWindowName(name))
  {
    const auto error = ErrorResponse::RenderWindowNotFound(name, req.path);
    this->SendErrorResponse(res, 404, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasStdMultiCameraGetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  CameraState state;
  try
  {
    state = m_RenderWindowBridge->GetStdMultiCamera(name);
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
    return;
  }

  const auto j = CameraStateToJson(state);
  res.status = 200;
  res.set_content(j.dump(), "application/json");
}

void RenderingController::HandlePUT_stdmultiCamera(const httplib::Request& req, httplib::Response& res) const
{
  const auto name = ReadRequiredPathParam(req, "name");

  if (!IsValidStdMultiWindowName(name))
  {
    const auto error = ErrorResponse::RenderWindowNotFound(name, req.path);
    this->SendErrorResponse(res, 404, error);
    return;
  }

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

  CameraPatch patch;
  if (const auto err = ParseCameraPatch(body, IsStdMulti3dWindow(name), patch))
  {
    const auto error = ErrorResponse::InvalidRequest(*err, req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasStdMultiCameraSetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  try
  {
    m_RenderWindowBridge->SetStdMultiCamera(name, patch);
    res.status = 204;
  }
  catch (const mitk::Exception& e)
  {
    const auto error = ErrorResponse::RenderingError(
      std::string("Camera update failed: ") + e.what(), req.path);
    this->SendErrorResponse(res, 422, error);
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
  }
}

void RenderingController::HandleGET_stdmultiSelectedSlice(const httplib::Request& req, httplib::Response& res) const
{
  const auto name = ReadRequiredPathParam(req, "name");

  if (!IsValidStdMultiWindowName(name))
  {
    const auto error = ErrorResponse::RenderWindowNotFound(name, req.path);
    this->SendErrorResponse(res, 404, error);
    return;
  }

  if (IsStdMulti3dWindow(name))
  {
    const auto error = ErrorResponse::UnsupportedOperation(
      "selected-slice is not applicable to the 3D window.", req.path);
    this->SendErrorResponse(res, 404, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasStdMultiSelectedSliceGetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  SliceState state;
  try
  {
    state = m_RenderWindowBridge->GetStdMultiSelectedSlice(name);
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
    return;
  }

  const auto j = SliceStateToJson(state);
  res.status = 200;
  res.set_content(j.dump(), "application/json");
}

void RenderingController::HandlePUT_stdmultiSelectedSlice(const httplib::Request& req, httplib::Response& res) const
{
  const auto name = ReadRequiredPathParam(req, "name");

  if (!IsValidStdMultiWindowName(name))
  {
    const auto error = ErrorResponse::RenderWindowNotFound(name, req.path);
    this->SendErrorResponse(res, 404, error);
    return;
  }

  if (IsStdMulti3dWindow(name))
  {
    const auto error = ErrorResponse::UnsupportedOperation(
      "selected-slice is not applicable to the 3D window.", req.path);
    this->SendErrorResponse(res, 404, error);
    return;
  }

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

  unsigned int step = 0;
  if (const auto err = ParseSliceStepBody(body, step))
  {
    const auto error = ErrorResponse::InvalidRequest(*err, req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasStdMultiSelectedSliceStepSetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  try
  {
    m_RenderWindowBridge->SetStdMultiSelectedSliceStep(name, step);
    res.status = 204;
  }
  catch (const mitk::Exception& e)
  {
    const auto error = ErrorResponse::RenderingError(
      std::string("Slice update failed: ") + e.what(), req.path);
    this->SendErrorResponse(res, 422, error);
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
  }
}

void RenderingController::HandleGET_screenshot(const httplib::Request& req, httplib::Response& res) const
{
  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasScreenshotProvider())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  ScreenshotQueryParams params;
  try
  {
    params = ParseScreenshotQueryParams(req);
  }
  catch (const InvalidScreenshotRequest& e)
  {
    this->SendErrorResponse(res, 400, ErrorResponse::InvalidRequest(e.detail, req.path));
    return;
  }

  try
  {
    const auto imageData = m_RenderWindowBridge->TakeScreenshot(params.size, params.format);
    res.status = 200;
    res.set_content(
      reinterpret_cast<const char*>(imageData.data()),
      imageData.size(),
      ContentTypeFor(params.format));
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
  }
}

void RenderingController::HandleGET_stdmultiScreenshot(const httplib::Request& req, httplib::Response& res) const
{
  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasStdMultiEditorScreenshotProvider())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  ScreenshotQueryParams params;
  try
  {
    params = ParseScreenshotQueryParams(req);
  }
  catch (const InvalidScreenshotRequest& e)
  {
    this->SendErrorResponse(res, 400, ErrorResponse::InvalidRequest(e.detail, req.path));
    return;
  }

  try
  {
    const auto imageData = m_RenderWindowBridge->TakeStdMultiEditorScreenshot(params.size, params.format);
    res.status = 200;
    res.set_content(
      reinterpret_cast<const char*>(imageData.data()),
      imageData.size(),
      ContentTypeFor(params.format));
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
  }
}

void RenderingController::HandleGET_stdmultiWindowScreenshot(const httplib::Request& req, httplib::Response& res) const
{
  const auto name = ReadRequiredPathParam(req, "name");

  if (!IsValidStdMultiWindowName(name))
  {
    const auto error = ErrorResponse::RenderWindowNotFound(name, req.path);
    this->SendErrorResponse(res, 404, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasStdMultiWindowScreenshotProvider())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  ScreenshotQueryParams params;
  try
  {
    params = ParseScreenshotQueryParams(req);
  }
  catch (const InvalidScreenshotRequest& e)
  {
    this->SendErrorResponse(res, 400, ErrorResponse::InvalidRequest(e.detail, req.path));
    return;
  }

  try
  {
    const auto imageData = m_RenderWindowBridge->TakeStdMultiWindowScreenshot(name, params.size, params.format);
    res.status = 200;
    res.set_content(
      reinterpret_cast<const char*>(imageData.data()),
      imageData.size(),
      ContentTypeFor(params.format));
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
  }
}

void RenderingController::Dispatch(std::function<void()> task) const
{
  StorageThreadDispatcherBase::Pointer dispatcher;
  {
    std::lock_guard<std::mutex> lock(m_DispatcherMutex);
    dispatcher = m_Dispatcher.Lock();
  }
  if (dispatcher.IsNull())
  {
    task();
  }
  else
  {
    dispatcher->Execute(std::move(task));
  }
}

void RenderingController::SendErrorResponse(httplib::Response& res, int status, const nlohmann::json& error) const
{
  res.status = status;
  res.set_content(error.dump(), "application/json");
}

bool RenderingController::IsValidStdMultiWindowName(const std::string& name)
{
  return name == "axial" || name == "sagittal" || name == "coronal" || name == "3d";
}

bool RenderingController::IsStdMulti3dWindow(const std::string& name)
{
  return name == "3d";
}

std::string RenderingController::ReadRequiredPathParam(const httplib::Request& req,
                                                      const std::string& key)
{
  const auto it = req.path_params.find(key);
  return (it != req.path_params.end()) ? it->second : std::string();
}

std::pair<int, nlohmann::json> RenderingController::MapBridgeException(
  const std::exception& e, const std::string& instance)
{
  if (const auto* ne = dynamic_cast<const RenderWindowBridgeNoEditorException*>(&e))
    return {503, ErrorResponse::EditorNotActive(ne->what(), instance)};

  if (const auto* uw = dynamic_cast<const RenderWindowBridgeUnknownWindowException*>(&e))
    return {404, ErrorResponse::RenderWindowNotFound(uw->what(), instance)};

  if (const auto* us = dynamic_cast<const RenderWindowBridgeUnsupportedOperationException*>(&e))
    return {404, ErrorResponse::UnsupportedOperation(us->what(), instance)};

  return {500, ErrorResponse::InternalError(e.what(), instance)};
}

}
