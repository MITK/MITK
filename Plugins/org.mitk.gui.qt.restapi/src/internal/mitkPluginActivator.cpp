/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPluginActivator.h"
#include "QmitkRestApiPreferencePage.h"
#include "QmitkRestApiView.h"
#include "QmitkRestApiBridgeBindings.h"

#include <mitkIRestServerService.h>
#include <mitkRenderWindowBridge.h>

#include <usModuleRegistry.h>
#include <usModule.h>
#include <usModuleContext.h>

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE

namespace mitk
{
  ctkPluginContext* RestApiPluginActivator::m_context = nullptr;
  RestApiPluginActivator* RestApiPluginActivator::m_Instance = nullptr;

  RestApiPluginActivator::RestApiPluginActivator()
  {
    m_Instance = this;
  }

  RestApiPluginActivator::~RestApiPluginActivator()
  {
    m_Instance = nullptr;
  }

  void RestApiPluginActivator::start(ctkPluginContext* context)
  {
    BERRY_REGISTER_EXTENSION_CLASS(QmitkRestApiPreferencePage, context)
    BERRY_REGISTER_EXTENSION_CLASS(QmitkRestApiView, context)
    m_Context = context;

    // Get the MitkCore module context to access CppMicroServices services.
    m_MitkContext = us::ModuleRegistry::GetModule(1)->GetModuleContext();

    // Add a filtered service listener for IRestServerService.
    const std::string filter =
      "(" + us::ServiceConstants::OBJECTCLASS() + "=" +
      us_service_interface_iid<IRestServerService>() + ")";
    m_MitkContext->AddServiceListener(
      this, &RestApiPluginActivator::OnRestServerServiceChanged, filter);

    // Handle an already-registered IRestServerService.
    const auto refs = m_MitkContext->GetServiceReferences<IRestServerService>();
    if (!refs.empty())
    {
      this->ConnectRestServer(refs.front());
    }
  }

  void RestApiPluginActivator::stop(ctkPluginContext*)
  {
    if (m_MitkContext != nullptr)
    {
      m_MitkContext->RemoveServiceListener(
        this, &RestApiPluginActivator::OnRestServerServiceChanged);

      // If the UNREGISTERING event has not yet fired (shutdown ordering is not
      // guaranteed), clean up now. m_RenderWindowBridge being non-null means
      // ConnectRestServer ran but DisconnectRestServer has not yet run.
      if (m_RenderWindowBridge != nullptr)
      {
        const auto refs = m_MitkContext->GetServiceReferences<IRestServerService>();
        if (!refs.empty())
          this->DisconnectRestServer(refs.front());

        // Unconditional clear: DisconnectRestServer also resets the guard, but
        // we still need to handle the "service already gone" branch.
        m_RenderWindowBridge = nullptr;
      }

      m_MitkContext = nullptr;
    }

    this->m_context = nullptr;
  }

  RestApiPluginActivator* RestApiPluginActivator::getDefault()
  {
    return m_Instance;
  }

  ctkPluginContext* RestApiPluginActivator::getContext()
  {
    return m_context;
  }

  void RestApiPluginActivator::OnRestServerServiceChanged(const us::ServiceEvent event)
  {
    if (event.GetType() == us::ServiceEvent::REGISTERED)
    {
      this->ConnectRestServer(event.GetServiceReference());
    }
    else if (event.GetType() == us::ServiceEvent::UNREGISTERING)
    {
      this->DisconnectRestServer(event.GetServiceReference());
    }
  }

  void RestApiPluginActivator::ConnectRestServer(const us::ServiceReferenceU& ref)
  {
    if (m_MitkContext == nullptr || !ref)
    {
      return;
    }

    const us::ServiceReference<IRestServerService> typedRef = ref;
    auto* const service = m_MitkContext->GetService(typedRef);

    if (service == nullptr)
    {
      m_MitkContext->UngetService(typedRef);
      return;
    }

    // Obtain the server-owned bridge and configure the UI-layer callbacks.
    // We cache a non-owning pointer solely as a pointer-identity guard for
    // DisconnectRestServer; the service is released immediately after setup.
    m_RenderWindowBridge = service->GetRenderWindowBridge();

    ConfigureRestApiBridgeCallbacks(m_RenderWindowBridge);

    m_MitkContext->UngetService(typedRef);
  }

  void RestApiPluginActivator::DisconnectRestServer(const us::ServiceReferenceU& ref)
  {
    if (m_MitkContext == nullptr || !ref)
    {
      m_RenderWindowBridge = nullptr;
      return;
    }

    const us::ServiceReference<IRestServerService> typedRef = ref;
    auto* const service = m_MitkContext->GetService(typedRef);

    if (service != nullptr)
    {
      // m_RenderWindowBridge is used as a pointer-identity guard only: verify
      // that the bridge on the server is the one we configured before resetting.
      // Never dereference m_RenderWindowBridge directly — go through the service.
      if (service->GetRenderWindowBridge() == m_RenderWindowBridge)
        service->GetRenderWindowBridge()->ResetCallbacks();

      m_MitkContext->UngetService(typedRef);
    }

    m_RenderWindowBridge = nullptr;
  }
}
