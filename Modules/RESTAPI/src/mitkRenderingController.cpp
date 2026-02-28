/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRenderingController.h"
#include "mitkErrorResponse.h"
#include <mitkDataStorage.h>
#include <mitkException.h>
#include <mitkRenderingManager.h>

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
        auto nodeSet = DataStorage::SetOfObjects::New();
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
          // TODO(#728): remove const_cast once DataStorage::SetOfObjects accepts ConstPointer
          nodeSet->InsertElement(nodeSet->Size(),
            const_cast<DataNode*>(nodes[i].GetPointer()));
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
