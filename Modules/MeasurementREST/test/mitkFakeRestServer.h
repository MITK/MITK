/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFakeRestServer_h
#define mitkFakeRestServer_h

#include <mitkIRestServerService.h>

#include <map>
#include <string>

namespace mitk
{
  /**
   * A server that only names nodes: it gives each node it is asked for the
   * uid "node-<n>" and finds it again by that uid. Everything else of the
   * interface does nothing.
   */
  class FakeRestServer : public IRestServerService
  {
  public:
    DataStorage::Pointer Storage;

    bool Start() override { return false; }
    bool Start(const RestServerConfig&, const std::string&) override { return false; }
    std::string GetStartedBy() const override { return {}; }
    void Stop() override {}
    bool IsRunning() const override { return false; }
    void SetConfig(const RestServerConfig&) override {}
    RestServerConfig GetPendingConfig() const override { return {}; }
    std::optional<RestServerConfig> GetRunningConfig() const override { return std::nullopt; }
    RenderWindowBridge* GetRenderWindowBridge() override { return nullptr; }
    void SetDataStorage(DataStorage* dataStorage) override { this->Storage = dataStorage; }
    DataStorage::Pointer GetDataStorage() const override { return this->Storage; }
    RestRouteId AddRoute(RestRoute) override { return {}; }
    void RemoveRoute(RestRouteId) override {}
    RestResponse HandleRequest(RestRequest) override { return {}; }
    std::shared_ptr<RestJob> CreateJob(const std::string&) override { return nullptr; }

    DataNodeHandle FindNode(const std::string& uid) const override
    {
      const auto it = m_Nodes.find(uid);
      return it != m_Nodes.end() ? this->HoldNode(it->second) : nullptr;
    }

    DataNodeHandle HoldNode(DataNode* node) const override
    {
      if (node == nullptr)
        return nullptr;

      return DataNodeHandle(node, [keep = DataNode::Pointer(node)](DataNode*) {});
    }

    std::string GetNodeUid(DataNode* node) const override
    {
      for (const auto& [uid, known] : m_Nodes)
      {
        if (known.GetPointer() == node)
          return uid;
      }

      const auto uid = "node-" + std::to_string(m_Nodes.size() + 1);
      m_Nodes[uid] = node;
      return uid;
    }

    void Dispatch(const std::function<void()>& task) const override { task(); }
    void Post(std::function<void()> task) const override { task(); }
    std::string CreateToken(const std::string&, const RestTokenRights&) override { return {}; }
    void RemoveToken(const std::string&) override {}
    std::vector<RestTokenInfo> GetTokens() const override { return {}; }
    std::optional<std::string> GetServerUrl() const override { return std::nullopt; }
    std::optional<std::string> GetLastError() const override { return std::nullopt; }
    std::vector<std::string> GetClientIPs() const override { return {}; }
    std::optional<RequestInfo> GetLastRequestInfo() const override { return std::nullopt; }
    void SetLogLimit(std::optional<unsigned int>) override {}
    std::optional<unsigned int> GetLogLimit() const override { return std::nullopt; }
    std::vector<RequestInfo> GetRequestLog() const override { return {}; }
    void ClearRequestLog() override {}
    uint64_t GetRequestLogVersion() const override { return 0; }
    std::optional<int64_t> GetUptimeSeconds() const override { return std::nullopt; }

  private:
    mutable std::map<std::string, DataNode::Pointer> m_Nodes;
  };
}

#endif
