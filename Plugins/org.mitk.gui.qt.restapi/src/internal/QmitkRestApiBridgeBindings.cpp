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
#include <QmitkMxNMultiWidget.h>
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
   * v2 layout schema enforces 2D-only MxN cells: its view_direction enum has
   * no "3d" value, so by construction every cell loaded from a v2 document
   * is 2D. The MxN window list provider, MxN camera getter, and the
   * controller's PUT camera handler all assume this and hard-code the
   * 2D path.
   *
   * v3 migration checklist (when 3D MxN cells are introduced):
   *   1. Replace this constant with per-cell `kind` reads from
   *      QmitkMxNMultiWidget::WindowDescriptor.
   *   2. Update the MxN window list provider (this file) so `MxNWindowInfo::kind`
   *      reflects the descriptor instead of \c V2_MXN_WINDOW_KIND.
   *   3. Update \c SetMxNCameraGetter to derive `is3d` from the descriptor
   *      rather than from this constant.
   *   4. Update the controller's PUT /editors/mxn/windows/{id}/camera handler
   *      (mitkRenderingController.cpp) to derive `is3d` from the windows-list
   *      provider's kind before calling ParseCameraPatch.
   *   5. Update the OpenAPI schema and the layout v3 schema to expose 3D cells.
   *   6. (Performance) Add per-cell descriptor getters
   *      (e.g. \c GetMxNWindowDescriptor / \c GetStdMultiWindowDescriptor)
   *      to the bridge surface so \c HandleGET_mxnWindow and
   *      \c HandleGET_stdmultiWindow can resolve a single window without
   *      materialising the full list and scanning it linearly. Negligible at
   *      v1.2 scale; relevant once MxN cell counts grow.
   *
   * The static_assert below pins the invariant at compile time: changing the
   * constant without removing the reference sites will fail to build.
   */
  constexpr mitk::WindowKind V2_MXN_WINDOW_KIND = mitk::WindowKind::TwoD;
  static_assert(V2_MXN_WINDOW_KIND == mitk::WindowKind::TwoD,
                "V2_MXN_WINDOW_KIND must remain TwoD until the v3 migration "
                "checklist above is completed; otherwise the MxN camera/kind "
                "hard-coded paths would silently desynchronize.");

  /**
   * \brief Find the editor with the given Berry editor id and return its
   *        IRenderWindowPart, or nullptr if not currently open.
   *
   * GetPart(false): do not restore/open the editor if it is not yet realized.
   */
  mitk::IRenderWindowPart* FindRenderWindowPartByEditorId(const char* editorId)
  {
    const auto workbenchWindows = berry::PlatformUI::GetWorkbench()->GetWorkbenchWindows();
    for (const auto& window : workbenchWindows)
    {
      const auto page = window->GetActivePage();
      if (page.IsNull())
        continue;

      for (const auto& editorRef : page->GetEditorReferences())
      {
        if (editorRef->GetId() != editorId)
          continue;

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
   * \brief Find the StdMultiWidgetEditor and return it as an IRenderWindowPart.
   *
   * The StdMultiWidgetEditor is the authoritative source of the global crosshair
   * position. Other IRenderWindowPart implementations do not share this semantics.
   *
   * \return Pointer to IRenderWindowPart, or nullptr if not open.
   */
  mitk::IRenderWindowPart* GetStdMultiWidgetRenderWindowPart()
  {
    return FindRenderWindowPartByEditorId("org.mitk.editors.stdmultiwidget");
  }

  /**
   * \brief Find the MxNMultiWidgetEditor and return it as an IRenderWindowPart.
   *
   * \return Pointer to IRenderWindowPart, or nullptr if not open.
   */
  mitk::IRenderWindowPart* GetMxNMultiWidgetRenderWindowPart()
  {
    return FindRenderWindowPartByEditorId("org.mitk.editors.mxnmultiwidget");
  }

  /**
   * \brief Pair of the MxN editor's render-window part and multi-widget, both
   *        guaranteed non-null.
   */
  struct MxNEditor
  {
    mitk::IRenderWindowPart* rwp;
    QmitkMxNMultiWidget*     multiWidget;
  };

  /**
   * \brief Resolve the live MxN editor in a single workbench probe.
   *
   * Mirrors the global selected-position pattern: probe the workbench for the
   * editor part once, then dynamic_cast through the abstract editor base to
   * the concrete multi-widget. Both steps must succeed; either failure
   * surfaces as RenderWindowBridgeNoEditorException so the controller maps it
   * to 503 EDITOR_NOT_ACTIVE.
   *
   * \throws mitk::RenderWindowBridgeNoEditorException if the MxN editor is
   *         not open or has no live multi-widget.
   */
  MxNEditor ResolveMxNEditor()
  {
    auto* const rwp = GetMxNMultiWidgetRenderWindowPart();
    if (rwp == nullptr)
      throw mitk::RenderWindowBridgeNoEditorException(
        "MxNMultiWidgetEditor is not open");

    auto* const editor = dynamic_cast<QmitkAbstractMultiWidgetEditor*>(rwp);
    auto* const multiWidget = (editor != nullptr)
      ? dynamic_cast<QmitkMxNMultiWidget*>(editor->GetMultiWidget())
      : nullptr;
    if (multiWidget == nullptr)
      throw mitk::RenderWindowBridgeNoEditorException(
        "MxN editor has no live multi-widget");

    return { rwp, multiWidget };
  }

  /**
   * \brief Resolve the live MxN editor's QmitkMxNMultiWidget pointer.
   *
   * Convenience over ResolveMxNEditor for callers that only need the widget.
   *
   * \throws mitk::RenderWindowBridgeNoEditorException if the MxN editor is
   *         not open or has no live multi-widget.
   */
  QmitkMxNMultiWidget* GetMxNMultiWidget()
  {
    return ResolveMxNEditor().multiWidget;
  }

  /**
   * \brief Atomic snapshot of an MxN cell lookup.
   *
   * Captures all four fields in one UI-thread pass so subsequent code acts
   * on a consistent view even if the editor changes mid-call.
   */
  struct MxNRenderWindowSnapshot
  {
    mitk::IRenderWindowPart* rwp;
    QmitkMxNMultiWidget*     multiWidget;
    QString                  id;
    QmitkRenderWindow*       renderWindow;
  };

  /**
   * \brief Build an MxNRenderWindowSnapshot for a given window id.
   *
   * \throws mitk::RenderWindowBridgeNoEditorException     if the MxN editor is not open.
   * \throws mitk::RenderWindowBridgeUnknownWindowException if the id does not
   *         resolve to a live cell in the current layout.
   */
  MxNRenderWindowSnapshot ResolveMxNRenderWindow(const std::string& id)
  {
    const auto editor = ResolveMxNEditor();
    const auto qid = QString::fromStdString(id);
    auto* const qrw = editor.rwp->GetQmitkRenderWindow(qid);
    if (qrw == nullptr)
      throw mitk::RenderWindowBridgeUnknownWindowException(id);

    return { editor.rwp, editor.multiWidget, qid, qrw };
  }

  /**
   * \brief Resolve an MxN cell id to its BaseRenderer.
   *
   * Convenience over ResolveMxNRenderWindow for camera / slice handlers
   * that only need the renderer.
   *
   * \throws same as ResolveMxNRenderWindow plus
   *         mitk::RenderWindowBridgeRendererUnavailableException if the
   *         render window has no associated renderer (server-side
   *         inconsistency). The typed exception is symmetric to
   *         ResolveStdMultiRenderer below and maps to 500
   *         RENDERER_UNAVAILABLE in the controller instead of being
   *         lumped into the generic 500 INTERNAL_ERROR bucket.
   */
  mitk::BaseRenderer* ResolveMxNRenderer(const std::string& id)
  {
    const auto snap = ResolveMxNRenderWindow(id);
    auto* const renderer = snap.renderWindow->GetRenderer();
    if (renderer == nullptr)
      throw mitk::RenderWindowBridgeRendererUnavailableException(id);
    return renderer;
  }

  /**
   * \brief Resolve the BaseRenderer for a given StdMulti window name.
   *
   * \throws mitk::RenderWindowBridgeNoEditorException if the editor is not open.
   * \throws mitk::RenderWindowBridgeUnknownWindowException if the name is unknown.
   * \throws mitk::RenderWindowBridgeRendererUnavailableException if the window
   *         exists but its renderer is currently null (server-side inconsistency).
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
      throw mitk::RenderWindowBridgeRendererUnavailableException(windowName);

    return renderer;
  }

  /**
   * \brief Read the scene AABB from the TimeNavigationController's input world
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
    bounds.minPosition.Fill(std::numeric_limits<double>::max());
    bounds.maxPosition.Fill(std::numeric_limits<double>::lowest());
    for (int cornerId = 0; cornerId < 8; ++cornerId)
    {
      const auto corner = baseGeom->GetCornerPoint(cornerId);
      for (int i = 0; i < 3; ++i)
      {
        bounds.minPosition[i] = std::min(bounds.minPosition[i], corner[i]);
        bounds.maxPosition[i] = std::max(bounds.maxPosition[i], corner[i]);
      }
    }
    return bounds;
  }

  /**
   * \brief Encode a QPixmap to PNG/JPEG bytes, optionally scaling first.
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

  /**
   * \brief Read the vtkCamera state of a renderer into a plain CameraState.
   *
   * Shared between StdMulti and MxN camera getters: both editors expose
   * the same vtkCamera surface; the only difference is whether the cell
   * is 2D or 3D, which the caller passes in.
   */
  mitk::CameraState ReadCameraStateFromRenderer(mitk::BaseRenderer* renderer, bool is3d, const std::string& windowName)
  {
    auto* const vtkRen = renderer->GetVtkRenderer();
    if (vtkRen == nullptr)
      throw std::runtime_error("vtkRenderer unavailable for window " + windowName);
    auto* const cam = vtkRen->GetActiveCamera();
    if (cam == nullptr)
      throw std::runtime_error("vtkCamera unavailable for window " + windowName);

    mitk::CameraState state;
    double pos[3]; cam->GetPosition(pos);
    double foc[3]; cam->GetFocalPoint(foc);
    double up[3];  cam->GetViewUp(up);
    for (int i = 0; i < 3; ++i)
    {
      state.position[i]   = pos[i];
      state.focalPoint[i] = foc[i];
      state.viewUp[i]     = up[i];
    }
    if (is3d) state.perspectiveAngle = cam->GetViewAngle();
    else      state.parallelScale    = cam->GetParallelScale();
    return state;
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

  /**
   * \brief Apply a CameraPatch to a renderer's vtkCamera.
   *
   * standard_view is applied first so explicit pose fields override its
   * effect (matches the rule the controller's ParseCameraPatch contract
   * documents).
   */
  void ApplyCameraPatchToRenderer(mitk::BaseRenderer* renderer, const mitk::CameraPatch& patch, const std::string& windowName)
  {
    auto* const cc = renderer->GetCameraController();
    auto* const vtkRen = renderer->GetVtkRenderer();
    if (cc == nullptr || vtkRen == nullptr)
      throw std::runtime_error("CameraController unavailable for window " + windowName);
    auto* const cam = vtkRen->GetActiveCamera();
    if (cam == nullptr)
      throw std::runtime_error("vtkCamera unavailable for window " + windowName);

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
            "StdMultiWidgetEditor is not open -- cannot read crosshair position");

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
            "StdMultiWidgetEditor is not open -- cannot set crosshair position");
        rwp->SetSelectedPosition(pos);
      });

    rwb->SetStdMultiEditorScreenshotProvider(
      [](std::optional<std::pair<int, int>> size, mitk::ScreenshotFormat format) -> std::vector<unsigned char>
      {
        auto* const rwp = GetStdMultiWidgetRenderWindowPart();
        if (rwp == nullptr)
          throw mitk::RenderWindowBridgeNoEditorException(
            "StdMultiWidgetEditor is not open -- cannot capture editor screenshot");

        // Ask the editor for its multi-widget directly. Walking up the parent
        // chain of an active QmitkRenderWindow is unreliable: the render window
        // is hosted by an intermediate QmitkRenderWindowWidget, not by the
        // QmitkAbstractMultiWidget itself.
        auto* const editor = dynamic_cast<QmitkAbstractMultiWidgetEditor*>(rwp);
        auto* const canvas = (editor != nullptr) ? editor->GetMultiWidget() : nullptr;
        if (canvas == nullptr)
          throw mitk::RenderWindowBridgeUnsupportedOperationException(
            "Active editor does not expose a QmitkAbstractMultiWidget canvas");

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
        // Two editor aliases are known up front. Both appear in the list
        // regardless of state so clients can discover them; the `active` flag
        // and the `windowIds` list are populated from a live workbench
        // probe.
        mitk::EditorInfo stdmulti;
        stdmulti.alias = "stdmulti";
        stdmulti.pluginId = "org.mitk.editors.stdmultiwidget";

        if (auto* const rwp = GetStdMultiWidgetRenderWindowPart())
        {
          stdmulti.active = true;
          // StdMulti registers windows under bare ids ("axial", "sagittal",
          // "coronal", "3d") -- no prefix translation needed.
          const auto hash = rwp->GetQmitkRenderWindows();
          stdmulti.windowIds.reserve(hash.size());
          for (auto it = hash.keyBegin(); it != hash.keyEnd(); ++it)
            stdmulti.windowIds.push_back(it->toStdString());
        }

        mitk::EditorInfo mxn;
        mxn.alias = "mxn";
        mxn.pluginId = "org.mitk.editors.mxnmultiwidget";

        try
        {
          auto* const widget = GetMxNMultiWidget();
          mxn.active = true;
          // Source the cell list from ListWindowDescriptors so the order
          // matches GET /rendering/editors/mxn/windows (deterministic
          // pre-order splitter walk). Already canonical fully-qualified
          // ids; no prefix translation needed.
          const auto descriptors = widget->ListWindowDescriptors();
          mxn.windowIds.reserve(descriptors.size());
          for (const auto& descriptor : descriptors)
            mxn.windowIds.push_back(descriptor.id.toStdString());
        }
        catch (const mitk::RenderWindowBridgeNoEditorException&)
        {
          // MxN editor not open -- mxn.active stays false, windowIds empty.
        }

        return {stdmulti, mxn};
      });

    rwb->SetStdMultiCameraGetter(
      [](const std::string& windowName) -> mitk::CameraState
      {
        auto* const renderer = ResolveStdMultiRenderer(windowName);
        const bool is3d = (windowName == "3d");
        return ReadCameraStateFromRenderer(renderer, is3d, windowName);
      });

    rwb->SetStdMultiCameraSetter(
      [](const std::string& windowName, const mitk::CameraPatch& patch)
      {
        auto* const renderer = ResolveStdMultiRenderer(windowName);
        ApplyCameraPatchToRenderer(renderer, patch, windowName);
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
          state.bounds.minPosition = b->minPosition;
          state.bounds.maxPosition = b->maxPosition;
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
            "StdMultiWidgetEditor is not open -- cannot list render windows");

        std::vector<mitk::WindowInfo> result;
        const auto hash = rwp->GetQmitkRenderWindows();
        result.reserve(hash.size());
        for (auto it = hash.keyBegin(); it != hash.keyEnd(); ++it)
        {
          mitk::WindowInfo wi;
          wi.id = it->toStdString();
          // StdMulti slot semantics: id encodes the plane verbatim for the
          // three 2D slots; the "3d" slot has no anatomical plane.
          if (wi.id == "3d")
          {
            wi.kind = mitk::WindowKind::ThreeD;
          }
          else
          {
            wi.kind = mitk::WindowKind::TwoD;
            wi.viewDirection = mitk::ParseV2ViewDirection(wi.id);
          }
          result.push_back(wi);
        }
        return result;
      });

    // ----- MxN editor -----

    rwb->SetMxNLayoutGetter(
      []() -> std::string
      {
        // SerializeLayout returns nlohmann::json; the bridge boundary is a
        // plain std::string to keep nlohmann out of mitkRenderWindowBridge.h.
        // .dump() is on the engine side; the controller treats the result
        // as an opaque application/json body.
        auto* const widget = GetMxNMultiWidget();
        return widget->SerializeLayout().dump();
      });

    rwb->SetMxNLayoutSetter(
      [](const std::string& body) -> std::string
      {
        // Parse here, not in the bridge -- the controller has already
        // verified the body is valid JSON syntax, but we must still get
        // an nlohmann::json into ApplyLayout. Schema / structural failures
        // surface as mitk::Exception, which the controller catches locally
        // and maps to 400 INVALID_REQUEST.
        auto* const widget = GetMxNMultiWidget();
        nlohmann::json doc;
        try
        {
          doc = nlohmann::json::parse(body);
        }
        catch (const nlohmann::json::exception& e)
        {
          // The controller pre-parses, so reaching this branch means the
          // serialized body diverged from the original. Surface as a
          // document-shape failure (mitk::Exception) so the controller
          // still maps to 400; the wording carries the parse detail.
          mitkThrow() << "Layout JSON parse error: " << e.what();
        }
        widget->ApplyLayout(doc);
        return widget->SerializeLayout().dump();
      });

    rwb->SetMxNCameraGetter(
      [](const std::string& id) -> mitk::CameraState
      {
        constexpr bool is3d = (V2_MXN_WINDOW_KIND == mitk::WindowKind::ThreeD);
        return ReadCameraStateFromRenderer(
          ResolveMxNRenderer(id), is3d, id);
      });

    rwb->SetMxNCameraSetter(
      [](const std::string& id, const mitk::CameraPatch& patch)
      {
        ApplyCameraPatchToRenderer(ResolveMxNRenderer(id), patch, id);
      });

    rwb->SetMxNSelectedSliceGetter(
      [](const std::string& id) -> mitk::SliceState
      {
        // Defensive: v2 has no 3D MxN cell; if a v3 3D cell ever shows up here,
        // surface as UNSUPPORTED_OPERATION so the controller maps to 404.
        // (Matches the StdMulti pattern for /selected-slice on the 3D window.)
        auto* const renderer = ResolveMxNRenderer(id);
        auto* const snc = renderer->GetSliceNavigationController();
        if (snc == nullptr)
          throw std::runtime_error("SliceNavigationController unavailable for MxN cell '" + id + "'");
        auto* const stepper = snc->GetStepper();
        if (stepper == nullptr)
          throw std::runtime_error("Stepper unavailable for MxN cell '" + id + "'");

        mitk::SliceState state;
        state.step = stepper->GetPos();
        state.bounds.steps = stepper->GetSteps();

        if (const auto* const plane = snc->GetCurrentPlaneGeometry())
          state.position = plane->GetCenter();

        if (const auto b = ReadSceneBoundsFromTnc())
        {
          state.bounds.minPosition = b->minPosition;
          state.bounds.maxPosition = b->maxPosition;
          state.bounds.hasPositions = true;
        }
        return state;
      });

    rwb->SetMxNSelectedSliceStepSetter(
      [](const std::string& id, unsigned int step)
      {
        auto* const renderer = ResolveMxNRenderer(id);
        auto* const snc = renderer->GetSliceNavigationController();
        if (snc == nullptr)
          throw std::runtime_error("SliceNavigationController unavailable for MxN cell '" + id + "'");
        auto* const stepper = snc->GetStepper();
        if (stepper == nullptr)
          throw std::runtime_error("Stepper unavailable for MxN cell '" + id + "'");

        stepper->SetPos(step);
      });

    rwb->SetMxNSelectedPositionGetter(
      [](const std::string& id) -> mitk::SelectedPositionInfo
      {
        const auto snap = ResolveMxNRenderWindow(id);

        mitk::SelectedPositionInfo info;
        info.position = snap.multiWidget->GetSelectedPosition(snap.id);
        info.bounds = ReadSceneBoundsFromTnc();
        return info;
      });

    rwb->SetMxNSelectedPositionSetter(
      [](const std::string& id, const mitk::Point3D& position)
      {
        const auto snap = ResolveMxNRenderWindow(id);
        snap.multiWidget->SetSelectedPosition(position, snap.id);
      });

    rwb->SetMxNEditorScreenshotProvider(
      [](std::optional<std::pair<int, int>> size, mitk::ScreenshotFormat format) -> std::vector<unsigned char>
      {
        // Resolve the live MxN multi-widget and grab its canvas. Same encoding
        // pipeline as the StdMulti editor screenshot -- the cell-tree QSplitter
        // root that hosts the cells IS the multi-widget itself, so a direct
        // QWidget::grab on the multi-widget pointer yields the editor canvas.
        auto* const widget = GetMxNMultiWidget();
        return EncodePixmap(widget->grab(), size, format);
      });

    rwb->SetMxNWindowScreenshotProvider(
      [](const std::string& id,
         std::optional<std::pair<int, int>> size,
         mitk::ScreenshotFormat format) -> std::vector<unsigned char>
      {
        // Mirror the StdMulti per-window approach: grabFramebuffer on the
        // QmitkRenderWindow (a QVTKOpenGLNativeWidget / QOpenGLWidget) reads
        // the live OpenGL framebuffer without resizing the surface. Scaling,
        // if requested, is applied to the captured image afterwards.
        const auto snap = ResolveMxNRenderWindow(id);
        QImage img = snap.renderWindow->grabFramebuffer();
        return EncodePixmap(QPixmap::fromImage(std::move(img)), size, format);
      });

    rwb->SetMxNWindowListProvider(
      []() -> std::vector<mitk::MxNWindowInfo>
      {
        // Goes through the engine's public ListWindowDescriptors() query so
        // per-cell field values match what GET /editors/mxn/layout would
        // emit for the same state. Per-cell descriptor logic lives in
        // QmitkMxNMultiWidget::MakeWindowDescriptor; the bindings translate
        // to the bridge's plain-data MxNWindowInfo shape only.
        auto* const widget = GetMxNMultiWidget();
        const auto descriptors = widget->ListWindowDescriptors();

        std::vector<mitk::MxNWindowInfo> result;
        result.reserve(descriptors.size());
        for (const auto& d : descriptors)
        {
          mitk::MxNWindowInfo wi;
          wi.id = d.id.toStdString();
          if (!d.displayName.isEmpty())
          {
            wi.displayName = d.displayName.toStdString();
          }
          wi.kind = V2_MXN_WINDOW_KIND;
          // v2 layout schema requires `view_direction`; v3 may relax this when
          // 3D cells / tilted custom orientations are introduced -- in that
          // case leave `viewDirection` as nullopt.
          //
          // ParseV2ViewDirection throws std::invalid_argument on an unknown
          // token. The descriptor is the engine's serialized layout state, so
          // a bad token signals corrupt/out-of-spec layout state rather than
          // an internal failure. Rethrow as mitk::Exception so the controller
          // maps it to 422 RENDERING_ERROR (the same channel layout-apply
          // engine errors travel through), instead of falling through to the
          // misleading 500 INTERNAL_ERROR catch-all.
          try
          {
            wi.viewDirection = mitk::ParseV2ViewDirection(d.viewDirection.toStdString());
          }
          catch (const std::invalid_argument& e)
          {
            mitkThrow() << "MxN window list: invalid view_direction in cell '"
                        << wi.id << "': " << e.what();
          }
          wi.selectionGroup = d.selectionGroup.toStdString();
          result.push_back(wi);
        }
        return result;
      });
  }
}
