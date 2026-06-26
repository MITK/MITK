/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleActivator.h>
#include <usModuleContext.h>

#include <mitkRestServer.h>
#include <mitkIRestServerService.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>
#include <mitkIDataStorageService.h>
#include <mitkStorageThreadDispatcherBase.h>
#include <mitkLog.h>

#include <cstdlib>
#include <memory>
#include <mutex>
#include <sstream>

namespace mitk
{

/**
 * @brief Module activator for the REST API autoload module.
 *
 * Responsibilities:
 * - Creates RestServer instance and registers it as IRestServerService
 * - Tracks IPreferencesService for configuration
 * - Tracks IDataStorageService to connect DataStorage to the REST server
 * - Auto-starts server based on preferences
 */
class RestApiActivator : public us::ModuleActivator
{
public:
  RestApiActivator() = default;
  ~RestApiActivator() override = default;

  void Load(us::ModuleContext* context) override
  {
    m_Context = context;

    // Detect a duplicate registration. Only one autoload-module instance is
    // expected to register IRestServerService; a second one indicates a stale
    // RESTAPIAutoload artifact in another autoload directory on disk (e.g.,
    // a leftover from a previous AUTOLOAD_WITH target). Two RestServer
    // instances would silently desync the bridge wired by the Qt plugin from
    // the server consulted by IRestServerService consumers.
    const auto existing = context->GetServiceReferences<IRestServerService>();
    if (!existing.empty())
    {
      MITK_ERROR << "Another IRestServerService is already registered before "
                    "RestApiActivator::Load. This usually means a stale "
                    "RESTAPIAutoload binary exists in a second autoload "
                    "directory on disk (e.g., a leftover from a previous "
                    "AUTOLOAD_WITH target). Locate and remove duplicate "
                    "RESTAPIAutoload artifacts in the build/install tree.";
    }

    m_RestServer = std::make_unique<RestServer>();

    m_RestServerRegistration = context->RegisterService<IRestServerService>(m_RestServer.get());

    this->StartServiceTracking();
  }

  void Unload(us::ModuleContext* /*context*/) override
  {
    this->StopServiceTracking();

    if (m_RestServer)
    {
      m_RestServer->Stop();
    }

    if (m_RestServerRegistration)
    {
      m_RestServerRegistration.Unregister();
    }

    m_RestServer.reset();
    m_Context = nullptr;
  }

private:

  /**
   * @brief Start tracking both IDataStorageService and IPreferencesService.
   *
   * Uses a single unfiltered AddServiceListener because CppMicroServices keys
   * listeners by receiver object pointer — only one listener per 'this' is allowed.
   * Dispatching to the correct handler happens in OnServiceChanged.
   */
  void StartServiceTracking()
  {
    if (m_Context == nullptr)
    {
      return;
    }

    std::lock_guard<std::mutex> lock(m_Mutex);

    // Register a single unfiltered listener. CppMicroServices keys listeners by
    // receiver pointer, so only one AddServiceListener per 'this' is possible.
    // We dispatch to the appropriate handler in OnServiceChanged based on service type.
    m_Context->AddServiceListener(this, &RestApiActivator::OnServiceChanged);

    // Check if services are already available
    auto dsRefs = m_Context->GetServiceReferences<IDataStorageService>();
    if (!dsRefs.empty())
    {
      auto* service = m_Context->GetService(dsRefs.front());
      if (service != nullptr)
      {
        this->ConnectDataStorage(service);
        m_Context->UngetService(dsRefs.front());
      }
    }

    auto prefRefs = m_Context->GetServiceReferences<IPreferencesService>();
    if (!prefRefs.empty())
    {
      auto* service = m_Context->GetService(prefRefs.front());
      if (service != nullptr)
      {
        this->ApplyPreferences(service);
        m_Context->UngetService(prefRefs.front());
      }
    }
  }

  void StopServiceTracking()
  {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_Context != nullptr)
    {
      try
      {
        m_Context->RemoveServiceListener(this, &RestApiActivator::OnServiceChanged);
      }
      catch (...)
      {
        MITK_WARN << "Could not remove service listener";
      }
    }

    // Disconnect DataStorage and dispatcher from REST server
    if (m_RestServer != nullptr)
    {
      m_RestServer->SetDataStorage(nullptr);
      m_RestServer->SetDispatcher(nullptr);
      m_DataStorageConnected = false;
      m_Dispatcher = nullptr;
    }
  }

  void OnServiceChanged(const us::ServiceEvent event)
  {
    if (m_Context == nullptr)
    {
      return;
    }

    const auto& ref = event.GetServiceReference();
    const auto objectClass = ref.GetProperty(us::ServiceConstants::OBJECTCLASS());

    // Dispatch to the appropriate handler based on service type
    if (objectClass.ToString().find(us_service_interface_iid<IDataStorageService>()) != std::string::npos)
    {
      this->HandleDataStorageEvent(event);
    }
    else if (objectClass.ToString().find(us_service_interface_iid<IPreferencesService>()) != std::string::npos)
    {
      this->HandlePreferencesEvent(event);
    }
  }

  void HandlePreferencesEvent(const us::ServiceEvent& event)
  {
    if (event.GetType() == us::ServiceEvent::REGISTERED || event.GetType() == us::ServiceEvent::MODIFIED)
    {
      us::ServiceReference<IPreferencesService> ref = event.GetServiceReference();
      auto* service = m_Context->GetService(ref);
      if (service != nullptr)
      {
        std::lock_guard<std::mutex> lock(m_Mutex);
        this->ApplyPreferences(service);
        m_Context->UngetService(ref);
      }
    }
  }

  void ApplyPreferences(IPreferencesService* prefsService)
  {
    if (prefsService == nullptr || m_RestServer == nullptr)
    {
      return;
    }

    auto* prefs = prefsService->GetSystemPreferences();
    if (prefs == nullptr)
    {
      return;
    }

    // Get REST API preferences node
    auto* restApiPrefs = prefs->Node("/org.mitk.restapi");

    // Use RestServerConfig defaults as single source of truth
    RestServerConfig defaults;
    RestServerConfig config;

    // Server settings
    config.host = restApiPrefs->Get("host", defaults.host);
    config.port = restApiPrefs->GetInt("port", defaults.port);
    config.enabled = restApiPrefs->GetBool("enabled", defaults.enabled);
    config.threadPoolSize = restApiPrefs->GetInt("threadPoolSize", defaults.threadPoolSize);
    config.readTimeoutSeconds = restApiPrefs->GetInt("readTimeoutSeconds", defaults.readTimeoutSeconds);
    config.writeTimeoutSeconds = restApiPrefs->GetInt("writeTimeoutSeconds", defaults.writeTimeoutSeconds);

    // Client access control
    const int accessModeInt = restApiPrefs->GetInt("clientAccessMode", static_cast<int>(defaults.clientAccessMode));
    if (accessModeInt >= 0 && accessModeInt <= 2)
    {
      config.clientAccessMode = static_cast<ClientAccessMode>(accessModeInt);
    }
    else
    {
      MITK_WARN << "Invalid clientAccessMode value " << accessModeInt << "; defaulting to LocalhostOnly";
      config.clientAccessMode = ClientAccessMode::LocalhostOnly;
    }

    const std::string allowedIPsStr = restApiPrefs->Get("allowedClientIPs", "");
    if (!allowedIPsStr.empty())
    {
      std::istringstream stream(allowedIPsStr);
      std::string line;
      while (std::getline(stream, line))
      {
        // Trim whitespace
        auto start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        auto end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);
        if (!line.empty())
        {
          config.allowedClientIPs.push_back(line);
        }
      }
    }

    // Authentication
    config.requireAuth = restApiPrefs->GetBool("requireAuth", defaults.requireAuth);
    // SECURITY NOTE: the token stored in preferences is unencrypted. Protect the
    // preferences file with appropriate filesystem permissions. As a more secure
    // alternative, set the MITK_REST_API_TOKEN environment variable; when set it
    // takes precedence over the stored preference value.
    const char* envToken = std::getenv("MITK_REST_API_TOKEN");
    if (envToken != nullptr && envToken[0] != '\0')
    {
      config.apiToken = envToken;
    }
    else
    {
      config.apiToken = restApiPrefs->Get("apiToken", defaults.apiToken);
    }

    // Payload limits
    config.maxPayloadSizeMB = restApiPrefs->GetInt("maxPayloadSizeMB", defaults.maxPayloadSizeMB);

    // File path restrictions
    const int fileAccessModeInt = restApiPrefs->GetInt("fileAccessMode", static_cast<int>(defaults.fileAccessMode));
    if (fileAccessModeInt >= 0 && fileAccessModeInt <= 1)
    {
      config.fileAccessMode = static_cast<FileAccessMode>(fileAccessModeInt);
    }
    else
    {
      MITK_WARN << "Invalid fileAccessMode value " << fileAccessModeInt << "; defaulting to AllowedDirectories";
      config.fileAccessMode = FileAccessMode::AllowedDirectories;
    }

    const std::string allowedDirsStr = restApiPrefs->Get("allowedFileDirectories", "");
    if (!allowedDirsStr.empty())
    {
      // Preferences stores directories as semicolon-separated string
      std::istringstream stream(allowedDirsStr);
      std::string entry;
      while (std::getline(stream, entry, ';'))
      {
        auto start = entry.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        auto end = entry.find_last_not_of(" \t\r\n");
        entry = entry.substr(start, end - start + 1);
        if (!entry.empty())
        {
          config.allowedFileDirectories.push_back(entry);
        }
      }
    }

    // Rate limiting
    config.rateLimitEnabled = restApiPrefs->GetBool("rateLimitEnabled", defaults.rateLimitEnabled);
    config.rateLimitPerMinute = restApiPrefs->GetInt("rateLimitPerMinute", defaults.rateLimitPerMinute);

    // HTTPS
    config.httpsEnabled = restApiPrefs->GetBool("httpsEnabled", defaults.httpsEnabled);
    config.sslCertPath = restApiPrefs->Get("sslCertPath", defaults.sslCertPath);
    config.sslKeyPath = restApiPrefs->Get("sslKeyPath", defaults.sslKeyPath);

    m_RestServer->SetConfig(config);

    // Record the auto-start intent and gate state. The actual Start() is deferred
    // (see MaybeAutoStart) so it never runs inline during plugin bring-up, where
    // creating and waiting on the listen thread under the loader lock deadlocks.
    m_AutoStartRequested = restApiPrefs->GetBool("autoStart", false);
    m_ConfigEnabled = config.enabled;
    this->MaybeAutoStart();
  }

  void HandleDataStorageEvent(const us::ServiceEvent& event)
  {
    if (event.GetType() == us::ServiceEvent::REGISTERED)
    {
      us::ServiceReference<IDataStorageService> ref = event.GetServiceReference();
      auto* service = m_Context->GetService(ref);
      if (service != nullptr)
      {
        std::lock_guard<std::mutex> lock(m_Mutex);
        this->ConnectDataStorage(service);
        m_Context->UngetService(ref);
      }
    }
    else if (event.GetType() == us::ServiceEvent::UNREGISTERING)
    {
      MITK_DEBUG << "DataStorageService unregistering, disconnecting DataStorage from REST server";
      if (m_RestServer != nullptr)
      {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_RestServer->SetDataStorage(nullptr);
        m_RestServer->SetDispatcher(nullptr);
        m_DataStorageConnected = false;
        m_Dispatcher = nullptr;
      }
    }
  }

  void ConnectDataStorage(IDataStorageService* service)
  {
    if (service == nullptr || m_RestServer == nullptr)
    {
      return;
    }

    auto dataStorage = service->GetActiveDataStorage();
    if (dataStorage.IsNotNull())
    {
      MITK_DEBUG << "Connecting DataStorage to REST server";
      m_RestServer->SetDataStorage(dataStorage);

      StorageThreadDispatcherBase::Pointer dispatcher = service->GetDispatcher();
      if (dispatcher.IsNotNull())
      {
        m_RestServer->SetDispatcher(dispatcher);
      }

      m_Dispatcher = dispatcher;        // cached for deferred auto-start (null in headless)
      m_DataStorageConnected = true;
      this->MaybeAutoStart();
    }
  }

  /**
   * @brief Post (or, headless, directly perform) the auto-start once all
   *        prerequisites hold. Idempotent: starts at most once per module load.
   *
   * Prerequisites: auto-start requested, server enabled, a DataStorage is
   * connected, and the server is not already running. Called from both
   * ApplyPreferences and ConnectDataStorage so whichever event completes the
   * prerequisite set triggers the start, regardless of arrival order.
   *
   * The start is deferred onto the dispatch (main) thread rather than run inline:
   * during plugin bring-up the loader lock is held, and creating+waiting on the
   * listen thread there deadlocks. Posting a queued task escapes that window.
   *
   * @pre m_Mutex is held by the caller.
   */
  void MaybeAutoStart()
  {
    if (m_AutoStartPosted || m_RestServer == nullptr)
    {
      return;
    }
    if (!m_AutoStartRequested || !m_ConfigEnabled || !m_DataStorageConnected)
    {
      return;
    }
    if (m_RestServer->IsRunning())
    {
      return;
    }

    m_AutoStartPosted = true;

    if (m_Dispatcher.IsNotNull())
    {
      MITK_INFO << "Auto-starting REST API server (deferred to the main event loop)";
      m_Dispatcher->Post([this]() { this->StartDeferred(); });
    }
    else
    {
      // No dispatcher means no event loop to defer onto, so start inline. This is
      // safe only because no current host registers IDataStorageService without a
      // dispatcher: such a host would drive ConnectDataStorage() to here during
      // this module's Load(), i.e. under the loader lock, reintroducing the very
      // deadlock the dispatcher path avoids. A genuinely dispatcher-less REST host
      // must add its own non-Qt deferral rather than rely on this inline start.
      MITK_INFO << "Auto-starting REST API server";
      m_RestServer->Start();
    }
  }

  /**
   * @brief Body of the deferred auto-start; runs on the dispatch (main) thread.
   *
   * Re-checks the gate because state can change between posting and running (the
   * user may have started the server manually, the DataStorage may have
   * disconnected, or the server may have been disabled in the meantime).
   *
   * No liveness guard is needed: module Unload() runs on this same main thread
   * after the Qt event loop has stopped, so this callback never races destruction
   * of the activator or the RestServer. If that unload-thread assumption is ever
   * invalidated, hold the RestServer through a shared owning handle and capture a
   * weak handle here that this task locks before use.
   */
  void StartDeferred()
  {
    RestServer* server = nullptr;
    {
      std::lock_guard<std::mutex> lock(m_Mutex);
      if (!m_AutoStartRequested || !m_ConfigEnabled || !m_DataStorageConnected ||
          m_RestServer == nullptr || m_RestServer->IsRunning())
      {
        return;
      }
      server = m_RestServer.get();
    }

    // Start() outside the activator lock: it briefly blocks in wait_until_ready()
    // and takes the RestServer's own mutex. Holding m_Mutex across it is
    // unnecessary and would stall service-change handling. Safe to use `server`
    // after the unlock because Unload() (the only path that destroys it) runs on
    // this same main thread and cannot interleave.
    server->Start();
  }

  us::ModuleContext* m_Context = nullptr;

  std::unique_ptr<RestServer> m_RestServer;
  us::ServiceRegistration<IRestServerService> m_RestServerRegistration;

  std::mutex m_Mutex;

  // Auto-start gate state (all guarded by m_Mutex). Auto-start is deferred onto
  // the dispatch thread and fires at most once, when every prerequisite holds.
  StorageThreadDispatcherBase::Pointer m_Dispatcher;  // cached for deferred auto-start; may be null (headless)
  bool m_AutoStartRequested = false;                  // from the autoStart preference
  bool m_ConfigEnabled = false;                       // from config.enabled
  bool m_DataStorageConnected = false;                // set once a DataStorage is wired
  bool m_AutoStartPosted = false;                     // latch: auto-start is posted/performed at most once
};

} // namespace mitk

US_EXPORT_MODULE_ACTIVATOR(mitk::RestApiActivator)
