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
#include <regex>
#include <set>
#include <sstream>
#include <string_view>
#include <vector>

namespace mitk
{

namespace
{
  /**
   * \brief JSON shape of GET /rendering/selected-position (and the per-cell
   *        MxN selected-position resource).
   *
   * Bounds: when no geometry is loaded, `bounds.min_position` /
   * `bounds.max_position` serialize as `null` rather than being omitted, so
   * clients can rely on the keys always being present. Field names mirror
   * the slice navigator's `bounds.min_position` / `bounds.max_position`
   * convention.
   */
  nlohmann::json SerializeSelectedPositionInfoToJson(const mitk::SelectedPositionInfo& info)
  {
    nlohmann::json j;
    j["position"] = {info.position[0], info.position[1], info.position[2]};

    if (info.bounds.has_value())
    {
      const auto& b = info.bounds.value();
      j["bounds"]["min_position"] = {b.minPosition[0], b.minPosition[1], b.minPosition[2]};
      j["bounds"]["max_position"] = {b.maxPosition[0], b.maxPosition[1], b.maxPosition[2]};
    }
    else
    {
      j["bounds"]["min_position"] = nullptr;
      j["bounds"]["max_position"] = nullptr;
    }
    return j;
  }

  /**
   * \brief Parse `{"position": [x,y,z]}` from a PUT body.
   *
   * Shared between PUT /rendering/selected-position and PUT
   * /rendering/editors/mxn/windows/{id}/selected-position: same field
   * name, same shape, same validation rules.
   *
   * \returns nullopt on success; an error fragment otherwise.
   */
  std::optional<std::string> ParsePositionFromBody(const nlohmann::json& body, mitk::Point3D& out)
  {
    if (!body.contains("position"))
      return "'position' field is required.";

    const auto& posJson = body["position"];
    if (!posJson.is_array() || posJson.size() != 3)
      return "'position' must be an array of exactly 3 numbers.";

    for (const auto& v : posJson)
      if (!v.is_number())
        return "'position' must be an array of exactly 3 numbers.";

    out[0] = posJson[0].get<double>();
    out[1] = posJson[1].get<double>();
    out[2] = posJson[2].get<double>();
    return std::nullopt;
  }
}

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

  res.status = 200;
  res.set_content(SerializeSelectedPositionInfoToJson(posInfo).dump(), "application/json");
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

  Point3D newPos;
  if (const auto err = ParsePositionFromBody(body, newPos))
  {
    const auto error = ErrorResponse::InvalidRequest(*err, req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasPositionSetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

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
    // Filled by the dispatched lambda when the resolved step is outside
    // [0, steps). Surface as 422 so clients can distinguish an out-of-range
    // input from internal failure -- mirrors the Stepper's actual contract
    // (it would silently clamp otherwise, hiding a likely client bug).
    bool stepOutOfRange = false;
    TimeStepType resolvedStep = 0;
    unsigned int totalSteps = 0;
    this->Dispatch([&computeStep, &tncNull, &stepperNull,
                    &stepOutOfRange, &resolvedStep, &totalSteps]()
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
      const auto step = computeStep(tnc);
      const auto steps = stepper->GetSteps();
      if (steps == 0 || step >= static_cast<TimeStepType>(steps))
      {
        stepOutOfRange = true;
        resolvedStep = step;
        totalSteps = steps;
        return;
      }
      stepper->SetPos(static_cast<unsigned int>(step));
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
    if (stepOutOfRange)
    {
      std::ostringstream oss;
      oss << "Resolved time step " << resolvedStep << " is out of range [0, "
          << totalSteps << ").";
      const auto error = ErrorResponse::RenderingError(oss.str(), req.path);
      this->SendErrorResponse(res, 422, error);
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
      for (const auto& id : info.windowIds)
        j["windows"].push_back(id);
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
   * - Unknown top-level fields -> 400.
   * - position/focal_point/view_up: array of 3 numbers if present.
   * - parallel_scale: number > 0, only for 2D windows.
   * - perspective_angle: number in (0, 180), only for 3D windows.
   * - standard_view: one of anterior/posterior/left/right/cranial/caudal.
   * - Empty body (no recognised field) -> 400.
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
   * and /rendering/editors/stdmulti/windows/{id}/screenshot -- the contract
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

  // SerializeSelectedPositionInfoToJson and ParsePositionFromBody live in the
  // earlier anonymous namespace (top of this file) so they are visible to the
  // global selected-position handlers (HandleGET_selectedPosition /
  // HandlePUT_selectedPosition).

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
   * \brief Parse `{"step": N}` from a slice-PUT body.
   *
   * \param body            Pre-parsed JSON body.
   * \param resourceContext Resource label embedded into the rejection when
   *                        `position` is present (e.g. "StdMulti
   *                        selected-slice", "MxN selected-slice"). Identifies
   *                        which slice resource produced the error.
   * \param positionHint    Message appended to the rejection when `position`
   *                        is present in the body. Differs between StdMulti
   *                        (always coupled; redirects to the global
   *                        selected-position resource) and MxN (has its own
   *                        per-cell selected-position resource).
   * \param step            Out-parameter for the parsed step.
   * \returns nullopt on success; an error fragment otherwise.
   */
  std::optional<std::string> ParseSliceStepBody(const nlohmann::json& body,
                                                std::string_view resourceContext,
                                                const std::string& positionHint,
                                                unsigned int& step)
  {
    if (!body.is_object())
      return "Request body must be a JSON object.";

    if (body.contains("position"))
      return std::string("'position' is not accepted on ") + std::string(resourceContext)
        + ". " + positionHint;

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
    wj["id"] = w.id;
    wj["kind"] = WindowKindToString(w.kind);
    if (w.viewDirection.has_value())
      wj["view_direction"] = AnatomicalPlaneToV2String(*w.viewDirection);
    arr.push_back(wj);
  }

  res.status = 200;
  res.set_content(arr.dump(), "application/json");
}

void RenderingController::HandleGET_stdmultiWindow(const httplib::Request& req, httplib::Response& res) const
{
  const auto id = ReadRequiredPathParam(req, "id");

  if (!IsValidStdMultiWindowId(id))
  {
    const auto error = ErrorResponse::RenderWindowNotFound(id, req.path);
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

  // The controller-side id validation already accepts only canonical ids,
  // so an empty window list at this point means the editor reports no windows
  // -- that is an editor state we also surface as RENDER_WINDOW_NOT_FOUND.
  const auto matched = std::find_if(windows.begin(), windows.end(),
    [&](const WindowInfo& w) { return w.id == id; });
  if (matched == windows.end())
  {
    const auto error = ErrorResponse::RenderWindowNotFound(id, req.path);
    this->SendErrorResponse(res, 404, error);
    return;
  }

  const bool is3d = IsStdMulti3dWindow(id);

  nlohmann::json j;
  j["id"] = id;
  j["kind"] = is3d ? "3d" : "2d";
  if (matched->viewDirection.has_value())
    j["view_direction"] = AnatomicalPlaneToV2String(*matched->viewDirection);
  j["has_camera"] = true;
  j["has_selected_slice"] = !is3d;

  res.status = 200;
  res.set_content(j.dump(), "application/json");
}

void RenderingController::HandleGET_stdmultiCamera(const httplib::Request& req, httplib::Response& res) const
{
  const auto id = ReadRequiredPathParam(req, "id");

  if (!IsValidStdMultiWindowId(id))
  {
    const auto error = ErrorResponse::RenderWindowNotFound(id, req.path);
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
    state = m_RenderWindowBridge->GetStdMultiCamera(id);
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
  const auto id = ReadRequiredPathParam(req, "id");

  if (!IsValidStdMultiWindowId(id))
  {
    const auto error = ErrorResponse::RenderWindowNotFound(id, req.path);
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
  if (const auto err = ParseCameraPatch(body, IsStdMulti3dWindow(id), patch))
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
    m_RenderWindowBridge->SetStdMultiCamera(id, patch);
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
  const auto id = ReadRequiredPathParam(req, "id");

  if (!IsValidStdMultiWindowId(id))
  {
    const auto error = ErrorResponse::RenderWindowNotFound(id, req.path);
    this->SendErrorResponse(res, 404, error);
    return;
  }

  if (IsStdMulti3dWindow(id))
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
    state = m_RenderWindowBridge->GetStdMultiSelectedSlice(id);
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
  const auto id = ReadRequiredPathParam(req, "id");

  if (!IsValidStdMultiWindowId(id))
  {
    const auto error = ErrorResponse::RenderWindowNotFound(id, req.path);
    this->SendErrorResponse(res, 404, error);
    return;
  }

  if (IsStdMulti3dWindow(id))
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
  if (const auto err = ParseSliceStepBody(body,
        "StdMulti selected-slice",
        "Use PUT /rendering/selected-position to move the global crosshair by world coordinates.",
        step))
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
    m_RenderWindowBridge->SetStdMultiSelectedSliceStep(id, step);
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
  const auto id = ReadRequiredPathParam(req, "id");

  if (!IsValidStdMultiWindowId(id))
  {
    const auto error = ErrorResponse::RenderWindowNotFound(id, req.path);
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
    const auto imageData = m_RenderWindowBridge->TakeStdMultiWindowScreenshot(id, params.size, params.format);
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

// ============================================================================
// MxN editor handlers
//
// The valid set of window ids is the current layout's cell list, which
// changes after every PUT layout. The controller therefore stays at
// shape-validation only and lets the bridge be the authority on
// membership - unknown ids surface there as 404 RENDER_WINDOW_NOT_FOUND.
// ============================================================================

namespace
{
  /**
   * \brief True if `id` matches the canonical MxN window-id pattern from the
   *        OpenAPI spec (`^[A-Za-z][A-Za-z0-9.-]*__[A-Za-z0-9_.-]+$`).
   *
   * Mirrors the `MxNWindowName` parameter pattern in `openapi.json` so that
   * malformed ids are rejected at the controller boundary with 400
   * INVALID_REQUEST, before any UI-thread dispatch. Well-formed but unknown
   * ids fall through and surface from the bridge as 404 RENDER_WINDOW_NOT_FOUND.
   */
  bool IsValidMxNWindowId(const std::string& id)
  {
    static const std::regex kPattern(R"(^[A-Za-z][A-Za-z0-9.-]*__[A-Za-z0-9_.-]+$)");
    return std::regex_match(id, kPattern);
  }

  nlohmann::json MxNWindowInfoToWindowsListJson(const std::vector<mitk::MxNWindowInfo>& windows)
  {
    auto arr = nlohmann::json::array();
    for (const auto& w : windows)
    {
      nlohmann::json wj;
      wj["id"] = w.id;
      // Optional display label: emit only when set, mirroring the layout
      // schema's `name` field semantics (see mxn-layout-v2.schema.json).
      if (w.displayName.has_value())
      {
        wj["name"] = *w.displayName;
      }
      wj["kind"] = WindowKindToString(w.kind);
      // Under v2 the layout schema's view_direction enum is closed
      // {axial, sagittal, coronal, original} and required for every window
      // leaf, so every 2D MxN cell carries a value. The std::optional wrapper
      // exists purely for v3 forward-compat (3D cells with no plane). For 2D
      // cells we emit unconditionally; an unset optional under v2 indicates a
      // bridge-layer bug (loud failure beats silently dropping a required
      // field and producing a response that violates the OpenAPI schema).
      if (w.kind == WindowKind::TwoD)
      {
        if (!w.viewDirection.has_value())
        {
          mitkThrow() << "MxN window list: 2D cell '" << w.id
                      << "' has no view_direction set; expected an "
                         "AnatomicalPlane value under v2.";
        }
        wj["view_direction"] = AnatomicalPlaneToV2String(*w.viewDirection);
      }
      else if (w.viewDirection.has_value())
      {
        // v3 (3D cells): omit view_direction unless the descriptor still
        // carries one.
        wj["view_direction"] = AnatomicalPlaneToV2String(*w.viewDirection);
      }
      // links is always emitted; v2 has just one dimension (selection), v3
      // will add more keys here additively without breaking v2 clients.
      wj["links"] = { { "selection", w.selectionGroup } };
      arr.push_back(wj);
    }
    return arr;
  }
}

void RenderingController::HandleGET_mxnInfo(const httplib::Request& req, httplib::Response& res) const
{
  // Mirrors the stdmulti editor-info handler: walk the editor list, locate
  // the mxn entry, 503 EDITOR_NOT_ACTIVE if the editor is not open, 200 with
  // the windows list otherwise.
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
    if (ed.alias == "mxn")
    {
      if (!ed.active)
      {
        const auto error = ErrorResponse::EditorNotActive(
          "MxNMultiWidgetEditor is not open", req.path);
        this->SendErrorResponse(res, 503, error);
        return;
      }
      const auto j = EditorInfoToJson(ed, /*includeWindowList=*/true);
      res.status = 200;
      res.set_content(j.dump(), "application/json");
      return;
    }
  }

  // Provider returned a list that omits the "mxn" alias: the MxN editor
  // surface is unavailable from this workbench. Surface as 503 so clients
  // treat it the same as "no provider registered" (feature unavailable),
  // not as an internal crash.
  const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
  this->SendErrorResponse(res, 503, error);
}

void RenderingController::HandleGET_mxnWindows(const httplib::Request& req, httplib::Response& res) const
{
  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasMxNWindowListProvider())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  std::vector<MxNWindowInfo> windows;
  try
  {
    windows = m_RenderWindowBridge->ListMxNWindows();
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
    return;
  }

  res.status = 200;
  res.set_content(MxNWindowInfoToWindowsListJson(windows).dump(), "application/json");
}

void RenderingController::HandleGET_mxnWindow(const httplib::Request& req, httplib::Response& res) const
{
  const auto id = ReadRequiredPathParam(req, "id");

  // Shape-only check: malformed ids never reach the bridge dispatch.
  if (!IsValidMxNWindowId(id))
  {
    const auto error = ErrorResponse::InvalidRequest(
      "Malformed MxN window id '" + id + "'. Expected pattern: <prefix>__<bare> with URL-segment-safe characters.",
      req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasMxNWindowListProvider())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  std::vector<MxNWindowInfo> windows;
  try
  {
    windows = m_RenderWindowBridge->ListMxNWindows();
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
    return;
  }

  const auto it = std::find_if(windows.begin(), windows.end(),
    [&](const MxNWindowInfo& w) { return w.id == id; });
  if (it == windows.end())
  {
    const auto error = ErrorResponse::RenderWindowNotFound(id, req.path);
    this->SendErrorResponse(res, 404, error);
    return;
  }

  const bool is3d = (it->kind == WindowKind::ThreeD);

  nlohmann::json j;
  j["id"] = it->id;
  if (it->displayName.has_value())
  {
    j["name"] = *it->displayName;
  }
  j["kind"] = WindowKindToString(it->kind);
  // Mirrors the windows-list emission policy: under v2 every 2D cell has a
  // canonical AnatomicalPlane (Axial/Sagittal/Coronal/Original) and the
  // OpenAPI schema requires it. Throw if a 2D cell is missing one (bridge
  // contract violation). 3D cells (v3) omit the field.
  if (it->kind == WindowKind::TwoD)
  {
    if (!it->viewDirection.has_value())
    {
      mitkThrow() << "MxN window '" << it->id
                  << "' is 2D but has no view_direction set; expected an "
                     "AnatomicalPlane value under v2.";
    }
    j["view_direction"] = AnatomicalPlaneToV2String(*it->viewDirection);
  }
  else if (it->viewDirection.has_value())
  {
    j["view_direction"] = AnatomicalPlaneToV2String(*it->viewDirection);
  }
  j["links"] = { { "selection", it->selectionGroup } };
  j["has_camera"] = true;
  j["has_selected_slice"] = !is3d;
  j["has_selected_position"] = true;

  res.status = 200;
  res.set_content(j.dump(), "application/json");
}

// ----------------------------------------------------------------------
// Layout get/set
// ----------------------------------------------------------------------

void RenderingController::HandleGET_mxnLayout(const httplib::Request& req, httplib::Response& res) const
{
  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasMxNLayoutGetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  std::string layoutJson;
  try
  {
    layoutJson = m_RenderWindowBridge->GetMxNLayout();
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
    return;
  }

  res.status = 200;
  res.set_content(layoutJson, "application/json");
}

void RenderingController::HandlePUT_mxnLayout(const httplib::Request& req, httplib::Response& res) const
{
  if (req.body.empty())
  {
    const auto error = ErrorResponse::InvalidRequest(
      "Request body must be a v2.0 layout document.", req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  // Pre-parse the body for syntax validation. Schema / structural validation
  // is done by the engine's ApplyLayout via the bridge; duplicating it here
  // would drift. nlohmann::parse failures surface as 400.
  try
  {
    [[maybe_unused]] const auto parsed = nlohmann::json::parse(req.body);
  }
  catch (const nlohmann::json::exception& e)
  {
    const auto error = ErrorResponse::InvalidRequest(
      std::string("Invalid JSON body: ") + e.what(), req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasMxNLayoutSetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  std::string serialized;
  try
  {
    serialized = m_RenderWindowBridge->SetMxNLayout(req.body);
  }
  // Layout-specific catch: every mitk::Exception from ApplyLayout is treated
  // as a document-shape failure. Caught BEFORE the generic std::exception so
  // we don't fall through to MapBridgeException, which would map it to 422.
  // If the engine broadens ApplyLayout's failure model in the future, narrow
  // this catch.
  catch (const mitk::Exception& e)
  {
    const auto error = ErrorResponse::InvalidRequest(
      std::string("Layout document rejected: ") + e.what(), req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
    return;
  }

  // 200 + body (not 204): clients can refresh their cell name list from the
  // response without an additional GET.
  res.status = 200;
  res.set_content(serialized, "application/json");
}

// ----------------------------------------------------------------------
// Camera
// ----------------------------------------------------------------------

void RenderingController::HandleGET_mxnCamera(const httplib::Request& req, httplib::Response& res) const
{
  const auto id = ReadRequiredPathParam(req, "id");

  if (!IsValidMxNWindowId(id))
  {
    const auto error = ErrorResponse::InvalidRequest(
      "Malformed MxN window id '" + id + "'. Expected pattern: <prefix>__<bare> with URL-segment-safe characters.",
      req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasMxNCameraGetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  CameraState state;
  try
  {
    state = m_RenderWindowBridge->GetMxNCamera(id);
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
    return;
  }

  res.status = 200;
  res.set_content(CameraStateToJson(state).dump(), "application/json");
}

void RenderingController::HandlePUT_mxnCamera(const httplib::Request& req, httplib::Response& res) const
{
  const auto id = ReadRequiredPathParam(req, "id");

  if (!IsValidMxNWindowId(id))
  {
    const auto error = ErrorResponse::InvalidRequest(
      "Malformed MxN window id '" + id + "'. Expected pattern: <prefix>__<bare> with URL-segment-safe characters.",
      req.path);
    this->SendErrorResponse(res, 400, error);
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

  // is3d hard-coded false for v2: the schema's view_direction enum has no
  // "3d" value, so by construction every MxN cell is 2D. The plugin's
  // V2_MXN_WINDOW_KIND constant pins this invariant at compile time and
  // its v3 migration checklist (QmitkRestApiBridgeBindings.cpp) lists this
  // handler as a required update site -- when a 3D MxN cell type is
  // introduced, derive `is3d` from the windows list provider's `kind`
  // for the addressed cell here before calling ParseCameraPatch.
  CameraPatch patch;
  if (const auto err = ParseCameraPatch(body, /*is3d=*/false, patch))
  {
    const auto error = ErrorResponse::InvalidRequest(*err, req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasMxNCameraSetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  try
  {
    m_RenderWindowBridge->SetMxNCamera(id, patch);
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

// ----------------------------------------------------------------------
// Selected-slice
// ----------------------------------------------------------------------

void RenderingController::HandleGET_mxnSelectedSlice(const httplib::Request& req, httplib::Response& res) const
{
  const auto id = ReadRequiredPathParam(req, "id");

  if (!IsValidMxNWindowId(id))
  {
    const auto error = ErrorResponse::InvalidRequest(
      "Malformed MxN window id '" + id + "'. Expected pattern: <prefix>__<bare> with URL-segment-safe characters.",
      req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasMxNSelectedSliceGetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  SliceState state;
  try
  {
    state = m_RenderWindowBridge->GetMxNSelectedSlice(id);
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
    return;
  }

  res.status = 200;
  res.set_content(SliceStateToJson(state).dump(), "application/json");
}

void RenderingController::HandlePUT_mxnSelectedSlice(const httplib::Request& req, httplib::Response& res) const
{
  const auto id = ReadRequiredPathParam(req, "id");

  if (!IsValidMxNWindowId(id))
  {
    const auto error = ErrorResponse::InvalidRequest(
      "Malformed MxN window id '" + id + "'. Expected pattern: <prefix>__<bare> with URL-segment-safe characters.",
      req.path);
    this->SendErrorResponse(res, 400, error);
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

  // MxN slice PUT is step-only. World-anchor moves on a single cell live at
  // the per-cell selected-position resource; global anchor moves at
  // /rendering/selected-position.
  unsigned int step = 0;
  if (const auto err = ParseSliceStepBody(body,
        "MxN selected-slice",
        "Use PUT /rendering/editors/mxn/windows/{id}/selected-position for a per-cell "
        "world anchor, or PUT /rendering/selected-position for the global anchor.",
        step))
  {
    const auto error = ErrorResponse::InvalidRequest(*err, req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasMxNSelectedSliceStepSetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  try
  {
    m_RenderWindowBridge->SetMxNSelectedSliceStep(id, step);
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

// ----------------------------------------------------------------------
// Per-cell selected-position
//
// Distinct from the global /rendering/selected-position resource, which
// targets the StdMulti anchor. Per-cell anchors may legitimately diverge;
// the response/body shape is identical to the global resource's, just
// window-id-keyed.
// ----------------------------------------------------------------------

void RenderingController::HandleGET_mxnSelectedPosition(const httplib::Request& req, httplib::Response& res) const
{
  const auto id = ReadRequiredPathParam(req, "id");

  if (!IsValidMxNWindowId(id))
  {
    const auto error = ErrorResponse::InvalidRequest(
      "Malformed MxN window id '" + id + "'. Expected pattern: <prefix>__<bare> with URL-segment-safe characters.",
      req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasMxNSelectedPositionGetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  SelectedPositionInfo info;
  try
  {
    info = m_RenderWindowBridge->GetMxNSelectedPosition(id);
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
    return;
  }

  res.status = 200;
  res.set_content(SerializeSelectedPositionInfoToJson(info).dump(), "application/json");
}

void RenderingController::HandlePUT_mxnSelectedPosition(const httplib::Request& req, httplib::Response& res) const
{
  const auto id = ReadRequiredPathParam(req, "id");

  if (!IsValidMxNWindowId(id))
  {
    const auto error = ErrorResponse::InvalidRequest(
      "Malformed MxN window id '" + id + "'. Expected pattern: <prefix>__<bare> with URL-segment-safe characters.",
      req.path);
    this->SendErrorResponse(res, 400, error);
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

  Point3D newPos;
  if (const auto err = ParsePositionFromBody(body, newPos))
  {
    const auto error = ErrorResponse::InvalidRequest(*err, req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasMxNSelectedPositionSetter())
  {
    const auto error = ErrorResponse::RenderWindowNotAvailable(req.path);
    this->SendErrorResponse(res, 503, error);
    return;
  }

  try
  {
    m_RenderWindowBridge->SetMxNSelectedPosition(id, newPos);
    res.status = 204;
  }
  catch (const mitk::Exception& e)
  {
    const auto error = ErrorResponse::RenderingError(
      std::string("Position update failed: ") + e.what(), req.path);
    this->SendErrorResponse(res, 422, error);
  }
  catch (const std::exception& e)
  {
    const auto [status, payload] = MapBridgeException(e, req.path);
    this->SendErrorResponse(res, status, payload);
  }
}

// ----------------------------------------------------------------------
// Screenshots
// ----------------------------------------------------------------------

void RenderingController::HandleGET_mxnScreenshot(const httplib::Request& req, httplib::Response& res) const
{
  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasMxNEditorScreenshotProvider())
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
    const auto imageData = m_RenderWindowBridge->TakeMxNEditorScreenshot(params.size, params.format);
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

void RenderingController::HandleGET_mxnWindowScreenshot(const httplib::Request& req, httplib::Response& res) const
{
  const auto id = ReadRequiredPathParam(req, "id");

  if (!IsValidMxNWindowId(id))
  {
    const auto error = ErrorResponse::InvalidRequest(
      "Malformed MxN window id '" + id + "'. Expected pattern: <prefix>__<bare> with URL-segment-safe characters.",
      req.path);
    this->SendErrorResponse(res, 400, error);
    return;
  }

  if (m_RenderWindowBridge == nullptr || !m_RenderWindowBridge->HasMxNWindowScreenshotProvider())
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
    const auto imageData = m_RenderWindowBridge->TakeMxNWindowScreenshot(id, params.size, params.format);
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

bool RenderingController::IsValidStdMultiWindowId(const std::string& id)
{
  return id == "axial" || id == "sagittal" || id == "coronal" || id == "3d";
}

bool RenderingController::IsStdMulti3dWindow(const std::string& id)
{
  return id == "3d";
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

  if (const auto* ru = dynamic_cast<const RenderWindowBridgeRendererUnavailableException*>(&e))
    return {500, ErrorResponse::RendererUnavailable(ru->what(), instance)};

  return {500, ErrorResponse::InternalError(e.what(), instance)};
}

}
