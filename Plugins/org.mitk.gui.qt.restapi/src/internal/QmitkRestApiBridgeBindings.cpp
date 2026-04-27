/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRestApiBridgeBindings.h"

#include <mitkRenderWindowBridge.h>
#include <mitkRenderingManager.h>
#include <mitkTimeNavigationController.h>
#include <mitkCameraController.h>
#include <mitkBaseRenderer.h>
#include <mitkSliceNavigationController.h>
#include <mitkStepper.h>
#include <mitkPlaneGeometry.h>
#include <mitkBaseGeometry.h>
#include <QmitkAbstractMultiWidget.h>
#include <QmitkAbstractMultiWidgetEditor.h>
#include <QmitkRenderWindow.h>

#include <vtkCamera.h>
#include <vtkRenderer.h>

#include <berryPlatformUI.h>
#include <berryIWorkbenchWindow.h>
#include <berryIWorkbenchPage.h>
#include <berryIEditorReference.h>
#include <mitkIRenderWindowPart.h>

#include <QBuffer>
#include <QIODevice>
#include <QImage>
#include <QPixmap>
#include <QWidget>

#include <algorithm>
#include <limits>
#include <stdexcept>

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

  /**
   * @brief Read the scene AABB from the TimeNavigationController's input world
   *        time geometry. Returns nullopt if no input geometry is available.
   */
  std::optional<mitk::WorldBounds> ReadSceneBoundsFromTnc()
  {
    auto* const tnc = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();
    if (tnc == nullptr) return std::nullopt;
    const auto tg = tnc->GetInputWorldTimeGeometry();
    if (tg == nullptr) return std::nullopt;
    const auto baseGeom = tg->GetGeometryForTimeStep(tnc->GetSelectedTimeStep());
    if (baseGeom.IsNull()) return std::nullopt;

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
    return bounds;
  }

  /**
   * @brief Encode a QPixmap to PNG/JPEG bytes, optionally scaling first.
   *
   * Shared by the editor and per-window screenshot providers. Scaling is done
   * on the captured image rather than by resizing the live render surface.
   */
  std::vector<unsigned char> EncodePixmap(
    QPixmap px,
    std::optional<std::pair<int, int>> size,
    mitk::ScreenshotFormat format)
  {
    if (size.has_value())
      px = px.scaled(size->first, size->second, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    const char* const formatStr = (format == mitk::ScreenshotFormat::Jpeg) ? "JPEG" : "PNG";
    QByteArray bytes;
    QBuffer buf(&bytes);
    buf.open(QIODevice::WriteOnly);
    if (!px.save(&buf, formatStr))
      throw std::runtime_error(std::string("Failed to encode screenshot as ") + formatStr);
    return std::vector<unsigned char>(bytes.begin(), bytes.end());
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
}

namespace mitk
{
  void ConfigureRestApiBridgeCallbacks(RenderWindowBridge* rwb)
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

        return EncodePixmap(w->grab(), size, format);
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
        info.bounds = ReadSceneBoundsFromTnc();
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

    rwb->SetStdMultiEditorScreenshotProvider(
      [](std::optional<std::pair<int, int>> size, mitk::ScreenshotFormat format) -> std::vector<unsigned char>
      {
        auto* const rwp = GetStdMultiWidgetRenderWindowPart();
        if (rwp == nullptr)
          throw mitk::RenderWindowBridgeNoEditorException(
            "StdMultiWidgetEditor is not open — cannot capture editor screenshot");

        // Ask the editor for its multi-widget directly. Walking up the parent
        // chain of an active QmitkRenderWindow is unreliable: the render window
        // is hosted by an intermediate QmitkRenderWindowWidget, not by the
        // QmitkAbstractMultiWidget itself.
        auto* const editor = dynamic_cast<QmitkAbstractMultiWidgetEditor*>(rwp);
        auto* const canvas = (editor != nullptr) ? editor->GetMultiWidget() : nullptr;
        if (canvas == nullptr)
          throw std::runtime_error(
            "Unexpected editor type — cannot locate QmitkAbstractMultiWidget canvas");

        return EncodePixmap(canvas->grab(), size, format);
      });

    rwb->SetStdMultiWindowScreenshotProvider(
      [](const std::string& windowName,
         std::optional<std::pair<int, int>> size,
         mitk::ScreenshotFormat format) -> std::vector<unsigned char>
      {
        auto* const rwp = GetStdMultiWidgetRenderWindowPart();
        if (rwp == nullptr)
          throw mitk::RenderWindowBridgeNoEditorException(
            "StdMultiWidgetEditor is not open");

        auto* const qrw = rwp->GetQmitkRenderWindow(QString::fromStdString(windowName));
        if (qrw == nullptr)
          throw mitk::RenderWindowBridgeUnknownWindowException(windowName);

        // QmitkRenderWindow is a QVTKOpenGLNativeWidget (QOpenGLWidget). Capturing
        // via grabFramebuffer() reads the current OpenGL framebuffer without
        // resizing the live render window; we scale the resulting image
        // afterwards if a different size was requested.
        QImage img = qrw->grabFramebuffer();
        return EncodePixmap(QPixmap::fromImage(std::move(img)), size, format);
      });

    rwb->SetEditorListProvider(
      []() -> std::vector<mitk::EditorInfo>
      {
        // Two editor aliases are known up front. The MxN entry appears in the
        // list regardless of state so clients can discover it, but its
        // activation is not yet implemented (always inactive).
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
        // mxn.active stays false; activation is not yet implemented.

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
        // overrides win over it.
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

    rwb->SetStdMultiSelectedSliceGetter(
      [](const std::string& windowName) -> mitk::SliceState
      {
        if (windowName == "3d")
          throw mitk::RenderWindowBridgeUnsupportedOperationException(
            "selected-slice is not applicable to the 3D window");

        auto* const renderer = ResolveStdMultiRenderer(windowName);
        auto* const snc = renderer->GetSliceNavigationController();
        if (snc == nullptr)
          throw std::runtime_error("SliceNavigationController unavailable for " + windowName);
        auto* const stepper = snc->GetStepper();
        if (stepper == nullptr)
          throw std::runtime_error("Stepper unavailable for " + windowName);

        mitk::SliceState state;
        state.step = stepper->GetPos();
        state.bounds.steps = stepper->GetSteps();

        if (const auto* const plane = snc->GetCurrentPlaneGeometry())
          state.position = plane->GetCenter();

        if (const auto b = ReadSceneBoundsFromTnc())
        {
          state.bounds.minPosition = b->min;
          state.bounds.maxPosition = b->max;
          state.bounds.hasPositions = true;
        }
        return state;
      });

    rwb->SetStdMultiSelectedSliceStepSetter(
      [](const std::string& windowName, unsigned int step)
      {
        if (windowName == "3d")
          throw mitk::RenderWindowBridgeUnsupportedOperationException(
            "selected-slice is not applicable to the 3D window");

        auto* const renderer = ResolveStdMultiRenderer(windowName);
        auto* const snc = renderer->GetSliceNavigationController();
        if (snc == nullptr)
          throw std::runtime_error("SliceNavigationController unavailable for " + windowName);
        auto* const stepper = snc->GetStepper();
        if (stepper == nullptr)
          throw std::runtime_error("Stepper unavailable for " + windowName);

        stepper->SetPos(step);
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
