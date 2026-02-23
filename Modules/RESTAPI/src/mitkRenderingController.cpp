/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRenderingController.h"
#include "mitkErrorResponse.h"
#include <mitkRenderingManager.h>

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

  this->Dispatch([type]() {
    RenderingManager::GetInstance()->RequestUpdateAll(type);
  });

  res.status = 204;
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
  std::string uid;

  if (!req.body.empty())
  {
    try
    {
      const auto body = nlohmann::json::parse(req.body);
      if (body.contains("uid"))
      {
        uid = body["uid"].get<std::string>();
      }
    }
    catch (const nlohmann::json::exception&)
    {
      const auto error = ErrorResponse::InvalidRequest("Invalid JSON body.", req.path);
      this->SendErrorResponse(res, 400, error);
      return;
    }
  }

  if (!uid.empty())
  {
    // Node-scoped reinit: fit views to the geometry of the specified node.
    const auto result = m_Bridge.GetNodeData(uid);

    if (!result.nodeFound)
    {
      const auto error = ErrorResponse::NodeNotFound(uid, req.path);
      this->SendErrorResponse(res, 404, error);
      return;
    }

    const auto& data = result.data;

    if (data == nullptr)
    {
      const auto error = ErrorResponse::NoData(uid, req.path);
      this->SendErrorResponse(res, 422, error);
      return;
    }

    const auto* geometry = data->GetTimeGeometry();

    if (geometry == nullptr)
    {
      const auto error = ErrorResponse::Create(
        "NO_GEOMETRY",
        "No Geometry",
        "Node '" + uid + "' has no usable time geometry.",
        422,
        req.path);
      this->SendErrorResponse(res, 422, error);
      return;
    }

    // Capture data (smart pointer clone) to keep geometry alive across dispatch.
    this->Dispatch([data, geometry]() {
      RenderingManager::GetInstance()->InitializeViews(
        geometry, RenderingManager::REQUEST_UPDATE_ALL, true);
    });
  }
  else
  {
    // Global reinit: fit all views to the bounding box of all visible data.
    const auto dataStorage = m_Bridge.GetDataStorage();

    this->Dispatch([dataStorage]() {
      RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);
    });
  }

  res.status = 204;
}

void RenderingController::Dispatch(std::function<void()> task) const
{
  if (m_Dispatcher == nullptr)
  {
    task();
  }
  else
  {
    m_Dispatcher->Execute(std::move(task));
  }
}

void RenderingController::SendErrorResponse(httplib::Response& res, int status, const nlohmann::json& error)
{
  res.status = status;
  res.set_content(error.dump(), "application/json");
}

}
