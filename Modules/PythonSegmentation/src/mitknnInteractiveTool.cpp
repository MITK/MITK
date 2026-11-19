/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveTool.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitknnInteractiveBoundingBox.h>
#include <mitknnInteractiveBoxInteractor.h>
#include <mitknnInteractiveLassoInteractor.h>
#include <mitknnInteractivePointInteractor.h>
#include <mitknnInteractiveScribbleInteractor.h>
#include <mitkPlanarFigure.h>
#include <mitkProgressTask.h>
#include <mitkPythonHelper.h>
#include <mitkPythonWorker.h>
#include <mitkRenderingManager.h>
#include <mitkSharedBuffer.h>
#include <mitkToolManager.h>
#include <mitkTorchDevice.h>

#include "mitkPythonWorkerModules.h"
#include "mitkStopHook.h"
#include "mitknnInteractiveBoundingBoxHelpers.h"

#include <usGetModuleContext.h>
#include <usModuleResource.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <functional>
#include <limits>

using namespace mitk::nnInteractive;

namespace
{
  // Every point of a scribble is stamped into the slice on the GUI thread.
  constexpr std::size_t MAX_STROKE_PROMPT_POINTS = 100;

  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }

  // Strip credentials (a "user:pass@" userinfo) and any query/fragment from a URL
  // so it can be safely shown in error messages and written to the log. The real
  // URL is still used for the connection; this is purely for display. Best-effort
  // string surgery, deliberately not a full URL parser.
  std::string SanitizeUrlForDisplay(const std::string& url)
  {
    std::string result = url;

    // Drop query and fragment (an API key is sometimes pasted as ?key=...).
    if (const auto cut = result.find_first_of("?#"); cut != std::string::npos)
      result.erase(cut);

    // Drop userinfo: anything between "://" and the next "@" within the authority.
    if (const auto schemeEnd = result.find("://"); schemeEnd != std::string::npos)
    {
      const auto authStart = schemeEnd + 3;
      const auto authEnd = result.find('/', authStart);
      const auto at = result.find('@', authStart);

      if (at != std::string::npos && (authEnd == std::string::npos || at < authEnd))
        result.erase(authStart, at - authStart + 1);
    }

    return result;
  }

  // The worker module raises it for every error that means the remote lease is
  // gone (see nninteractive_worker.py).
  constexpr auto REMOTE_CONNECTION_LOST = "RemoteConnectionLost";

  bool IsRemoteConnectionLost(const mitk::Exception& e)
  {
    const auto* error = dynamic_cast<const mitk::PythonWorkerError*>(&e);
    return error != nullptr && error->GetPythonType() == REMOTE_CONNECTION_LOST;
  }

  std::optional<mitk::nnInteractiveTool::SessionLoss::Cause> GetSessionLossCause(const mitk::Exception& e)
  {
    if (IsRemoteConnectionLost(e))
      return mitk::nnInteractiveTool::SessionLoss::Cause::RemoteConnection;

    if (dynamic_cast<const mitk::PythonWorkerExitedException*>(&e) != nullptr)
      return mitk::nnInteractiveTool::SessionLoss::Cause::WorkerExited;

    return std::nullopt;
  }

  // Whoever catches a rethrown error reports only its description, which names
  // the exception but not where it came from.
  void LogPythonTraceback(const mitk::Exception& e)
  {
    if (const auto* error = dynamic_cast<const mitk::PythonWorkerError*>(&e))
      MITK_ERROR << "nnInteractive: " << error->GetTraceback();
  }

  nlohmann::json ToJson(const mitk::nnInteractive::InteractionBoundingBox& boundingBox)
  {
    return {
      { boundingBox[0][0], boundingBox[0][1] },
      { boundingBox[1][0], boundingBox[1][1] },
      { boundingBox[2][0], boundingBox[2][1] }
    };
  }
}

namespace mitk
{
  MITK_TOOL_MACRO(MITKPYTHONSEGMENTATION_EXPORT, nnInteractiveTool, "nnInteractive")

  class nnInteractiveTool::Impl
  {
  public:
    Impl()
      : PromptType(PromptType::Positive),
        AutoZoom(true),
        AutoRefine(false)
    {
      this->Interactors[InteractionType::Point] = std::make_unique<PointInteractor>();
      this->Interactors[InteractionType::Box] = std::make_unique<BoxInteractor>();
      this->Interactors[InteractionType::Scribble] = std::make_unique<ScribbleInteractor>();
      this->Interactors[InteractionType::Lasso] = std::make_unique<LassoInteractor>();
    }

    ~Impl()
    {
    }

    Interactor* GetEnabledInteractor() const
    {
      for (auto& [interactionType, interactor] : this->Interactors)
      {
        if (interactor->IsEnabled())
          return interactor.get();
      }

      return nullptr;
    }

    std::optional<Backend>& GetBackend()
    {
      return m_Backend;
    }

    void SetBackend(Backend backend)
    {
      m_Backend = backend;
    }

    void ResetBackend()
    {
      m_Backend.reset();
    }

    /** A prompt of AddPrompt(), from geometry instead of an interactor. */
    struct GeometricPrompt
    {
      nnInteractive::InteractionType Type;
      nnInteractive::PromptType PromptType;
      std::vector<Point3D> Points;
      PlaneGeometry::ConstPointer Plane;
    };

    /** What the session gets for a prompt, prepared on the GUI thread. */
    struct PromptCall
    {
      std::string Method;
      nlohmann::json Params;
      std::shared_ptr<SharedBuffer> Mask;
    };

    // Methods that call the worker or prepare its calls are defined at the
    // bottom of this file.
    void SetAutoZoom() const;
    nlohmann::json GetPointParams(const Point3D& point, const Image* inputAtTimeStep, bool include) const;
    nlohmann::json GetBoxParams(const Point3D& corner, const Point3D& oppositeCorner, const Image* inputAtTimeStep, bool include) const;
    nlohmann::json GetMaskParams(const InteractionBoundingBox* boundingBox, bool include) const;
    PromptCall DrawPrompt(const GeometricPrompt& prompt, const Image* inputAtTimeStep);
    void ResetInteractions() const;
    bool Undo() const;

    void SetCapabilities(const nlohmann::json& capabilities)
    {
      const auto& interactions = capabilities["interactions"];

      this->Interactions.Point = interactions.value("point", true);
      this->Interactions.Box = interactions.value("box", true);
      this->Interactions.Scribble = interactions.value("scribble", true);
      this->Interactions.Lasso = interactions.value("lasso", true);
      this->Interactions.Mask = interactions.value("mask", true);
      this->License = capabilities.value("license", std::string());
      this->SupportsUndo = capabilities.value("undo", false);
    }

    bool HasUndoableInteraction() const
    {
      return LastInteractionType.has_value() || LastInteractionWasMask;
    }

    void ClearLastInteraction()
    {
      LastInteractionType.reset();
      LastInteractionPromptType.reset();
      LastInteractionWasMask = false;
      LastInteractionByProgram = false;
    }

    void ClearInteractions()
    {
      for (auto& [interactionType, interactor] : this->Interactors)
        interactor->Reset();

      this->InitialSeg = nullptr;
      this->ClearLastInteraction();
    }

    std::unique_ptr<PythonWorker> Worker;
    bool SessionStarted = false;
    SupportedInteractions Interactions;
    std::string License;
    bool SupportsUndo = false;

    // The session writes its predictions into this shared memory. TargetBuffer
    // is an image on the same memory, without a copy.
    std::shared_ptr<SharedBuffer> TargetSegment;
    Image::Pointer TargetBuffer;

    nnInteractive::PromptType PromptType;
    InteractorMap Interactors;
    Image::Pointer InitialSeg;
    bool AutoZoom;
    bool AutoRefine;
    bool Remote = false;
    TimeStepType SessionReferenceDataTimeStep = 0;
    TimeStepType SessionWorkingDataTimeStep = 0;

    // Single-level undo bookkeeping. nnInteractive can undo only the last
    // interaction, so one record of what it was (a prompt, or the
    // initial-segmentation mask) is enough to remove the right visualization
    // on undo. A prompt is shown by the interactor of its type, whether it
    // came from there or from AddPrompt(). UndoRefreshPending makes
    // DoUpdatePreview repaint the preview from the restored target buffer
    // instead of adding an interaction.
    std::optional<nnInteractive::InteractionType> LastInteractionType;
    std::optional<nnInteractive::PromptType> LastInteractionPromptType;
    bool LastInteractionWasMask = false;
    bool LastInteractionByProgram = false;
    bool UndoRefreshPending = false;

    // The prompt of AddPrompt() for DoUpdatePreview(), what became of it, and
    // who waits for that. UpdatePreview() reports errors as messages to the
    // user instead of throwing them, so DoUpdatePreview() keeps them for the
    // caller of AddPrompt().
    std::optional<GeometricPrompt> PendingPrompt;
    bool PendingPromptApplied = false;
    std::string PendingPromptError;
    std::function<void(std::exception_ptr)> PendingPromptDone;

    // A start or a prompt runs off the GUI thread, during which the tool may be
    // deactivated, the session aborted or the time point changed. Those wait
    // until the work no longer uses the worker. The work reads AbortRequested,
    // which stops it.
    bool Starting = false;
    std::atomic<bool> AbortRequested = false;
    bool TimePointChangedMeanwhile = false;

    // Set by the destructor, which must not wait for a server.
    std::atomic<bool> Destroying = false;

  private:
    std::optional<Backend> m_Backend;
  };
}

mitk::nnInteractiveTool::nnInteractiveTool()
  : SegWithPreviewTool(true),
    m_Impl(std::make_unique<Impl>())
{
  this->KeepActiveAfterAcceptOn();
  this->ResetsToEmptyPreviewOn();
  this->RequestDeactivationConfirmationOn();
  this->RequiresVolumetricReferenceOn(); // the session image is handed to Python as a volume
  this->RequiresReferenceGeometryOn();
  this->SetSelectedLabels({1});

  for (const auto& [interactionType, interactor] : m_Impl->Interactors)
  {
    interactor->UpdatePreviewEvent += MessageDelegate1<nnInteractiveTool, bool>(
      this, &nnInteractiveTool::UpdatePreview);
  }
}

mitk::nnInteractiveTool::~nnInteractiveTool()
{
  // The work uses the worker.
  m_Impl->Destroying = true;
  m_Impl->AbortRequested = true;
  this->WaitForWorkOffGUIThread();

  for (const auto& [interactionType, interactor] : m_Impl->Interactors)
  {
    interactor->UpdatePreviewEvent -= MessageDelegate1<nnInteractiveTool, bool>(
      this, &nnInteractiveTool::UpdatePreview);
  }
}

const char* mitk::nnInteractiveTool::GetName() const
{
  return "nnInteractive";
}

us::ModuleResource mitk::nnInteractiveTool::GetIconResource() const
{
  auto segmentationModule = us::GetModuleContext()->GetModule("MitkSegmentation");
  auto iconResource = segmentationModule->GetResource("AI.svg");
  return iconResource;
}

void mitk::nnInteractiveTool::Deactivated()
{
  // The worker is still in use by the start, which tears down what it set up
  // once it has stopped. No interaction can exist before a session.
  if (m_Impl->Starting)
  {
    m_Impl->AbortRequested = true;
    Superclass::Deactivated();
    return;
  }

  // A prompt still uses the worker. Once it has stopped, the session ends,
  // see UpdateCleanUp().
  if (this->IsUpdating())
  {
    m_Impl->AbortRequested = true;
    this->DisableInteractor();
    Superclass::Deactivated();
    this->DeactivatedEvent.Send();
    return;
  }

  this->DisableInteractor();

  // Ended first, so resetting the interactions only clears the local prompts
  // and cannot report a lost session during the teardown.
  this->EndSession();
  this->ResetInteractions();

  Superclass::Deactivated();

  this->DeactivatedEvent.Send();
}

bool mitk::nnInteractiveTool::ConfirmBeforeDeactivation()
{
  return this->GetRequestDeactivationConfirmation() && this->HasInteractions();
}

const mitk::nnInteractiveTool::InteractorMap& mitk::nnInteractiveTool::GetInteractors() const
{
  return m_Impl->Interactors;
}

const Interactor* mitk::nnInteractiveTool::GetInteractor(InteractionType interactionType) const
{
  return m_Impl->Interactors.at(interactionType).get();
}

void mitk::nnInteractiveTool::EnableInteractor(InteractionType nextInteractionType, PromptType promptType)
{
  // Disable any other interactor if enabled.
  for (const auto& [interactionType, interactor] : m_Impl->Interactors)
  {
    if (interactionType != nextInteractionType && interactor->IsEnabled())
    {
      this->DisableInteractor(interactionType);
      break;
    }
  }

  // Enable the requested interactor for the given prompt type.
  m_Impl->Interactors[nextInteractionType]->Enable(promptType);
  m_Impl->PromptType = promptType;
}

void mitk::nnInteractiveTool::DisableInteractor(std::optional<InteractionType> interactionType)
{
  if (interactionType.has_value())
  {
    // Disable a specific interactor.
    m_Impl->Interactors[interactionType.value()]->Disable();
  }
  else
  {
    // Disable the currently enabled interator, if any.
    for (auto& [interactionType, interactor] : m_Impl->Interactors)
    {
      if (interactor->IsEnabled())
      {
        interactor->Disable();
        break;
      }
    }
  }
}

void mitk::nnInteractiveTool::ResetInteractions()
{
  // A prompt still uses the worker. The panel offers no reset meanwhile.
  if (this->IsUpdating())
    return;

  // An interactor that stays enabled would have the preview update below
  // read its last prompt, which no longer exists.
  this->DisableInteractor();

  m_Impl->ClearInteractions();

  if (this->IsSessionRunning())
  {
    try
    {
      m_Impl->ResetInteractions();
    }
    catch (const Exception& e)
    {
      // The local prompts are already cleared above. A lost session is
      // reported to the GUI, which tears it down.
      if (!this->HandleSessionError(e))
      {
        LogPythonTraceback(e);
        throw;
      }
    }
  }

  this->UpdatePreview();
  this->InteractionsChangedEvent.Send();
}

bool mitk::nnInteractiveTool::HasResultToConfirm() const
{
  return this->IsSessionRunning() && this->HasInteractions();
}

bool mitk::nnInteractiveTool::HasInteractions() const
{
  // Check if any interactor has interactions.
  for (const auto& [interactionType, interactor] : m_Impl->Interactors)
  {
    if (interactor->HasInteractions())
      return true;
  }

  // Check if the initialization segmentation has been set.
  return m_Impl->InitialSeg.IsNotNull();
}

void mitk::nnInteractiveTool::UndoLastInteraction()
{
  // A prompt still uses the worker. The panel offers no undo meanwhile.
  if (!this->IsSessionRunning() || !this->CanUndo() || this->IsUpdating())
    return;

  bool undone = false;

  try
  {
    undone = m_Impl->Undo();
  }
  catch (const Exception& e)
  {
    // The session can be lost here (lease expired, server gone or at capacity,
    // worker ended). Tear it down and notify the GUI; otherwise propagate as
    // the other session operations do.
    if (this->HandleSessionError(e))
      return;

    LogPythonTraceback(e);
    throw;
  }

  if (!undone)
  {
    // The session reports nothing to undo; resync our record and stop.
    m_Impl->ClearLastInteraction();
    this->InteractionsChangedEvent.Send();
    return;
  }

  // Remove the visualization of the undone interaction. The recorded prompt
  // type is the one used at interaction time, which is not necessarily the
  // currently active prompt type.
  if (m_Impl->LastInteractionType.has_value())
  {
    m_Impl->Interactors.at(m_Impl->LastInteractionType.value())
      ->RemoveLastInteraction(m_Impl->LastInteractionPromptType.value());
  }
  else if (m_Impl->LastInteractionWasMask)
  {
    m_Impl->InitialSeg = nullptr;
  }

  m_Impl->ClearLastInteraction();

  // session.undo() restored the target buffer in place; repaint the preview
  // from it via the regular update path (see the UndoRefreshPending branch in
  // DoUpdatePreview).
  m_Impl->UndoRefreshPending = true;
  this->UpdatePreview();

  // The refresh runs nothing off the GUI thread, so it has happened by now.
  // Clear the flag defensively: if UpdatePreview() returned before
  // reaching DoUpdatePreview (no input or preview image), a stale true would
  // make the next interaction take the refresh branch and be dropped silently.
  m_Impl->UndoRefreshPending = false;

  this->InteractionsChangedEvent.Send();
}

bool mitk::nnInteractiveTool::CanUndo() const
{
  return this->IsSessionRunning() && m_Impl->HasUndoableInteraction();
}

std::optional<mitk::nnInteractiveTool::Prompt> mitk::nnInteractiveTool::GetLastProgramPrompt() const
{
  if (!m_Impl->LastInteractionByProgram)
    return std::nullopt;

  return Prompt{ m_Impl->LastInteractionType.value(), m_Impl->LastInteractionPromptType.value() };
}

bool mitk::nnInteractiveTool::GetAutoZoom() const
{
  return m_Impl->AutoZoom;
}

void mitk::nnInteractiveTool::SetAutoZoom(bool autoZoom)
{
  if (m_Impl->AutoZoom == autoZoom)
    return;

  m_Impl->AutoZoom = autoZoom;
  this->OptionsChangedEvent.Send();

  // A prompt still uses the worker. The panel offers no change meanwhile.
  if (this->IsSessionRunning() && !this->IsUpdating())
  {
    try
    {
      m_Impl->SetAutoZoom();
    }
    catch (const Exception& e)
    {
      // A lost session is reported to the GUI, which tears it down.
      if (!this->HandleSessionError(e))
      {
        LogPythonTraceback(e);
        throw;
      }
    }
  }
}

bool mitk::nnInteractiveTool::GetAutoRefine() const
{
  return m_Impl->AutoRefine;
}

void mitk::nnInteractiveTool::SetAutoRefine(bool autoRefine)
{
  if (m_Impl->AutoRefine == autoRefine)
    return;

  m_Impl->AutoRefine = autoRefine;
  this->OptionsChangedEvent.Send();
}

std::optional<Backend> mitk::nnInteractiveTool::GetBackend() const
{
  return m_Impl->GetBackend();
}

void mitk::nnInteractiveTool::SetToolManager(ToolManager* toolManager)
{
  Superclass::SetToolManager(toolManager);

  for (auto& [interactionType, interactor] : m_Impl->Interactors)
    interactor->SetToolManager(toolManager);
}

void mitk::nnInteractiveTool::InitializeSessionWithMask(Image* mask)
{
  if (mask == nullptr)
    return;

  if (this->IsUpdating())
  {
    MITK_WARN << "nnInteractive: a prompt is still computing, so the session is not initialized with the mask.";
    return;
  }

  m_Impl->InitialSeg = mask;
  this->UpdatePreview();
}

void mitk::nnInteractiveTool::SetPreviewLabel(MultiLabelSegmentation::LabelValueType value, const Color& color)
{
  auto image = this->GetPreviewSegmentation();

  if (image == nullptr)
    return;

  auto label = image->GetLabel(value);

  if (label.IsNull())
  {
    label = Label::New(value, "preview");
    image->AddLabel(label, 0, false, false);
  }

  image->GetLabel(value)->SetColor(color);
  image->UpdateLabel(value, label);

  image->SetActiveLabel(value);
  this->SetSelectedLabels({value});
}

void mitk::nnInteractiveTool::DoUpdatePreview(const Image* inputAtTimeStep, const Image* /*oldSegAtTimeStep*/, MultiLabelSegmentation* previewImage, TimeStepType timeStep)
{
  // This method assumes it is only called when an interaction has occurred or
  // when a session should be (re)initialized with a label mask. Otherwise,
  // calling this method will reset any existing preview content.

  if (previewImage == nullptr || !this->IsSessionRunning())
    return;

  // Color the preview with the selected target label's color (falling back to
  // the special preview color) so it matches the label being segmented.
  auto previewColor = this->GetSpecialPreviewColor();

  if (const auto* segmentation = this->GetTargetSegmentation(); segmentation != nullptr)
  {
    if (const auto* activeLabel = segmentation->GetActiveLabel(); activeLabel != nullptr)
      previewColor = activeLabel->GetColor();
  }

  this->SetPreviewLabel(1, previewColor);

  // An undo restored the target buffer in place; just repaint the preview from
  // it. This must run before the interactor branch (an interactor is typically
  // still enabled while undoing) and must not emit PreviewUpdatedEvent, so it
  // does not trigger the GUI's auto-confirm.
  if (m_Impl->UndoRefreshPending)
  {
    m_Impl->UndoRefreshPending = false;
    previewImage->UpdateGroupImage(previewImage->GetActiveLayer(), m_Impl->TargetBuffer, timeStep, 0);
    return;
  }

  const auto* interactor = m_Impl->GetEnabledInteractor();

  // What the session gets, read from the interactor and the images here on the
  // GUI thread. The prediction runs off it.
  std::string method;
  nlohmann::json params;
  std::shared_ptr<SharedBuffer> mask;
  std::optional<InteractionType> interactionType;
  auto promptType = m_Impl->PromptType;
  bool byProgram = false;

  // A prompt of AddPrompt() takes precedence over an interactor that may be
  // enabled as well. Its errors go back to the caller of AddPrompt() rather
  // than to the user.
  if (m_Impl->PendingPrompt.has_value())
  {
    const auto prompt = std::move(m_Impl->PendingPrompt.value());
    m_Impl->PendingPrompt.reset();

    try
    {
      auto call = m_Impl->DrawPrompt(prompt, inputAtTimeStep);
      method = std::move(call.Method);
      params = std::move(call.Params);
      mask = std::move(call.Mask);
    }
    catch (const Exception& e)
    {
      m_Impl->PendingPromptError = e.GetDescription();
      return;
    }
    catch (const std::exception& e)
    {
      m_Impl->PendingPromptError = e.what();
      return;
    }

    interactionType = prompt.Type;
    promptType = prompt.PromptType;
    byProgram = true;
  }
  else if (interactor != nullptr)
  {
    interactionType = interactor->GetType();
    const bool include = promptType == PromptType::Positive;

    switch (*interactionType)
    {
      case InteractionType::Point:
      {
        auto point = static_cast<const PointInteractor*>(interactor)->GetLastPoint();
        method = "add_point";
        params = m_Impl->GetPointParams(point.value(), inputAtTimeStep, include);
        break;
      }
      case InteractionType::Box:
      {
        auto box = static_cast<const BoxInteractor*>(interactor)->GetLastBox();
        method = "add_bbox";
        params = m_Impl->GetBoxParams(box->GetWorldControlPoint(0), box->GetWorldControlPoint(1), inputAtTimeStep, include);
        break;
      }
      case InteractionType::Scribble:
      {
        auto scribbleInteractor = static_cast<const ScribbleInteractor*>(interactor);
        auto scribbleMask = scribbleInteractor->GetLastScribbleMask();
        if (scribbleMask.IsNull())
        {
          MITK_WARN << "Skipping scribble preview update: no mask available.";
          return;
        }
        method = "add_scribble";
        params = m_Impl->GetMaskParams(scribbleInteractor->GetLastScribbleBoundingBox(), include);
        mask = SharedBuffer::CopyOf(scribbleMask.GetPointer());
        break;
      }
      case InteractionType::Lasso:
      {
        auto lassoInteractor = static_cast<const LassoInteractor*>(interactor);
        auto lassoMask = lassoInteractor->GetLastLassoMask();
        if (lassoMask.IsNull())
        {
          MITK_WARN << "Skipping lasso preview update: no mask available.";
          return;
        }
        method = "add_lasso";
        params = m_Impl->GetMaskParams(lassoInteractor->GetLastLassoBoundingBox(), include);
        mask = SharedBuffer::CopyOf(lassoMask.GetPointer());
        break;
      }
      default:
        MITK_ERROR << "Cannot update preview because of unknown interaction type!";
        return;
    }
  }
  else if (m_Impl->InitialSeg.IsNotNull())
  {
    const auto maskTimeStep = m_Impl->InitialSeg->GetTimeSteps() > 1
      ? m_Impl->SessionWorkingDataTimeStep
      : 0;

    method = "add_initial_seg";
    params = { { "run_prediction", m_Impl->AutoRefine } };
    mask = SharedBuffer::CopyOf(m_Impl->InitialSeg, maskTimeStep);
  }
  else
  {
    this->ResetPreviewContentAtTimeStep(timeStep);
    m_Impl->ClearLastInteraction();
    this->InteractionsChangedEvent.Send();
    return;
  }

  auto* worker = m_Impl->Worker.get();
  auto* task = m_ProgressCommand->GetProgressTask();

  // A remote prompt is short. Ending its worker would leave the lease on the
  // server until its idle timeout, so a deactivation or an aborted session
  // waits for it, and the teardown releases the lease. A cancellation by the
  // user and the destruction of the tool still end it at once.
  const bool remote = m_Impl->Remote;
  const auto progress = MakeStopHook(this, task, [this, remote]() {
    return m_Impl->Destroying.load() || (!remote && m_Impl->AbortRequested.load());
  });

  // Held for the result, as the tool can be deactivated meanwhile.
  const MultiLabelSegmentation::Pointer preview = previewImage;

  this->RunOffGUIThread(
    [worker, progress, method, params, mask]() {
      PythonWorker::Buffers buffers;

      if (mask != nullptr)
        buffers["mask"] = mask.get();

      worker->Call("nninteractive_worker." + method, params, buffers, progress);
    },
    [this, task, preview, timeStep, interactionType, promptType, byProgram](std::exception_ptr error) {
      if (error != nullptr)
      {
        if (!byProgram)
        {
          this->EndFailedPrompt(error, task, false);
          return;
        }

        // The model did not take the prompt of the program, so its drawing
        // goes again, and the caller of AddPrompt() learns why.
        m_Impl->Interactors.at(*interactionType)->RemoveLastInteraction(promptType);

        try
        {
          this->EndFailedPrompt(error, task, true);
          m_Impl->PendingPromptError = "nnInteractive: the prompt was cancelled, or the session ended.";
        }
        catch (const Exception& e)
        {
          m_Impl->PendingPromptError = e.GetDescription();
        }
        catch (const std::exception& e)
        {
          m_Impl->PendingPromptError = e.what();
        }

        return;
      }

      // Deactivated or aborted meanwhile, which ends the session in UpdateCleanUp().
      if (m_Impl->AbortRequested)
      {
        if (byProgram)
          m_Impl->PendingPromptError = "nnInteractive: the session ended while the prompt computed.";

        return;
      }

      if (interactionType.has_value())
      {
        preview->UpdateGroupImage(preview->GetActiveLayer(), m_Impl->TargetBuffer, timeStep, 0);

        // Record this interaction so UndoLastInteraction() knows which prompt
        // to remove: the prompt type the enabled interactor was activated with
        // when the interaction was made, or the one of AddPrompt().
        m_Impl->LastInteractionType = interactionType;
        m_Impl->LastInteractionPromptType = promptType;
        m_Impl->LastInteractionWasMask = false;
        m_Impl->LastInteractionByProgram = byProgram;

        if (byProgram)
          m_Impl->PendingPromptApplied = true;

        this->InteractionsChangedEvent.Send();
        this->PreviewUpdatedEvent.Send();
      }
      else
      {
        preview->UpdateGroupImage(preview->GetActiveLayer(), m_Impl->TargetBuffer, timeStep, 0, ImageAccessorBase::IgnoreLock);

        // The initial-segmentation mask is one undoable step (the session
        // snapshots before applying it); it has no prompt node to remove.
        m_Impl->ClearLastInteraction();
        m_Impl->LastInteractionWasMask = true;

        this->InteractionsChangedEvent.Send();
      }
    });
}

void mitk::nnInteractiveTool::EndFailedPrompt(std::exception_ptr error, const ProgressTask* task, bool byProgram)
{
  try
  {
    std::rethrow_exception(error);
  }
  catch (const Exception& e)
  {
    // Deactivated or aborted meanwhile, which ends the session in UpdateCleanUp().
    if (m_Impl->AbortRequested)
      return;

    // The cancellation ended the worker and with it the session.
    if (task != nullptr && task->IsCancelRequested())
    {
      this->SessionLostEvent.Send({ SessionLoss::Cause::Cancelled, e.GetDescription(), byProgram });
      return;
    }

    // The session can be lost mid-interaction (lease expired, server gone or
    // at capacity, worker ended). Tear it down and notify the GUI; otherwise
    // propagate the error.
    if (this->HandleSessionError(e))
      return;

    LogPythonTraceback(e);
    throw;
  }
}

void mitk::nnInteractiveTool::Notify(InteractionEvent* event, bool isHandled)
{
  // If the event is not yet handled, forward it to the currently enabled
  // interactor, if any. It ignores the event while a prompt computes, see
  // UpdatePrepare().
  if (!isHandled)
  {
    for (auto& [interactionType, interactor] : m_Impl->Interactors)
    {
      if (interactor->IsEnabled())
      {
        interactor->HandleEvent(event);
        return;
      }
    }
  }

  Superclass::Notify(event, isHandled);
}

void mitk::nnInteractiveTool::ConfirmCleanUp()
{
  this->ConfirmCleanUpEvent.Send(true);
}

std::string mitk::nnInteractiveTool::GetVirtualEnvName() const
{
  return this->GetName();
}

const std::vector<std::pair<std::string, std::string>>& mitk::nnInteractiveTool::GetSessionDefiningPreferences()
{
  // Key, default-as-stored. The defaults must match what StartSession()/the
  // session construction reads and what the preference page writes, so that
  // seeding leaves the stored value equal to the effective one.
  static const std::vector<std::pair<std::string, std::string>> keys = {
    { "nnInteractive/inferenceMode", "local" },
    { "nnInteractive/serverUrl", "" },
    { "nnInteractive/apiKey", "" },
    { "nnInteractive/modelSource", "huggingface" },
    { "nnInteractive/modelCheckpoint", "" },
    { "nnInteractive/localModelPath", "" },
    { "nnInteractive/backend", "auto" },
    { "nnInteractive/gpuBackend", "cuda:0" },
    { "nnInteractive/useTorchCompile", "true" },
    { "nnInteractive/interactionsStorage", "auto" },
  };

  return keys;
}

namespace
{
  // What a session start reads on the GUI thread and what it brings back from
  // the worker. The work off the GUI thread and its done share it, one after
  // the other.
  struct SessionStart
  {
    bool Remote = false;
    std::string ServerUrl;
    std::string ApiKey;

    mitk::Torch::BackendPreference Backend = mitk::Torch::BackendPreference::Auto;
    std::string GPUDevice;
    std::string ModelSource;
    std::string LocalModelPath;
    std::string ModelCheckpoint;
    bool TorchCompile = false;
    std::string InteractionsStorage;
    bool AutoZoom = true;

    std::unique_ptr<mitk::SharedBuffer> Image;
    nlohmann::json Spacing;
    std::shared_ptr<mitk::SharedBuffer> TargetSegment;
    mitk::Image::Pointer TargetBuffer;
    mitk::TimeStepType ReferenceTimeStep = 0;
    mitk::TimeStepType WorkingTimeStep = 0;

    /** Whether the session runs in the worker, even if binding the image failed. */
    bool Started = false;
    nlohmann::json Capabilities;
    std::optional<mitk::nnInteractive::Backend> SelectedBackend;
    bool ProxyBypassed = false;
  };

  void SetSessionImage(mitk::PythonWorker& worker, SessionStart& start, const mitk::PythonWorker::ProgressHook& progress)
  {
    worker.Call("nninteractive_worker.set_image",
      { { "spacing", start.Spacing } },
      { { "image", start.Image.get() }, { "target", start.TargetSegment.get() } },
      progress);

    // The worker keeps its own mapping of the image.
    start.Image.reset();
  }

  void StartRemoteSession(mitk::PythonWorker& worker, SessionStart& start, const mitk::PythonWorker::ProgressHook& progress)
  {
    // Expected failures, like an unreachable server or a rejected API key, come
    // back as short user-facing messages. The server URL is shown without
    // credentials.
    const auto result = worker.Call("nninteractive_worker.start_remote", {
      { "server_url", start.ServerUrl },
      { "server_display", SanitizeUrlForDisplay(start.ServerUrl) },
      { "api_key", start.ApiKey }
    }, {}, progress);

    if (const auto error = result.value("error", std::string()); !error.empty())
      mitkThrow() << error;

    start.ProxyBypassed = result.value("proxy_bypassed", false);
    start.Capabilities = result["capabilities"];
    start.Started = true;

    // The lease is claimed. If configuring the session or uploading the image
    // fails because the connection dropped mid-flight, ending the session
    // releases the half-open lease, and a short message replaces the raw
    // traceback.
    try
    {
      worker.Call("nninteractive_worker.set_autozoom", { { "enabled", start.AutoZoom } }, {}, progress);
      SetSessionImage(worker, start, progress);
    }
    catch (const mitk::Exception& e)
    {
      if (IsRemoteConnectionLost(e))
        mitkThrow() << "Lost the connection to the nnInteractive server while starting the session. "
                       "Check the server and try again.";

      throw;
    }
  }

  void StartLocalSession(mitk::PythonWorker& worker, SessionStart& start, const mitk::PythonWorker::ProgressHook& progress)
  {
    // A client-only install (nninteractive-client) has no local inference backend
    // and no PyTorch. Fail fast with an actionable message instead of letting the
    // torch import below raise a raw traceback.
    if (!worker.Call("nninteractive_worker.is_local_inference_available", {}, {}, progress).get<bool>())
      mitkThrow() << "This is a client-only nnInteractive installation, which supports remote "
                     "sessions only. Select Remote in Preferences -> Segmentation -> nnInteractive, "
                     "or uninstall and reinitialize in Full mode to enable local inference.";

    const auto deviceSelection = mitk::Torch::SelectDevice(worker, start.Backend, start.GPUDevice, 6000, progress);
    const bool useCUDADevice = deviceSelection.SelectedBackend == mitk::Torch::Backend::CUDA;

    if (!useCUDADevice)
      start.AutoZoom = false;

    // torch.compile is only meaningful on Linux with a CUDA device (the library
    // itself forces it off on Windows, and it relies on Triton/NVIDIA). The
    // useCUDADevice guard also neutralizes a stale preference, e.g. enabled and
    // then the backend switched to CPU or auto-fell back to CPU.
    bool useTorchCompile = false;
#if defined(__linux__)
    useTorchCompile = useCUDADevice && start.TorchCompile;
#endif

    // The checkpoint is a local folder or managed by nnInteractive's model
    // management, which downloads it if needed and works offline once it has.
    // An empty checkpoint means the library's recommended default.
    const auto result = worker.Call("nninteractive_worker.start_local", {
      { "device", deviceSelection.Device },
      { "model_source", start.ModelSource },
      { "local_model_path", start.LocalModelPath },
      { "model_checkpoint", start.ModelCheckpoint },
      { "use_torch_compile", useTorchCompile },
      { "do_autozoom", start.AutoZoom },
      { "interactions_storage", start.InteractionsStorage }
    }, {}, progress);

    if (const auto error = result.value("error", std::string()); !error.empty())
      mitkThrow() << error;

    start.Capabilities = result["capabilities"];
    start.Started = true;
    start.SelectedBackend = deviceSelection.SelectedBackend;

    SetSessionImage(worker, start, progress);
  }
}

void mitk::nnInteractiveTool::StartSession(std::shared_ptr<ProgressTask> task, std::function<void(std::exception_ptr error)> done)
{
  if (m_Impl->Starting || this->IsUpdating())
  {
    done(MakeExceptionPtr("nnInteractive: the session cannot be started while the tool is busy."));
    return;
  }

  if (this->IsSessionRunning())
    this->EndSession();

  auto prefs = GetPreferences();

  // Materialize the session-defining preferences with their effective values so a
  // later no-op preferences "OK" does not register as a value change and tear the
  // session down (mitk::Preferences fires its change event only on an actual
  // change, and a default written into a never-stored key counts as one). Safe
  // here: no session is running yet, so the GUI's stale-session observer is a
  // no-op for these writes.
  for (const auto& [key, defaultValue] : GetSessionDefiningPreferences())
    prefs->Put(key, prefs->Get(key, defaultValue));

  auto start = std::make_shared<SessionStart>();

  try
  {
    start->Remote = prefs->Get("nnInteractive/inferenceMode", "local") == "remote";

    if (start->Remote)
    {
      start->ServerUrl = prefs->Get("nnInteractive/serverUrl", "");
      start->ApiKey = prefs->Get("nnInteractive/apiKey", "");

      if (start->ServerUrl.empty())
        mitkThrow() << "Remote mode is selected but no server URL is configured. "
                       "Click on Settings to configure a server URL.";
    }
    else
    {
      start->Backend = Torch::ParseBackendPreference(prefs->Get("nnInteractive/backend", "auto"));
      start->GPUDevice = prefs->Get("nnInteractive/gpuBackend", "cuda:0");
      start->ModelSource = prefs->Get("nnInteractive/modelSource", "huggingface");
      start->LocalModelPath = prefs->Get("nnInteractive/localModelPath", "");
      start->ModelCheckpoint = prefs->Get("nnInteractive/modelCheckpoint", "");
      start->TorchCompile = prefs->GetBool("nnInteractive/useTorchCompile", true);

      if (start->ModelSource == "local" && start->LocalModelPath.empty())
        mitkThrow() << "nnInteractive: Local model mode is selected but no checkpoint folder is configured. "
                       "Set the path in Preferences -> Segmentation -> nnInteractive.";

      // Storage backend for the interaction tensor. "auto" (the library default)
      // uses a dense tensor for smaller images and blosc2 for larger ones; blosc2
      // trades speed for lower RAM, tensor the other way around. Only applies to
      // local sessions; a remote server decides this server-side.
      start->InteractionsStorage = prefs->Get("nnInteractive/interactionsStorage", "auto");
      if (start->InteractionsStorage != "blosc2" && start->InteractionsStorage != "tensor")
        start->InteractionsStorage = "auto";
    }

    start->AutoZoom = m_Impl->AutoZoom;

    auto image = this->GetToolManager()->GetReferenceData(0)->GetDataAs<Image>();

    const auto timePoint = this->GetToolManager()->GetCurrentTimePoint();
    start->ReferenceTimeStep = image->GetTimeGeometry()->TimePointToTimeStep(timePoint);

    // Pin the session to the time steps that are active now. OnTimePointChanged()
    // ends the session if either changes, since the Python model is bound to a
    // single 3D slice.
    const auto* workingSeg = this->GetTargetSegmentation();
    start->WorkingTimeStep = workingSeg != nullptr
      ? workingSeg->GetTimeGeometry()->TimePointToTimeStep(timePoint)
      : 0;

    auto imageAtTimeStep = this->GetImageByTimeStep(image, start->ReferenceTimeStep);

    start->Image = SharedBuffer::CopyOf(imageAtTimeStep);
    start->TargetSegment = std::make_shared<SharedBuffer>(MultiLabelSegmentation::GetPixelType(), start->Image->GetShape());
    start->TargetBuffer = start->TargetSegment->AsImage(*imageAtTimeStep->GetTimeGeometry());

    const auto spacing = imageAtTimeStep->GetGeometry()->GetSpacing();
    start->Spacing = { spacing[2], spacing[1], spacing[0] };

    const auto python = PythonHelper::GetVirtualEnvExecutablePath(this->GetVirtualEnvName());

    if (python.empty())
      mitkThrow() << "nnInteractive is not installed.";

    m_Impl->Worker = std::make_unique<PythonWorker>(python, std::vector<PythonWorker::Module> {
      ReadWorkerModule("torch_device.py"),
      ReadWorkerModule("nnInteractive/nninteractive_worker.py")
    }, "nnInteractive");
  }
  catch (...)
  {
    m_Impl->Worker.reset();
    done(std::current_exception());
    return;
  }

  m_Impl->Starting = true;
  m_Impl->AbortRequested = false;
  m_Impl->TimePointChangedMeanwhile = false;

  auto* worker = m_Impl->Worker.get();
  const auto progress = MakeStopHook(this, task.get(), [this]() { return m_Impl->AbortRequested.load(); });

  this->RunOffGUIThread(
    [worker, progress, start]() {
      if (start->Remote)
        StartRemoteSession(*worker, *start, progress);
      else
        StartLocalSession(*worker, *start, progress);
    },
    [this, task, start, done](std::exception_ptr error) {
      m_Impl->Starting = false;

      // Deactivated or aborted after the session had started: it is not wanted any more.
      if (error == nullptr && m_Impl->AbortRequested)
        error = MakeExceptionPtr("nnInteractive: aborted.");

      m_Impl->SessionStarted = start->Started;
      m_Impl->Remote = start->Remote;

      if (error != nullptr)
      {
        // A remote session holds a lease on the server, which ending it releases.
        // Otherwise the server slot is held until the idle reaper expires it.
        if (this->IsSessionRunning())
        {
          this->EndSession();
        }
        else
        {
          m_Impl->Worker.reset();
          m_Impl->SessionStarted = false;
          m_Impl->Remote = false;
        }

        m_Impl->AbortRequested = false;
        m_Impl->TimePointChangedMeanwhile = false;

        done(error);
        return;
      }

      if (start->ProxyBypassed)
        MITK_INFO << "Connected to the nnInteractive server directly, bypassing the configured HTTP proxy.";

      m_Impl->SetCapabilities(start->Capabilities);

      const bool autoZoomChanged = m_Impl->AutoZoom != start->AutoZoom;
      m_Impl->AutoZoom = start->AutoZoom;

      if (start->SelectedBackend.has_value())
        m_Impl->SetBackend(*start->SelectedBackend);

      m_Impl->TargetSegment = start->TargetSegment;
      m_Impl->TargetBuffer = start->TargetBuffer;
      m_Impl->SessionReferenceDataTimeStep = start->ReferenceTimeStep;
      m_Impl->SessionWorkingDataTimeStep = start->WorkingTimeStep;

      // A fresh session has nothing to undo (set_image resets the session's undo
      // state); keep the tool's record in sync.
      m_Impl->ClearLastInteraction();
      m_Impl->UndoRefreshPending = false;

      if (autoZoomChanged)
        this->OptionsChangedEvent.Send();

      done(nullptr);

      if (m_Impl->TimePointChangedMeanwhile)
      {
        m_Impl->TimePointChangedMeanwhile = false;
        this->OnTimePointChanged();
      }

      // Whoever started it, a GUI can follow.
      if (this->IsSessionRunning())
        this->SessionStartedEvent.Send();
    });
}

void mitk::nnInteractiveTool::AddPrompt(InteractionType interactionType,
                                        const std::vector<Point3D>& points,
                                        PromptType promptType,
                                        const PlaneGeometry* plane,
                                        std::function<void(std::exception_ptr error)> done)
{
  if (!this->IsSessionRunning())
    mitkThrow() << "nnInteractive has no session. Start one first.";

  // The session takes one prompt at a time, as with the mouse.
  if (this->IsUpdating() || this->IsRunningOffGUIThread())
    mitkThrow() << "nnInteractive is still computing a prompt.";

  const auto supported = this->GetSupportedInteractions();
  std::size_t minCount = 1;
  std::size_t maxCount = 1;
  bool isSupported = false;

  switch (interactionType)
  {
    case InteractionType::Point:
      isSupported = supported.Point;
      break;
    case InteractionType::Box:
      isSupported = supported.Box;
      minCount = maxCount = 2;
      break;
    case InteractionType::Scribble:
      isSupported = supported.Scribble;
      minCount = 2;
      maxCount = MAX_STROKE_PROMPT_POINTS;
      break;
    case InteractionType::Lasso:
      isSupported = supported.Lasso;
      minCount = 3;
      maxCount = MAX_STROKE_PROMPT_POINTS;
      break;
  }

  if (!isSupported)
    mitkThrow() << "The model of this session does not support " << GetInteractionTypeAsString(interactionType) << " prompts.";

  if ((InteractionType::Scribble == interactionType || InteractionType::Lasso == interactionType)
      && points.size() > maxCount)
  {
    mitkThrow() << "A " << GetInteractionTypeAsString(interactionType) << " prompt takes at most "
      << MAX_STROKE_PROMPT_POINTS << " points. A few points along the structure are enough.";
  }

  if (points.size() < minCount || points.size() > maxCount)
  {
    mitkThrow() << "A " << GetInteractionTypeAsString(interactionType) << " prompt needs "
      << (minCount == maxCount ? std::to_string(minCount) : "at least " + std::to_string(minCount))
      << (minCount == 1 ? " point." : " points.");
  }

  // Cloned, as the plane of a render window changes with its slice.
  m_Impl->PendingPrompt = Impl::GeometricPrompt{ interactionType, promptType, points,
    plane != nullptr ? plane->Clone().GetPointer() : nullptr };
  m_Impl->PendingPromptApplied = false;
  m_Impl->PendingPromptError.clear();
  m_Impl->PendingPromptDone = std::move(done);

  // As the interactors trigger it.
  try
  {
    this->UpdatePreview(false);
  }
  catch (...)
  {
    // The update did not start, so nothing takes the prompt or calls done,
    // and the next update of anybody must not find them.
    m_Impl->PendingPrompt.reset();
    m_Impl->PendingPromptDone = nullptr;
    throw;
  }

  // The update ended already, without work off the GUI thread: it did not
  // reach DoUpdatePreview(), or that failed before.
  if (!this->IsUpdating())
    this->EndProgramPrompt();
}

void mitk::nnInteractiveTool::EndProgramPrompt()
{
  m_Impl->PendingPrompt.reset();

  auto done = std::exchange(m_Impl->PendingPromptDone, nullptr);

  if (!done)
    return;

  if (m_Impl->PendingPromptApplied)
  {
    done(nullptr);
    return;
  }

  done(MakeExceptionPtr(m_Impl->PendingPromptError.empty()
    ? std::string("The prompt was not applied, as the tool has no image or preview to work on.")
    : m_Impl->PendingPromptError));
}

void mitk::nnInteractiveTool::EndSession()
{
  if (!this->IsSessionRunning())
    return;

  if (m_Impl->Remote && m_Impl->Worker->IsRunning())
  {
    // Release the server lease right away so the slot frees up for other users
    // instead of waiting for the idle reaper. close() is best-effort and
    // idempotent server-side; swallow errors so teardown always completes.
    try
    {
      m_Impl->Worker->Call("nninteractive_worker.close");
    }
    catch (const Exception& e)
    {
      MITK_WARN << "nnInteractive: error while releasing the remote session (ignored): "
                << e.GetDescription();
    }
  }

  // Ending the process frees the model and the memory of the session.
  m_Impl->Worker.reset();
  m_Impl->SessionStarted = false;
  m_Impl->TargetBuffer = nullptr;
  m_Impl->TargetSegment.reset();
  m_Impl->Interactions = {};
  m_Impl->License.clear();
  m_Impl->SupportsUndo = false;
  m_Impl->Remote = false;
  m_Impl->ResetBackend();
  m_Impl->SessionReferenceDataTimeStep = 0;
  m_Impl->SessionWorkingDataTimeStep = 0;
  m_Impl->ClearLastInteraction();
  m_Impl->UndoRefreshPending = false;

  this->SessionEndedEvent.Send();
}

bool mitk::nnInteractiveTool::IsRemoteSession() const
{
  return m_Impl->Remote;
}

mitk::nnInteractiveTool::SupportedInteractions mitk::nnInteractiveTool::GetSupportedInteractions() const
{
  if (!this->IsSessionRunning())
    return {};

  return m_Impl->Interactions;
}

std::optional<std::string> mitk::nnInteractiveTool::GetModelLicense() const
{
  if (!this->IsSessionRunning() || m_Impl->License.empty())
    return std::nullopt;

  return m_Impl->License;
}

bool mitk::nnInteractiveTool::SupportsUndo() const
{
  return this->IsSessionRunning() && m_Impl->SupportsUndo;
}

bool mitk::nnInteractiveTool::HandleSessionError(const Exception& error)
{
  const auto cause = GetSessionLossCause(error);

  if (!cause.has_value())
    return false;

  const SessionLoss loss{ *cause, error.GetDescription() };
  MITK_ERROR << "nnInteractive: the session is lost. " << loss.Details;

  // Do not tear down here: this runs while an interactor is mid-event, where
  // disabling/resetting interactors is unsafe. Signal the GUI, which calls
  // AbortSession() on the next event-loop tick.
  this->SessionLostEvent.Send(loss);

  return true;
}

void mitk::nnInteractiveTool::AbortSession()
{
  // A start or a prompt still uses the worker. They end the session once they
  // have stopped, see StartSession() and UpdateCleanUp().
  if (m_Impl->Starting || this->IsUpdating())
  {
    m_Impl->AbortRequested = true;
    return;
  }

  this->TearDownSession();
}

void mitk::nnInteractiveTool::TearDownSession()
{
  m_Impl->AbortRequested = false;
  m_Impl->TimePointChangedMeanwhile = false;

  this->DisableInteractor();
  this->EndSession();

  // Not ResetInteractions(), which updates the preview of a tool that may have
  // been deactivated meanwhile.
  m_Impl->ClearInteractions();
  this->ResetPreviewContent();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  this->InteractionsChangedEvent.Send();
}

void mitk::nnInteractiveTool::UpdatePrepare()
{
  Superclass::UpdatePrepare();

  // The session takes one prompt at a time. A prompt made in the views while
  // this one computes would be shown but never reach the session.
  for (auto& [interactionType, interactor] : m_Impl->Interactors)
    interactor->SetInputSuspended(true);
}

void mitk::nnInteractiveTool::UpdateCleanUp()
{
  for (auto& [interactionType, interactor] : m_Impl->Interactors)
    interactor->SetInputSuspended(false);

  // Posted, so that the caller of AddPrompt() finds the update ended.
  if (m_Impl->PendingPromptDone)
    this->PostToGUIThread([this]() { this->EndProgramPrompt(); });

  // Posted, as the update has not ended yet. Through AbortSession(), which
  // checks again: a prompt can start before the posted call runs, and then
  // the teardown follows that one.
  if (m_Impl->AbortRequested)
  {
    this->PostToGUIThread([this]() { this->AbortSession(); });
  }
  else if (m_Impl->TimePointChangedMeanwhile)
  {
    m_Impl->TimePointChangedMeanwhile = false;
    this->PostToGUIThread([this]() { this->OnTimePointChanged(); });
  }
}

bool mitk::nnInteractiveTool::IsCancelable() const
{
  return true;
}

void mitk::nnInteractiveTool::OnTimePointChanged()
{
  // The Python session is bound to a single 3D slice extracted at the time
  // step that was active when StartSession() ran. Without a session there is
  // nothing to reconcile, and DoUpdatePreview() short-circuits without a
  // session, so we deliberately do not call the base class handler (which
  // would invoke UpdatePreview against a stale binding). A session that is
  // being started or computes a prompt is reconciled once that has ended.
  if (m_Impl->Starting || this->IsUpdating())
  {
    m_Impl->TimePointChangedMeanwhile = true;
    return;
  }

  if (!this->IsSessionRunning())
    return;

  const auto timePoint = this->GetToolManager()->GetCurrentTimePoint();

  const auto* referenceNode = this->GetToolManager()->GetReferenceData(0);
  const auto* referenceImage = referenceNode != nullptr
    ? referenceNode->GetDataAs<Image>()
    : nullptr;
  if (referenceImage == nullptr)
    return;

  const auto currentImageTimeStep = referenceImage->GetTimeGeometry()->TimePointToTimeStep(timePoint);

  const auto* workingSeg = this->GetTargetSegmentation();
  const auto currentWorkingTimeStep = workingSeg != nullptr
    ? workingSeg->GetTimeGeometry()->TimePointToTimeStep(timePoint)
    : 0;

  if (currentImageTimeStep == m_Impl->SessionReferenceDataTimeStep &&
      currentWorkingTimeStep == m_Impl->SessionWorkingDataTimeStep)
  {
    return;
  }

  this->DisableInteractor();
  this->EndSession();
  this->ResetInteractions();
  this->ResetPreviewContent();
}

bool mitk::nnInteractiveTool::IsSessionRunning() const
{
  return m_Impl->Worker != nullptr && m_Impl->SessionStarted;
}

bool mitk::nnInteractiveTool::IsSessionStarting() const
{
  return m_Impl->Starting;
}

void mitk::nnInteractiveTool::Impl::SetAutoZoom() const
{
  this->Worker->Call("nninteractive_worker.set_autozoom", { { "enabled", this->AutoZoom } });
}

nlohmann::json mitk::nnInteractiveTool::Impl::GetPointParams(const Point3D& point, const Image* inputAtTimeStep, bool include) const
{
  itk::Index<3> index;
  inputAtTimeStep->GetGeometry()->WorldToIndex(point, index);

  return {
    { "coordinates", { index[2], index[1], index[0] } },
    { "include", include }
  };
}

nlohmann::json mitk::nnInteractiveTool::Impl::GetBoxParams(const Point3D& corner, const Point3D& oppositeCorner, const Image* inputAtTimeStep, bool include) const
{
  const auto* geometry = inputAtTimeStep->GetGeometry();

  std::array<itk::Index<3>, 2> indices;
  geometry->WorldToIndex(corner, indices[0]);
  geometry->WorldToIndex(oppositeCorner, indices[1]);

  auto bbox = nlohmann::json::array();

  for (int i = 2; i >= 0; --i)
  {
    // nnInteractive expects half-open bounding boxes [min, max).
    // Our indices are inclusive, so we add +1 to the upper bound.
    bbox.push_back({ std::min(indices[0][i], indices[1][i]), std::max(indices[0][i], indices[1][i]) + 1 });
  }

  return {
    { "bbox", bbox },
    { "include", include }
  };
}

nlohmann::json mitk::nnInteractiveTool::Impl::GetMaskParams(const InteractionBoundingBox* boundingBox, bool include) const
{
  nlohmann::json params = { { "include", include } };

  if (boundingBox != nullptr)
    params["bbox"] = ToJson(*boundingBox);

  return params;
}

mitk::nnInteractiveTool::Impl::PromptCall mitk::nnInteractiveTool::Impl::DrawPrompt(const GeometricPrompt& prompt, const Image* inputAtTimeStep)
{
  const bool include = prompt.PromptType == PromptType::Positive;
  const auto* geometry = inputAtTimeStep->GetGeometry();

  for (const auto& point : prompt.Points)
  {
    if (!geometry->IsInside(point))
      mitkThrow() << "The point " << point << " lies outside the image.";
  }

  // Without the plane of a render window, the prompt lies in the slice of
  // the image that its points lie in. A point is not drawn in a plane.
  auto plane = prompt.Plane;

  if (plane.IsNull() && prompt.Type != InteractionType::Point)
    plane = PlaneGeometry::ConstPointer(CreateSlicePlane(prompt.Points, geometry));

  // The interactor of the prompt's type draws it as it draws one of the
  // mouse, and the model gets what that drawing gives, as from the prompts
  // of the user.
  auto* interactor = this->Interactors.at(prompt.Type).get();

  switch (prompt.Type)
  {
    case InteractionType::Point:
    {
      const auto& point = prompt.Points.front();
      static_cast<PointInteractor*>(interactor)->AddPoint(point, prompt.PromptType);
      return { "add_point", this->GetPointParams(point, inputAtTimeStep, include), nullptr };
    }

    case InteractionType::Box:
    {
      const auto* box = static_cast<BoxInteractor*>(interactor)->AddBox(plane, prompt.Points[0], prompt.Points[1], prompt.PromptType);
      return { "add_bbox", this->GetBoxParams(box->GetWorldControlPoint(0), box->GetWorldControlPoint(1), inputAtTimeStep, include), nullptr };
    }

    case InteractionType::Scribble:
    {
      auto* scribbleInteractor = static_cast<ScribbleInteractor*>(interactor);
      scribbleInteractor->AddScribble(plane, prompt.Points, prompt.PromptType);

      const auto mask = scribbleInteractor->GetLastScribbleMask();
      return { "add_scribble", this->GetMaskParams(scribbleInteractor->GetLastScribbleBoundingBox(), include), SharedBuffer::CopyOf(mask.GetPointer()) };
    }

    case InteractionType::Lasso:
    {
      auto* lassoInteractor = static_cast<LassoInteractor*>(interactor);
      lassoInteractor->AddLasso(plane, prompt.Points, prompt.PromptType);

      const auto mask = lassoInteractor->GetLastLassoMask();
      return { "add_lasso", this->GetMaskParams(lassoInteractor->GetLastLassoBoundingBox(), include), SharedBuffer::CopyOf(mask.GetPointer()) };
    }
  }

  mitkThrow() << "Unknown interaction type of a prompt.";
}

void mitk::nnInteractiveTool::Impl::ResetInteractions() const
{
  this->Worker->Call("nninteractive_worker.reset");
}

bool mitk::nnInteractiveTool::Impl::Undo() const
{
  return this->Worker->Call("nninteractive_worker.undo").get<bool>();
}
