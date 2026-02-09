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
#include <mitkLog.h>

#include <memory>
#include <mutex>

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

    // Create RestServer instance
    m_RestServer = std::make_unique<RestServer>();

    // Register as IRestServerService
    m_RestServerRegistration = context->RegisterService<IRestServerService>(m_RestServer.get());

    MITK_INFO << "REST API module loaded, service registered";

    this->StartServiceTracking();
  }

  void Unload(us::ModuleContext* /*context*/) override
  {
    this->StopServiceTracking();

    // Stop server
    if (m_RestServer)
    {
      m_RestServer->Stop();
    }

    // Unregister service
    if (m_RestServerRegistration)
    {
      m_RestServerRegistration.Unregister();
    }

    // Clean up
    m_RestServer.reset();
    m_Context = nullptr;

    MITK_INFO << "REST API module unloaded";
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

    // Disconnect DataStorage from REST server
    if (m_RestServer != nullptr)
    {
      m_RestServer->SetDataStorage(nullptr);
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

    config.host = restApiPrefs->Get("host", defaults.host);
    config.port = restApiPrefs->GetInt("port", defaults.port);
    config.enabled = restApiPrefs->GetBool("enabled", defaults.enabled);
    config.threadPoolSize = restApiPrefs->GetInt("threadPoolSize", defaults.threadPoolSize);
    config.readTimeoutSeconds = restApiPrefs->GetInt("readTimeoutSeconds", defaults.readTimeoutSeconds);
    config.writeTimeoutSeconds = restApiPrefs->GetInt("writeTimeoutSeconds", defaults.writeTimeoutSeconds);
    config.requireAuth = restApiPrefs->GetBool("requireAuth", defaults.requireAuth);
    config.apiToken = restApiPrefs->Get("apiToken", defaults.apiToken);

    m_RestServer->SetConfig(config);

    // Auto-start if enabled and not already running
    bool autoStart = restApiPrefs->GetBool("autoStart", false);
    if (autoStart && config.enabled && !m_RestServer->IsRunning())
    {
      MITK_INFO << "Auto-starting REST API server";
      m_RestServer->Start();
    }
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
      MITK_INFO << "Connecting DataStorage to REST server";
      m_RestServer->SetDataStorage(dataStorage);
    }
  }

  us::ModuleContext* m_Context = nullptr;

  std::unique_ptr<RestServer> m_RestServer;
  us::ServiceRegistration<IRestServerService> m_RestServerRegistration;

  std::mutex m_Mutex;
};

} // namespace mitk

US_EXPORT_MODULE_ACTIVATOR(mitk::RestApiActivator)
