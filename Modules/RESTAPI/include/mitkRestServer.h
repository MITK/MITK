/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRestServer_h
#define mitkRestServer_h

#include <MitkRESTAPIExports.h>
#include <mitkIRestServerService.h>

#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <set>

// Forward declaration for httplib
namespace httplib { class Server; }

namespace mitk
{
  // Forward declarations
  class DataStorageBridge;
  class HealthController;
  class DataStorageController;

  /**
   * @brief HTTP REST server implementation.
   *
   * Manages cpp-httplib server lifecycle and routes requests
   * to appropriate controllers.
   */
  class MITKRESTAPI_EXPORT RestServer : public IRestServerService
  {
  public:
    RestServer();
    ~RestServer() override;

    // Non-copyable, non-movable (due to thread)
    RestServer(const RestServer&) = delete;
    RestServer& operator=(const RestServer&) = delete;
    RestServer(RestServer&&) = delete;
    RestServer& operator=(RestServer&&) = delete;

    bool Start() override;
    void Stop() override;
    bool IsRunning() const override;

    void SetConfig(const RestServerConfig& config) override;
    RestServerConfig GetPendingConfig() const override;
    std::optional<RestServerConfig> GetRunningConfig() const override;

    void SetDataStorage(DataStorage* dataStorage) override;
    DataStorage* GetDataStorage() const override;

    std::optional<std::string> GetServerUrl() const override;
    std::optional<std::string> GetLastError() const override;

    std::vector<std::string> GetClientIPs() const override;
    std::optional<RequestInfo> GetLastRequestInfo() const override;

  private:
    void RecordRequest(const std::string& endpoint, const std::string& method,
                       int responseCode, const std::string& clientIP);
    void RegisterRoutes();
    void ServerThreadFunc();

    // Server components
    std::unique_ptr<httplib::Server> m_Server;
    std::unique_ptr<std::thread> m_ServerThread;

    // Business logic
    std::unique_ptr<DataStorageBridge> m_Bridge;
    std::unique_ptr<HealthController> m_HealthController;
    std::unique_ptr<DataStorageController> m_DataStorageController;

    // State
    mutable std::mutex m_Mutex;
    RestServerConfig m_PendingConfig;
    std::optional<RestServerConfig> m_RunningConfig;
    std::atomic<bool> m_Running{false};
    std::optional<std::string> m_LastError;

    // Request tracking
    std::set<std::string> m_ClientIPs;
    std::optional<RequestInfo> m_LastRequest;
    // Temporary directory for data serialization
    std::string m_TempDirectory;

    /**
     * @brief Create and setup the temporary directory for data operations.
     *
     * Called during Start(). Creates a unique temp directory for this server session.
     *
     * @return true if successful.
     */
    bool SetupTempDirectory();

    /**
     * @brief Clean up the temporary directory.
     *
     * Called during Stop(). Removes all files and the directory itself.
     * Takes the path as a parameter to allow thread-safe cleanup after
     * the path has been copied from m_TempDirectory while holding the lock.
     *
     * @param tempDir Path to the temporary directory to clean up.
     */
    static void CleanupTempDirectory(const std::string& tempDir);
  };
}

#endif
