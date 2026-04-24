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

#include <mitkIRestServerService.h>
#include <mitkRenderWindowBridge.h>
#include <mitkRenderingManager.h>
#include <mitkTimeNavigationController.h>
#include <mitkCameraController.h>
#include <mitkBaseRenderer.h>
#include <QmitkRenderWindow.h>

#include <vtkCamera.h>
#include <vtkRenderer.h>

#include <algorithm>
#include <limits>

#include <usModuleRegistry.h>
#include <usModule.h>
#include <usModuleContext.h>

#include <usModuleInitialization.h>

#include <berryPlatformUI.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>
#include <berryIEditorReference.h>
#include <mitkIRenderWindowPart.h>

#include <QBuffer>
#include <QIODevice>
#include <QPixmap>
#include <QWidget>

#include <stdexcept>

US_INITIALIZE_MODULE

namespace
{
  /**
   * @brief Find the StdMultiWidgetEditor and return it as an IRenderWindowPart.
   *
   * The StdMultiWidgetEditor is the authoritative source of the global crosshair
   * position. Other IRenderWindowPart implementations do not share this semantics.
   *
   * @return Pointer to IRenderWindowPart, or nullptr if not open.
   */
  mitk::IRenderWindowPart* GetStdMultiWidgetRenderWindowPart()
  {
    const auto workbenchWindows = berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows();
    for (const auto& window : workbenchWindows)
    {
      const auto page = window->GetActivePage();
      if (page.IsNull())
        continue;

      for (const auto& editorRef : page->GetEditorReferences())
      {
        if (editorRef->GetId() != "org.mitk.editors.stdmultiwidget")
          continue;

        // GetPart(false): do not restore/open the editor if it is not yet realized.
        const auto part = editorRef->GetPart(false);
        if (part.IsNull())
          continue;

        auto* const rwp = dynamic_cast<mitk::IRenderWindowPart*>(part.GetPointer());
        if (rwp != nullptr)
          return rwp;
      }
    }
    return nullptr;
  }

  /**
   * @brief Resolve the BaseRenderer for a given StdMulti window name.
   *
   * @throws mitk::RenderWindowBridgeNoEditorException if the editor is not open.
   * @throws mitk::RenderWindowBridgeUnknownWindowException if the name is unknown.
   */
  mitk::BaseRenderer* ResolveStdMultiRenderer(const std::string& windowName)
  {
    auto* const rwp = GetStdMultiWidgetRenderWindowPart();
    if (rwp == nullptr)
      throw mitk::RenderWindowBridgeNoEditorException(
        "StdMultiWidgetEditor is not open");

    auto* const qrw = rwp->GetQmitkRenderWindow(QString::fromStdString(windowName));
    if (qrw == nullptr)
      throw mitk::RenderWindowBridgeUnknownWindowException(windowName);

    auto* const renderer = qrw->GetRenderer();
    if (renderer == nullptr)
      throw mitk::RenderWindowBridgeUnknownWindowException(windowName);

    return renderer;
  }

  mitk::CameraController::StandardView StandardViewFromName(const std::string& v)
  {
    if (v == "anterior")  return mitk::CameraController::ANTERIOR;
    if (v == "posterior") return mitk::CameraController::POSTERIOR;
    if (v == "left")      return mitk::CameraController::SINISTER;
    if (v == "right")     return mitk::CameraController::DEXTER;
    if (v == "cranial")   return mitk::CameraController::CRANIAL;
    if (v == "caudal")    return mitk::CameraController::CAUDAL;
    // Controller validated the value before dispatch; unreachable if contract holds.
    throw std::runtime_error("Unknown standard_view: " + v);
  }

  void SetRenderWindowBridgeCallbacks(mitk::RenderWindowBridge* rwb)
  {
    rwb->SetScreenshotProvider(
      [](std::optional<std::pair<int, int>> size, mitk::ScreenshotFormat format) -> std::vector<unsigned char>
      {
        const auto workbenchWindows = berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows();
        if (workbenchWindows.isEmpty())
          throw std::runtime_error("No workbench window available for screenshot");

        QWidget* const w = static_cast<QWidget*>(workbenchWindows.first()->GetShell()->GetControl());
        if (w == nullptr)
          throw std::runtime_error("No workbench window widget available for screenshot");

        QPixmap px = w->grab();

        if (size.has_value())
          px = px.scaled(size->first, size->second, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        const char* const formatStr = (format == mitk::ScreenshotFormat::Jpeg) ? "JPEG" : "PNG";
        QByteArray bytes;
        QBuffer buf(&bytes);
        buf.open(QIODevice::WriteOnly);
        px.save(&buf, formatStr);

        return std::vector<unsigned char>(bytes.begin(), bytes.end());
      });

    rwb->SetPositionGetter(
      []() -> mitk::SelectedPositionInfo
      {
        auto* const rwp = GetStdMultiWidgetRenderWindowPart();
        if (rwp == nullptr)
          throw mitk::RenderWindowBridgeNoEditorException(
            "StdMultiWidgetEditor is not open — cannot read crosshair position");

        mitk::SelectedPositionInfo info;
        info.position = rwp->GetSelectedPosition();

        auto* const tnc = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
        if (tnc != nullptr)
        {
          const auto tg = tnc->GetInputWorldTimeGeometry();
          if (tg != nullptr)
          {
            const auto baseGeom = tg->GetGeometryForTimeStep(tnc->GetSelectedTimeStep());
            if (baseGeom.IsNotNull())
            {
              mitk::WorldBounds bounds;
              bounds.min.Fill(std::numeric_limits<double>::max());
              bounds.max.Fill(std::numeric_limits<double>::lowest());
              for (int cornerId = 0; cornerId < 8; ++cornerId)
              {
                const auto corner = baseGeom->GetCornerPoint(cornerId);
                for (int i = 0; i < 3; ++i)
                {
                  bounds.min[i] = std::min(bounds.min[i], corner[i]);
                  bounds.max[i] = std::max(bounds.max[i], corner[i]);
                }
              }
              info.bounds = bounds;
            }
          }
        }

        return info;
      });

    rwb->SetPositionSetter(
      [](const mitk::Point3D& pos)
      {
        auto* const rwp = GetStdMultiWidgetRenderWindowPart();
        if (rwp == nullptr)
          throw mitk::RenderWindowBridgeNoEditorException(
            "StdMultiWidgetEditor is not open — cannot set crosshair position");
        rwp->SetSelectedPosition(pos);
      });

    rwb->SetEditorListProvider(
      []() -> std::vector<mitk::EditorInfo>
      {
        // Two editor aliases are known up front. The MxN entry appears in the
        // list regardless of state so clients can discover it, but stays inactive
        // until WP3.
        mitk::EditorInfo stdmulti;
        stdmulti.alias = "stdmulti";
        stdmulti.pluginId = "org.mitk.editors.stdmultiwidget";

        if (auto* const rwp = GetStdMultiWidgetRenderWindowPart())
        {
          stdmulti.active = true;
          const auto hash = rwp->GetQmitkRenderWindows();
          stdmulti.windowNames.reserve(hash.size());
          for (auto it = hash.keyBegin(); it != hash.keyEnd(); ++it)
            stdmulti.windowNames.push_back(it->toStdString());
        }

        mitk::EditorInfo mxn;
        mxn.alias = "mxn";
        mxn.pluginId = "org.mitk.editors.mxnmultiwidget";
        // mxn.active stays false until WP3.

        return {stdmulti, mxn};
      });

    rwb->SetStdMultiCameraGetter(
      [](const std::string& windowName) -> mitk::CameraState
      {
        auto* const renderer = ResolveStdMultiRenderer(windowName);
        auto* const vtkRen = renderer->GetVtkRenderer();
        if (vtkRen == nullptr)
          throw std::runtime_error("vtkRenderer unavailable for window " + windowName);
        auto* const cam = vtkRen->GetActiveCamera();
        if (cam == nullptr)
          throw std::runtime_error("vtkCamera unavailable for window " + windowName);

        mitk::CameraState state;

        double pos[3];    cam->GetPosition(pos);
        double foc[3];    cam->GetFocalPoint(foc);
        double up[3];     cam->GetViewUp(up);
        for (int i = 0; i < 3; ++i)
        {
          state.position[i]   = pos[i];
          state.focalPoint[i] = foc[i];
          state.viewUp[i]     = up[i];
        }

        const bool is3d = (windowName == "3d");
        if (is3d)
          state.perspectiveAngle = cam->GetViewAngle();
        else
          state.parallelScale = cam->GetParallelScale();

        return state;
      });

    rwb->SetStdMultiCameraSetter(
      [](const std::string& windowName, const mitk::CameraPatch& patch)
      {
        auto* const renderer = ResolveStdMultiRenderer(windowName);
        auto* const cc = renderer->GetCameraController();
        auto* const vtkRen = renderer->GetVtkRenderer();
        if (cc == nullptr || vtkRen == nullptr)
          throw std::runtime_error("CameraController unavailable for window " + windowName);
        auto* const cam = vtkRen->GetActiveCamera();
        if (cam == nullptr)
          throw std::runtime_error("vtkCamera unavailable for window " + windowName);

        // Apply standard_view first so explicit position/focal/view_up/etc.
        // overrides win over it (concept §8).
        if (patch.standardView)
          cc->SetStandardView(StandardViewFromName(*patch.standardView));

        if (patch.position)
          cam->SetPosition((*patch.position)[0], (*patch.position)[1], (*patch.position)[2]);
        if (patch.focalPoint)
          cam->SetFocalPoint((*patch.focalPoint)[0], (*patch.focalPoint)[1], (*patch.focalPoint)[2]);
        if (patch.viewUp)
          cam->SetViewUp((*patch.viewUp)[0], (*patch.viewUp)[1], (*patch.viewUp)[2]);
        if (patch.parallelScale)
          cam->SetParallelScale(*patch.parallelScale);
        if (patch.perspectiveAngle)
          cam->SetViewAngle(*patch.perspectiveAngle);

        mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
      });

    rwb->SetStdMultiWindowListProvider(
      []() -> std::vector<mitk::WindowInfo>
      {
        auto* const rwp = GetStdMultiWidgetRenderWindowPart();
        if (rwp == nullptr)
          throw mitk::RenderWindowBridgeNoEditorException(
            "StdMultiWidgetEditor is not open — cannot list render windows");

        std::vector<mitk::WindowInfo> result;
        const auto hash = rwp->GetQmitkRenderWindows();
        result.reserve(hash.size());
        for (auto it = hash.keyBegin(); it != hash.keyEnd(); ++it)
        {
          mitk::WindowInfo wi;
          wi.name = it->toStdString();
          wi.kind = (wi.name == "3d") ? mitk::WindowKind::ThreeD : mitk::WindowKind::TwoD;
          result.push_back(wi);
        }
        return result;
      });

  }
}

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
        else
          m_RenderWindowBridge = nullptr; // service already gone, just clear the guard
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

    SetRenderWindowBridgeCallbacks(m_RenderWindowBridge);

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
