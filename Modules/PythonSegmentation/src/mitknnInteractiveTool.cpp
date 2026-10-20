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
#include <mitkPythonHelper.h>
#include <mitkPythonWorker.h>
#include <mitkRenderingManager.h>
#include <mitkSharedBuffer.h>
#include <mitkToolManager.h>
#include <mitkTorchDevice.h>

#include "mitkInitializationHook.h"
#include "mitkPythonWorkerModules.h"

#include <usGetModuleContext.h>
#include <usModuleResource.h>

#include <algorithm>

using namespace mitk::nnInteractive;

namespace
{
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

    // Methods that call the worker are defined at the bottom of this file.
    void SetAutoZoom() const;
    void AddPointInteraction(const Point3D& point, const Image* inputAtTimeStep) const;
    void AddBoxInteraction(const PlanarFigure* box, const Image* inputAtTimeStep) const;
    void AddMaskInteraction(const std::string& method, const Image* mask, const InteractionBoundingBox* boundingBox) const;
    void AddInitialSegInteraction(MultiLabelSegmentation* previewImage, TimeStepType timeStep) const;
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
    }

    std::unique_ptr<PythonWorker> Worker;
    bool SessionStarted = false;
    SupportedInteractions Interactions;
    std::string License;
    bool SupportsUndo = false;

    // The session writes its predictions into this shared memory. TargetBuffer
    // is an image on the same memory, without a copy.
    std::unique_ptr<SharedBuffer> TargetSegment;
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
    // interaction, so one record of what it was (an interactor prompt, or the
    // initial-segmentation mask) is enough to remove the right visualization
    // on undo. UndoRefreshPending makes DoUpdatePreview repaint the preview
    // from the restored target buffer instead of adding an interaction.
    std::optional<nnInteractive::InteractionType> LastInteractionType;
    std::optional<nnInteractive::PromptType> LastInteractionPromptType;
    bool LastInteractionWasMask = false;
    bool UndoRefreshPending = false;

    // StartSession() processes events through this hook, during which the
    // tool may be deactivated, the session aborted or the time point changed.
    // Those are deferred until StartSession() returns.
    PythonWorker::ProgressHook StartHook;
    bool Starting = false;
    bool AbortRequested = false;
    bool TimePointChangedWhileStarting = false;

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
  // The worker is still on the stack of StartSession(), which tears down what
  // it set up once it has stopped. No interaction can exist before a session.
  if (m_Impl->Starting)
  {
    m_Impl->AbortRequested = true;
    Superclass::Deactivated();
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
  for (auto& [interactionType, interactor] : m_Impl->Interactors)
    interactor->Reset();

  m_Impl->InitialSeg = nullptr;
  m_Impl->ClearLastInteraction();

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
  if (!this->IsSessionRunning() || !this->CanUndo())
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

  // UpdatePreview() is synchronous, so the one-shot refresh has happened by
  // now. Clear the flag defensively: if UpdatePreview() returned before
  // reaching DoUpdatePreview (no input or preview image), a stale true would
  // make the next interaction take the refresh branch and be dropped silently.
  m_Impl->UndoRefreshPending = false;
}

bool mitk::nnInteractiveTool::CanUndo() const
{
  return this->IsSessionRunning() && m_Impl->HasUndoableInteraction();
}

bool mitk::nnInteractiveTool::GetAutoZoom() const
{
  return m_Impl->AutoZoom;
}

void mitk::nnInteractiveTool::SetAutoZoom(bool autoZoom)
{
  m_Impl->AutoZoom = autoZoom;

  if (this->IsSessionRunning())
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
  m_Impl->AutoRefine = autoRefine;
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

  try
  {
    if (interactor != nullptr)
    {
      switch (interactor->GetType())
      {
        case InteractionType::Point:
        {
          auto point = static_cast<const PointInteractor*>(interactor)->GetLastPoint();
          m_Impl->AddPointInteraction(point.value(), inputAtTimeStep);
          break;
        }
        case InteractionType::Box:
        {
          auto box = static_cast<const BoxInteractor*>(interactor)->GetLastBox();
          m_Impl->AddBoxInteraction(box, inputAtTimeStep);
          break;
        }
        case InteractionType::Scribble:
        {
          auto scribbleInteractor = static_cast<const ScribbleInteractor*>(interactor);
          auto mask = scribbleInteractor->GetLastScribbleMask();
          if (mask.IsNull())
          {
            MITK_WARN << "Skipping scribble preview update: no mask available.";
            return;
          }
          m_Impl->AddMaskInteraction("add_scribble", mask.GetPointer(), scribbleInteractor->GetLastScribbleBoundingBox());
          break;
        }
        case InteractionType::Lasso:
        {
          auto lassoInteractor = static_cast<const LassoInteractor*>(interactor);
          auto mask = lassoInteractor->GetLastLassoMask();
          if (mask.IsNull())
          {
            MITK_WARN << "Skipping lasso preview update: no mask available.";
            return;
          }
          m_Impl->AddMaskInteraction("add_lasso", mask.GetPointer(), lassoInteractor->GetLastLassoBoundingBox());
          break;
        }
        default:
          MITK_ERROR << "Cannot update preview because of unknown interaction type!";
          return;
      }

      previewImage->UpdateGroupImage(previewImage->GetActiveLayer(), m_Impl->TargetBuffer, timeStep, 0);

      // Record this interaction so UndoLastInteraction() knows which prompt to
      // remove. m_Impl->PromptType is the prompt type the enabled interactor
      // was activated with, i.e. the one used for this interaction.
      m_Impl->LastInteractionType = interactor->GetType();
      m_Impl->LastInteractionPromptType = m_Impl->PromptType;
      m_Impl->LastInteractionWasMask = false;

      this->PreviewUpdatedEvent.Send();
    }
    else if (m_Impl->InitialSeg.IsNotNull())
    {
      m_Impl->AddInitialSegInteraction(previewImage, timeStep);

      // The initial-segmentation mask is one undoable step (the session
      // snapshots before applying it); it has no prompt node to remove.
      m_Impl->LastInteractionType.reset();
      m_Impl->LastInteractionPromptType.reset();
      m_Impl->LastInteractionWasMask = true;
    }
    else
    {
      this->ResetPreviewContentAtTimeStep(timeStep);
      m_Impl->ClearLastInteraction();
    }
  }
  catch (const Exception& e)
  {
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
  // interactor, if any.
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

void mitk::nnInteractiveTool::StartSession(ProgressTask* task)
{
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

  m_Impl->Starting = true;
  m_Impl->AbortRequested = false;
  m_Impl->TimePointChangedWhileStarting = false;
  m_Impl->StartHook = MakeInitializationHook(this, task, [this]() { return m_Impl->AbortRequested; });

  const auto finishStart = [this]() {
    m_Impl->Starting = false;
    m_Impl->StartHook = nullptr;
  };

  try
  {
    m_Impl->Remote = prefs->Get("nnInteractive/inferenceMode", "local") == "remote";

    const auto python = PythonHelper::GetVirtualEnvExecutablePath(this->GetVirtualEnvName());

    if (python.empty())
      mitkThrow() << "nnInteractive is not installed.";

    m_Impl->Worker = std::make_unique<PythonWorker>(python, std::vector<PythonWorker::Module> {
      ReadWorkerModule("torch_device.py"),
      ReadWorkerModule("nnInteractive/nninteractive_worker.py")
    }, "nnInteractive");

    if (m_Impl->Remote)
      this->ConstructRemoteSession();
    else
      this->ConstructLocalSession();
  }
  catch (...)
  {
    finishStart();

    // If construction failed before a session was established, do not leave the
    // remote flag set: IsRemoteSession() must report false when nothing runs,
    // and EndSession() (the usual reset path) is a no-op without a session.
    if (!this->IsSessionRunning())
    {
      m_Impl->Worker.reset();
      m_Impl->Remote = false;
    }

    // Deactivated or aborted meanwhile: nothing of the start may survive.
    if (m_Impl->AbortRequested)
    {
      m_Impl->AbortRequested = false;
      this->EndSession();
    }

    throw;
  }

  finishStart();

  if (m_Impl->TimePointChangedWhileStarting)
  {
    m_Impl->TimePointChangedWhileStarting = false;
    this->OnTimePointChanged();
  }
}

void mitk::nnInteractiveTool::ConstructRemoteSession()
{
  auto prefs = GetPreferences();
  const auto serverUrl = prefs->Get("nnInteractive/serverUrl", "");
  const auto apiKey = prefs->Get("nnInteractive/apiKey", "");

  if (serverUrl.empty())
    mitkThrow() << "Remote mode is selected but no server URL is configured. "
                   "Click on Settings to configure a server URL.";

  m_Impl->ResetBackend();

  // Expected failures, like an unreachable server or a rejected API key, come
  // back as short user-facing messages. The server URL is shown without
  // credentials.
  const auto result = m_Impl->Worker->Call("nninteractive_worker.start_remote", {
    { "server_url", serverUrl },
    { "server_display", SanitizeUrlForDisplay(serverUrl) },
    { "api_key", apiKey }
  }, {}, m_Impl->StartHook);

  if (const auto error = result.value("error", std::string()); !error.empty())
    mitkThrow() << error;

  if (result.value("proxy_bypassed", false))
    MITK_INFO << "Connected to the nnInteractive server directly, bypassing the configured HTTP proxy.";

  m_Impl->SetCapabilities(result["capabilities"]);
  m_Impl->SessionStarted = true;

  // The lease is claimed. If configuring the session or uploading the image
  // fails because the connection dropped mid-flight, release the half-open
  // lease and surface a short message instead of a raw traceback.
  try
  {
    m_Impl->SetAutoZoom();
    this->BindSessionImageAndTargetBuffer();
  }
  catch (const Exception& e)
  {
    // The lease is already claimed, so any failure here must release it;
    // otherwise the server slot is held until the idle reaper expires it.
    const bool connectionLost = IsRemoteConnectionLost(e);

    this->EndSession();

    if (connectionLost)
      mitkThrow() << "Lost the connection to the nnInteractive server while starting the session. "
                     "Check the server and try again.";

    throw;
  }
}

void mitk::nnInteractiveTool::ConstructLocalSession()
{
  auto& worker = *m_Impl->Worker;

  // A client-only install (nninteractive-client) has no local inference backend
  // and no PyTorch. Fail fast with an actionable message instead of letting the
  // torch import below raise a raw traceback.
  if (!worker.Call("nninteractive_worker.is_local_inference_available", {}, {}, m_Impl->StartHook).get<bool>())
    mitkThrow() << "This is a client-only nnInteractive installation, which supports remote "
                   "sessions only. Select Remote in Preferences -> Segmentation -> nnInteractive, "
                   "or uninstall and reinitialize in Full mode to enable local inference.";

  auto prefs = GetPreferences();
  const auto backend = Torch::ParseBackendPreference(prefs->Get("nnInteractive/backend", "auto"));
  const auto gpuBackendPref = prefs->Get("nnInteractive/gpuBackend", "cuda:0");

  const auto deviceSelection = Torch::SelectDevice(worker, backend, gpuBackendPref, 6000, m_Impl->StartHook);
  const bool useCUDADevice = deviceSelection.SelectedBackend == Torch::Backend::CUDA;

  const auto modelSource = prefs->Get("nnInteractive/modelSource", "huggingface");
  const auto localModelPath = prefs->Get("nnInteractive/localModelPath", "");

  if (modelSource == "local" && localModelPath.empty())
    mitkThrow() << "nnInteractive: Local model mode is selected but no checkpoint folder is configured. "
                   "Set the path in Preferences -> Segmentation -> nnInteractive.";

  m_Impl->ResetBackend();

  if (!useCUDADevice)
    this->SetAutoZoom(false);

  // torch.compile is only meaningful on Linux with a CUDA device (the library
  // itself forces it off on Windows, and it relies on Triton/NVIDIA). The
  // useCUDADevice guard also neutralizes a stale preference, e.g. enabled and
  // then the backend switched to CPU or auto-fell back to CPU.
  bool useTorchCompile = false;
#if defined(__linux__)
  useTorchCompile = useCUDADevice && prefs->GetBool("nnInteractive/useTorchCompile", true);
#endif

  // Storage backend for the interaction tensor. "auto" (the library default)
  // uses a dense tensor for smaller images and blosc2 for larger ones; blosc2
  // trades speed for lower RAM, tensor the other way around. Only applies to
  // local sessions; a remote server decides this server-side.
  auto storageBackend = prefs->Get("nnInteractive/interactionsStorage", "auto");
  if (storageBackend != "blosc2" && storageBackend != "tensor")
    storageBackend = "auto";

  // The checkpoint is a local folder or managed by nnInteractive's model
  // management, which downloads it if needed and works offline once it has.
  // An empty checkpoint means the library's recommended default.
  const auto result = worker.Call("nninteractive_worker.start_local", {
    { "device", deviceSelection.Device },
    { "model_source", modelSource },
    { "local_model_path", localModelPath },
    { "model_checkpoint", prefs->Get("nnInteractive/modelCheckpoint", "") },
    { "use_torch_compile", useTorchCompile },
    { "do_autozoom", m_Impl->AutoZoom },
    { "interactions_storage", storageBackend }
  }, {}, m_Impl->StartHook);

  if (const auto error = result.value("error", std::string()); !error.empty())
    mitkThrow() << error;

  m_Impl->SetCapabilities(result["capabilities"]);
  m_Impl->SessionStarted = true;
  m_Impl->SetBackend(deviceSelection.SelectedBackend);

  try
  {
    this->BindSessionImageAndTargetBuffer();
  }
  catch (...)
  {
    this->EndSession();
    throw;
  }
}

void mitk::nnInteractiveTool::BindSessionImageAndTargetBuffer()
{
  auto image = this->GetToolManager()->GetReferenceData(0)->GetDataAs<Image>();

  const auto timePoint = this->GetToolManager()->GetCurrentTimePoint();
  const auto timeStep = image->GetTimeGeometry()->TimePointToTimeStep(timePoint);

  auto imageAtTimeStep = this->GetImageByTimeStep(image, timeStep);

  // The worker keeps its own mapping of the image, so this copy can go once
  // the session has it.
  const auto imageSegment = SharedBuffer::CopyOf(imageAtTimeStep);

  m_Impl->TargetBuffer = nullptr;
  m_Impl->TargetSegment = std::make_unique<SharedBuffer>(MultiLabelSegmentation::GetPixelType(), imageSegment->GetShape());
  m_Impl->TargetBuffer = m_Impl->TargetSegment->AsImage(*imageAtTimeStep->GetTimeGeometry());

  const auto spacing = imageAtTimeStep->GetGeometry()->GetSpacing();

  m_Impl->Worker->Call("nninteractive_worker.set_image",
    { { "spacing", { spacing[2], spacing[1], spacing[0] } } },
    { { "image", imageSegment.get() }, { "target", m_Impl->TargetSegment.get() } },
    m_Impl->StartHook);

  // Pin the session to the time steps that were active when it was started.
  // OnTimePointChanged() ends the session if either changes, since the Python
  // model is bound to a single 3D slice.
  m_Impl->SessionReferenceDataTimeStep = timeStep;

  const auto* workingSeg = this->GetTargetSegmentation();
  m_Impl->SessionWorkingDataTimeStep = workingSeg != nullptr
    ? workingSeg->GetTimeGeometry()->TimePointToTimeStep(timePoint)
    : 0;

  // A fresh session has nothing to undo (set_image resets the session's undo
  // state); keep the tool's record in sync.
  m_Impl->ClearLastInteraction();
  m_Impl->UndoRefreshPending = false;
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
  if (m_Impl->Starting)
  {
    m_Impl->AbortRequested = true;
    return;
  }

  this->DisableInteractor();

  // End the dead session first so ResetInteractions() below skips the remote
  // session.reset_interactions() call (guarded by IsSessionRunning()).
  this->EndSession();

  this->ResetInteractions();
  this->ResetPreviewContent();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::nnInteractiveTool::OnTimePointChanged()
{
  // The Python session is bound to a single 3D slice extracted at the time
  // step that was active when StartSession() ran. Without a session there is
  // nothing to reconcile, and DoUpdatePreview() short-circuits without a
  // session, so we deliberately do not call the base class handler (which
  // would invoke UpdatePreview against a stale binding). A session that is
  // being started is reconciled once StartSession() has finished.
  if (m_Impl->Starting)
  {
    m_Impl->TimePointChangedWhileStarting = true;
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

void mitk::nnInteractiveTool::Impl::SetAutoZoom() const
{
  this->Worker->Call("nninteractive_worker.set_autozoom", { { "enabled", this->AutoZoom } });
}

void mitk::nnInteractiveTool::Impl::AddPointInteraction(const Point3D& point, const Image* inputAtTimeStep) const
{
  itk::Index<3> index;
  inputAtTimeStep->GetGeometry()->WorldToIndex(point, index);

  this->Worker->Call("nninteractive_worker.add_point", {
    { "coordinates", { index[2], index[1], index[0] } },
    { "include", this->PromptType == PromptType::Positive }
  });
}

void mitk::nnInteractiveTool::Impl::AddBoxInteraction(const PlanarFigure* box, const Image* inputAtTimeStep) const
{
  const auto* geometry = inputAtTimeStep->GetGeometry();

  std::array<itk::Index<3>, 2> indices;
  geometry->WorldToIndex(box->GetWorldControlPoint(0), indices[0]);
  geometry->WorldToIndex(box->GetWorldControlPoint(2), indices[1]);

  auto bbox = nlohmann::json::array();

  for (int i = 2; i >= 0; --i)
  {
    // nnInteractive expects half-open bounding boxes [min, max).
    // Our indices are inclusive, so we add +1 to the upper bound.
    bbox.push_back({ std::min(indices[0][i], indices[1][i]), std::max(indices[0][i], indices[1][i]) + 1 });
  }

  this->Worker->Call("nninteractive_worker.add_bbox", {
    { "bbox", bbox },
    { "include", this->PromptType == PromptType::Positive }
  });
}

void mitk::nnInteractiveTool::Impl::AddMaskInteraction(const std::string& method, const Image* mask, const InteractionBoundingBox* boundingBox) const
{
  const auto maskSegment = SharedBuffer::CopyOf(mask);

  nlohmann::json params = { { "include", this->PromptType == PromptType::Positive } };

  if (boundingBox != nullptr)
    params["bbox"] = ToJson(*boundingBox);

  this->Worker->Call("nninteractive_worker." + method, params, { { "mask", maskSegment.get() } });
}

void mitk::nnInteractiveTool::Impl::AddInitialSegInteraction(MultiLabelSegmentation* previewImage, TimeStepType timeStep) const
{
  const auto maskTimeStep = this->InitialSeg->GetTimeSteps() > 1
    ? this->SessionWorkingDataTimeStep
    : 0;

  const auto maskSegment = SharedBuffer::CopyOf(this->InitialSeg, maskTimeStep);

  this->Worker->Call("nninteractive_worker.add_initial_seg",
    { { "run_prediction", this->AutoRefine } },
    { { "mask", maskSegment.get() } });

  previewImage->UpdateGroupImage(previewImage->GetActiveLayer(), this->TargetBuffer, timeStep, 0, ImageAccessorBase::IgnoreLock);
}

void mitk::nnInteractiveTool::Impl::ResetInteractions() const
{
  this->Worker->Call("nninteractive_worker.reset");
}

bool mitk::nnInteractiveTool::Impl::Undo() const
{
  return this->Worker->Call("nninteractive_worker.undo").get<bool>();
}
