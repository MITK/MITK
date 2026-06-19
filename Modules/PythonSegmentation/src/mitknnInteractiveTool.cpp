/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveTool.h>

#include <mitkCoreServices.h>
#include <mitkImageReadAccessor.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitknnInteractiveBoundingBox.h>
#include <mitknnInteractiveBoxInteractor.h>
#include <mitknnInteractiveLassoInteractor.h>
#include <mitknnInteractivePointInteractor.h>
#include <mitknnInteractiveScribbleInteractor.h>
#include <mitkPlanarFigure.h>
#include <mitkPythonContext.h>
#include <mitkRenderingManager.h>
#include <mitkToolManager.h>

#include <usGetModuleContext.h>
#include <usModuleResource.h>

#include <algorithm>
#include <regex>

using namespace mitk::nnInteractive;

namespace
{
  std::optional<int> parseCUDADevice(const std::string& gpuBackend)
  {
    const std::regex regex(R"(^\s*cuda:(\d+)\s*$)");
    std::smatch match;

    if (std::regex_match(gpuBackend, match, regex))
      return std::stoi(match[1].str());

    return std::nullopt;
  }

  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }

  // Format a std::string as a Python single-quoted string literal so a Windows
  // path like C:\foo\bar round-trips safely through the generated Python code.
  std::string PyQuote(const std::string& value)
  {
    std::string result;
    result.reserve(value.size() + 2);
    result.push_back('\'');
    for (char c : value)
    {
      if (c == '\\' || c == '\'')
        result.push_back('\\');
      result.push_back(c);
    }
    result.push_back('\'');
    return result;
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

  // Stable sentinel used to recognize a lost remote connection. We raise it
  // ourselves (see WrapInRemoteGuard) after a type-based catch, so detection
  // never depends on httpx or OS error wording.
  constexpr auto REMOTE_CONNECTION_LOST_SENTINEL = "NNI_REMOTE_CONNECTION_LOST";

  // Wrap generated Python that talks to the remote session so that any httpx
  // error (httpx.HTTPError is the base of every transport and status error) or
  // lease error is caught by type and re-raised as our own sentinel. The
  // session-error imports (httpx, ServerAtCapacityError, SessionExpiredError)
  // are made by ConstructRemoteSession() before any session call runs.
  std::string WrapInRemoteGuard(const std::string& code)
  {
    std::ostringstream out;
    out << "try:\n";
    std::istringstream in(code);
    for (std::string line; std::getline(in, line);)
      out << "    " << line << '\n';
    out << "except (httpx.HTTPError, ServerAtCapacityError, SessionExpiredError) as _nni_e:\n"
        << "    raise RuntimeError('" << REMOTE_CONNECTION_LOST_SENTINEL << ": ' + repr(_nni_e))\n";
    return out.str();
  }

  // mitk::Exception::GetDescription() may return nullptr; treat that as an empty
  // message so the remote-error helpers do not have to repeat the null-guard.
  std::string Description(const mitk::Exception& e)
  {
    const char* description = e.GetDescription();
    return description != nullptr ? description : "";
  }

  // Heartbeat cadence: beat at half the server's liveness timeout, but never less
  // often than every 5 s. Mirrors the client library's own _heartbeat_loop so a
  // single dropped beat still leaves margin before the lease is reaped.
  constexpr double HEARTBEAT_FRACTION_OF_LIVENESS = 0.5;
  constexpr double MIN_HEARTBEAT_INTERVAL_SEC = 5.0;
}

namespace mitk
{
  MITK_TOOL_MACRO(MITKPYTHONSEGMENTATION_EXPORT, nnInteractiveTool, "nnInteractive")

  class nnInteractiveTool::Impl
  {
  public:
    Impl()
      : TargetBuffer(Image::New()),
        PromptType(PromptType::Positive),
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

    PythonContext* GetPythonContext() const
    {
      return m_PythonContext.get();
    }

    void CreatePythonContext(const std::string& venvName)
    {
      m_PythonContext = std::make_unique<PythonContext>(venvName);
      m_PythonContext->Activate();
    }

    void DestroyPythonContext()
    {
      m_PythonContext = nullptr;
    }

    // Executes Python that drives the inference session. For remote sessions the
    // code is wrapped (see WrapInRemoteGuard) so httpx/lease failures surface as
    // our own stable sentinel; for local sessions it runs verbatim.
    void ExecuteSession(const std::string& code) const
    {
      m_PythonContext->Execute(this->Remote ? WrapInRemoteGuard(code) : code);
    }

    // Methods that execute Python code are defined at the bottom of this file.
    void SetAutoZoom() const;
    void AddPointInteraction(const Point3D& point, const Image* inputAtTimeStep) const;
    void AddBoxInteraction(const PlanarFigure* box, const Image* inputAtTimeStep) const;
    void AddScribbleInteraction(const Image* mask, const InteractionBoundingBox* boundingBox) const;
    void AddLassoInteraction(const Image* mask, const InteractionBoundingBox* boundingBox) const;
    void AddInitialSegInteraction(MultiLabelSegmentation* previewImage, TimeStepType timeStep) const;
    void ResetInteractions() const;
    bool Undo() const;

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

    Image::Pointer TargetBuffer;
    nnInteractive::PromptType PromptType;
    InteractorMap Interactors;
    Image::Pointer InitialSeg;
    bool AutoZoom;
    bool AutoRefine;
    bool Remote = false;
    int HeartbeatIntervalMs = 0;
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

  private:
    std::optional<Backend> m_Backend;
    std::unique_ptr<PythonContext> m_PythonContext;
  };
}

mitk::nnInteractiveTool::nnInteractiveTool()
  : SegWithPreviewTool(true),
    m_Impl(std::make_unique<Impl>())
{
  this->KeepActiveAfterAcceptOn();
  this->ResetsToEmptyPreviewOn();
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
  this->DisableInteractor();
  this->ResetInteractions();

  this->EndSession();

  Superclass::Deactivated();

  this->DeactivatedEvent.Send();
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
      // The local prompts are already cleared above. If the remote reset failed
      // because the connection is gone, there is nothing left to reset on the
      // server; the loss is surfaced on the next interaction, or by the heartbeat
      // timer if one is running.
      if (!this->IsRemoteConnectionError(Description(e)))
        throw;

      MITK_WARN << "nnInteractive: could not reset interactions on the remote server (connection lost).";
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
    // A remote session can fail here (lease expired, server gone, or at
    // capacity). Tear it down and notify the GUI; otherwise propagate as the
    // other session operations do.
    if (this->HandleSessionError(Description(e)))
      return;

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
      // Non-critical: if the connection dropped, the loss is surfaced on the
      // next interaction, or by the heartbeat timer if one is running.
      if (!this->IsRemoteConnectionError(Description(e)))
        throw;

      MITK_WARN << "nnInteractive: could not update auto-zoom on the remote server (connection lost).";
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

  if (previewImage == nullptr || m_Impl->GetPythonContext() == nullptr)
    return;

  this->SetPreviewLabel(1, this->GetSpecialPreviewColor());

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
          m_Impl->AddScribbleInteraction(mask.GetPointer(), scribbleInteractor->GetLastScribbleBoundingBox());
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
          m_Impl->AddLassoInteraction(mask.GetPointer(), lassoInteractor->GetLastLassoBoundingBox());
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
    // A remote session can fail mid-interaction (lease expired, server gone, or
    // at capacity). Tear it down and notify the GUI; otherwise propagate the
    // error as before so local failures keep their existing behavior.
    if (this->HandleSessionError(Description(e)))
      return;

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

bool mitk::nnInteractiveTool::CreatePythonContext()
{
  try
  {
    m_Impl->CreatePythonContext(this->GetVirtualEnvName());
  }
  catch (const Exception& e)
  {
    MITK_ERROR << e.GetDescription();
    return false;
  }

  return true;
}

mitk::PythonContext* mitk::nnInteractiveTool::GetPythonContext() const
{
  return m_Impl->GetPythonContext();
}

////////////////////////////////////////////////////////////////////////////////
// Functions and methods that execute Python code
////////////////////////////////////////////////////////////////////////////////

bool mitk::nnInteractiveTool::IsInstalled() const
{
  std::ostringstream pyCommands; pyCommands
    << "import importlib.util\n"
    << "is_installed = importlib.util.find_spec('nnInteractive') is not None\n";

  auto pythonContext = m_Impl->GetPythonContext();
  pythonContext->Execute(pyCommands.str());

  return pythonContext->GetVariableAsBool("is_installed").value_or(false);
}

mitk::nnInteractiveTool::VersionCheckResult mitk::nnInteractiveTool::CheckInstalledVersion(bool checkForUpdate) const
{
  VersionCheckResult result;

  const std::string spec = std::string(">=") + MINIMUM_VERSION + ",<" + MAXIMUM_VERSION_EXCLUSIVE;

  // Determine the offline verdict (installed vs. minimum) first and only reach
  // out to PyPI when the installed version is already supported, so a too-old
  // package is flagged instantly and a missing network never delays the result.
  // packaging ships with pip/torch, so the imports are expected to succeed; any
  // failure leaves nni_installed empty and the caller treats it as Unknown.
  std::ostringstream pyCommands; pyCommands
    << "nni_installed = ''\n"
    << "nni_latest = ''\n"
    << "nni_below_min = False\n"
    << "nni_update_available = False\n"
    << "try:\n"
    << "    from importlib.metadata import version\n"
    << "    from packaging.version import Version\n"
    << "    nni_installed = version('nnInteractive')\n"
    << "    nni_below_min = Version(nni_installed) < Version('" << MINIMUM_VERSION << "')\n";

  if (checkForUpdate)
  {
    pyCommands
      << "    if not nni_below_min:\n"
      << "        try:\n"
      << "            from packaging.specifiers import SpecifierSet\n"
      << "            import urllib.request, json\n"
      << "            with urllib.request.urlopen('https://pypi.org/pypi/nnInteractive/json', timeout=5) as _r:\n"
      << "                _data = json.load(_r)\n"
      << "            _spec = SpecifierSet('" << spec << "')\n"
      // Consider only releases with installable, non-yanked files. PyPI's
      // 'releases' map keeps every version ever registered, including yanked or
      // fileless ones that pip would never install, so filtering by the version
      // string alone could advertise an update that cannot actually be had.
      << "            _cands = [v for v, _files in _data['releases'].items()\n"
      << "                      if _files and not all(_f.get('yanked') for _f in _files)\n"
      << "                      and _spec.contains(v, prereleases=False)]\n"
      << "            if _cands:\n"
      << "                nni_latest = str(max(_cands, key=Version))\n"
      << "                nni_update_available = Version(nni_latest) > Version(nni_installed)\n"
      << "        except Exception:\n"
      << "            nni_latest = ''\n"
      << "            nni_update_available = False\n";
  }

  pyCommands
    << "except Exception:\n"
    << "    nni_installed = ''\n";

  auto pythonContext = m_Impl->GetPythonContext();

  try
  {
    pythonContext->Execute(pyCommands.str());
  }
  catch (...)
  {
    return result; // Status stays Unknown.
  }

  result.Installed = pythonContext->GetVariableAsString("nni_installed").value_or("");
  result.Latest = pythonContext->GetVariableAsString("nni_latest").value_or("");

  if (result.Installed.empty())
    return result; // Unknown: could not read the installed version.

  if (pythonContext->GetVariableAsBool("nni_below_min").value_or(false))
    result.Status = VersionStatus::BelowMinimum;
  else if (pythonContext->GetVariableAsBool("nni_update_available").value_or(false))
    result.Status = VersionStatus::UpdateAvailable;
  else
    result.Status = VersionStatus::UpToDate;

  return result;
}

bool mitk::nnInteractiveTool::GetCUDADeviceInfo(CUDADeviceInfo& info) const
{
  auto prefs = GetPreferences();
  const auto gpuBackend = prefs->Get("nnInteractive/gpuBackend", "cuda:0");
  auto cudaDevice = parseCUDADevice(gpuBackend);

  if (!cudaDevice.has_value())
    return false;

  std::ostringstream pyCommands; pyCommands
    << "import torch\n"
    << "is_cuda_available = False\n"
    << "try:\n"
    << "    if torch.cuda.is_available():\n"
    << "        name = torch.cuda.get_device_name(" << cudaDevice.value() << ")\n"
    << "        props = torch.cuda.get_device_properties(" << cudaDevice.value() << ")\n"
    << "        total_memory_mb = props.total_memory // (1024 ** 2)\n"
    << "        major = props.major\n"
    << "        minor = props.minor\n"
    << "        is_cuda_available = True\n"
    << "except Exception:\n"
    << "    pass\n";

  try
  {
    auto pythonContext = m_Impl->GetPythonContext();
    pythonContext->Execute(pyCommands.str());

    if (!pythonContext->GetVariableAsBool("is_cuda_available").value_or(false))
      return false;

    info.Name = pythonContext->GetVariableAsString("name").value_or("");
    info.TotalMemoryMB = pythonContext->GetVariableAsInt("total_memory_mb").value_or(0);
    info.Major = pythonContext->GetVariableAsInt("major").value_or(0);
    info.Minor = pythonContext->GetVariableAsInt("minor").value_or(0);

    return true;
  }
  catch (...)
  {
    return false;
  }
}

void mitk::nnInteractiveTool::StartSession()
{
  if (this->IsSessionRunning())
    this->EndSession();

  auto prefs = GetPreferences();
  m_Impl->Remote = prefs->Get("nnInteractive/inferenceMode", "local") == "remote";

  try
  {
    if (m_Impl->Remote)
      this->ConstructRemoteSession();
    else
      this->ConstructLocalSession();
  }
  catch (...)
  {
    // If construction failed before a session was established, do not leave the
    // remote flag set: IsRemoteSession() must report false when nothing runs,
    // and EndSession() (the usual reset path) is a no-op without a session.
    if (!this->IsSessionRunning())
      m_Impl->Remote = false;

    throw;
  }
}

void mitk::nnInteractiveTool::ConstructRemoteSession()
{
  auto prefs = GetPreferences();
  const auto serverUrl = prefs->Get("nnInteractive/serverUrl", "");
  const auto apiKey = prefs->Get("nnInteractive/apiKey", "");

  if (serverUrl.empty())
    mitkThrow() << "Remote mode is selected but no server URL is configured. "
                   "Set it in Preferences -> Segmentation -> nnInteractive.";

  m_Impl->ResetBackend();

  auto pythonContext = m_Impl->GetPythonContext();

  // Claim a session on the server. Map the expected failure modes to short,
  // user-facing messages (reported via the nni_connect_error variable) instead
  // of letting an httpx/Python traceback bubble up to the GUI. The server URL is
  // shown credential-stripped (nni_server_display), and the API key is cleared in
  // a finally so it never lingers in the shared dictionary even if a statement
  // here throws.
  {
    std::ostringstream pyCommands; pyCommands
      << "nni_server_url = " << PyQuote(serverUrl) << "\n"
      << "nni_server_display = " << PyQuote(SanitizeUrlForDisplay(serverUrl)) << "\n"
      << "nni_api_key = " << (apiKey.empty() ? std::string("None") : PyQuote(apiKey)) << "\n"
      << "nni_connect_error = ''\n"
      << "try:\n"
      << "    from nnInteractive.inference.remote import (\n"
      << "        nnInteractiveRemoteInferenceSession, ServerAtCapacityError, SessionExpiredError)\n"
      << "    import httpx\n"
      << "    try:\n"
      << "        session = nnInteractiveRemoteInferenceSession(server_url=nni_server_url, api_key=nni_api_key)\n"
      << "    except ServerAtCapacityError:\n"
      << "        nni_connect_error = 'The nnInteractive server is at capacity. Please try again later.'\n"
      << "    except SessionExpiredError:\n"
      << "        nni_connect_error = 'The nnInteractive server rejected the session request. Please try again.'\n"
      << "    except httpx.HTTPStatusError as _e:\n"
      << "        if _e.response.status_code == 401:\n"
      << "            nni_connect_error = 'The nnInteractive server rejected the API key. Check the API key in the nnInteractive preferences.'\n"
      << "        elif 'text/html' in _e.response.headers.get('content-type', ''):\n"
      << "            nni_connect_error = (f'The server at {nni_server_display} returned an HTML page instead of a response. '\n"
      << "                                 'An HTTP proxy may be intercepting the request; try adding the host to NO_PROXY.')\n"
      << "        else:\n"
      << "            nni_connect_error = f'The nnInteractive server returned an error (HTTP {_e.response.status_code}).'\n"
      << "    except (httpx.ConnectError, httpx.ConnectTimeout):\n"
      << "        nni_connect_error = f'Could not reach the nnInteractive server at {nni_server_display}. Check the server URL and make sure the server is running.'\n"
      << "    except httpx.HTTPError as _e:\n"
      << "        nni_connect_error = f'Could not connect to the nnInteractive server at {nni_server_display}: {_e}'\n"
      << "    except (ValueError, KeyError):\n"
      << "        nni_connect_error = ('The nnInteractive server returned an unexpected response. '\n"
      << "                             'An HTTP proxy or captive portal may be intercepting the request; '\n"
      << "                             'check the server URL and NO_PROXY.')\n"
      << "except ImportError:\n"
      << "    nni_connect_error = ('The installed nnInteractive does not support remote mode. '\n"
      << "                         'Reinstall or upgrade nnInteractive (remote mode requires v2.3.2 or newer).')\n"
      << "finally:\n"
      << "    del nni_api_key\n";
    pythonContext->Execute(pyCommands.str());
  }

  const auto connectError = pythonContext->GetVariableAsString("nni_connect_error").value_or("");

  pythonContext->Execute("del nni_server_url, nni_server_display, nni_connect_error\n");

  if (!connectError.empty())
    mitkThrow() << connectError;

  // The lease is claimed. If configuring the session or uploading the image
  // fails because the connection dropped mid-flight, release the half-open
  // lease and surface a short message instead of a raw traceback.
  try
  {
    m_Impl->ExecuteSession(
      std::string("session.set_do_autozoom(") + (m_Impl->AutoZoom ? "True" : "False") + ")\n");

    this->BindSessionImageAndTargetBuffer();

    // The client's background heartbeat daemon is starved in our embedded
    // interpreter (the GIL stays on the Qt main thread between Execute calls),
    // so the GUI drives the heartbeat from a timer instead. Read the
    // server-provided liveness timeout and beat at half of it, mirroring the
    // library's own cadence. A zero/disabled liveness timeout means no
    // heartbeat is needed.
    pythonContext->Execute(
      "nni_liveness = float(getattr(session, 'liveness_timeout_seconds', 0.0) or 0.0)\n");
    const auto liveness = pythonContext->GetVariableAsDouble("nni_liveness").value_or(0.0);
    pythonContext->Execute("del nni_liveness\n");
    m_Impl->HeartbeatIntervalMs = liveness > 0.0
      ? static_cast<int>(std::max(MIN_HEARTBEAT_INTERVAL_SEC, liveness * HEARTBEAT_FRACTION_OF_LIVENESS) * 1000.0)
      : 0;
  }
  catch (const Exception& e)
  {
    // The lease is already claimed, so any failure here must release it;
    // otherwise the server slot is held until the idle reaper expires it.
    // Classify before EndSession() clears the remote flag.
    const bool connectionLost =
      this->IsRemoteConnectionError(Description(e));

    this->EndSession();

    if (connectionLost)
      mitkThrow() << "Lost the connection to the nnInteractive server while starting the session. "
                     "Check the server and try again.";

    throw;
  }
}

void mitk::nnInteractiveTool::ConstructLocalSession()
{
  auto pythonContext = m_Impl->GetPythonContext();
  bool useCUDADevice = false;

  auto prefs = GetPreferences();
  const auto backendPref = prefs->Get("nnInteractive/backend", "auto");
  const auto gpuBackendPref = prefs->Get("nnInteractive/gpuBackend", "cuda:0");

  if (backendPref != "cpu")
  {
    CUDADeviceInfo deviceInfo;

    if (this->GetCUDADeviceInfo(deviceInfo))
    {
      MITK_INFO << "Found CUDA device: " << deviceInfo.Name;
      MITK_INFO << "  Compute capability: " << deviceInfo.Major << "." << deviceInfo.Minor;
      MITK_INFO << "  Total memory: " << deviceInfo.TotalMemoryMB << " MB";

      bool switchToCUDADevice = true;

      if (deviceInfo.Major < 6)
      {
        MITK_WARN << "Minimum required compute capability is 6.0";
        switchToCUDADevice = false;
      }

      if (deviceInfo.TotalMemoryMB < 6000)
      {
        MITK_WARN << "Minimum required total memory is 6 GB";
        switchToCUDADevice = false;
      }

      useCUDADevice = switchToCUDADevice;
    }

    if (!useCUDADevice)
    {
      if (backendPref == "auto")
      {
        MITK_WARN << "No compatible CUDA device detected. Falling back to CPU processing.";
      }
      else
      {
        MITK_WARN << "CPU backend would have been auto-selected, but the CUDA backend has been manually enforced. "
                  << "Continue at your own risk.";
        useCUDADevice = true;
      }
    }
  }

  {
    const auto modelSource = prefs->Get("nnInteractive/modelSource", "huggingface");

    std::ostringstream pyCommands; pyCommands
      << "import torch\n"
      << "import nnInteractive\n"
      << "from importlib.metadata import version\n"
      << "from pathlib import Path\n"
      << "from nnunetv2.utilities.find_class_by_name import recursive_find_python_class\n"
      << "from batchgenerators.utilities.file_and_folder_operations import join, load_json\n"
      << "print(f'nnInteractive version: {version(\"nnInteractive\")}')\n";

    if (modelSource == "local")
    {
      const auto localPath = prefs->Get("nnInteractive/localModelPath", "");

      if (localPath.empty())
        mitkThrow() << "nnInteractive: Local model mode is selected but no checkpoint folder is configured. "
                       "Set the path in Preferences -> Segmentation -> nnInteractive.";

      pyCommands
        << "print('Model source: local folder')\n"
        << "checkpoint_path = Path(" << PyQuote(localPath) << ")\n"
        << "if not checkpoint_path.is_dir():\n"
        << "    raise RuntimeError(f'nnInteractive checkpoint folder not found: {checkpoint_path}')\n";
    }
    else
    {
      const auto modelCheckpoint = prefs->Get("nnInteractive/modelCheckpoint", "nnInteractive_v1.0");

      pyCommands
        << "from huggingface_hub import snapshot_download\n"
        << "print('Model source: Hugging Face')\n"
        << "print('Model checkpoint: " << modelCheckpoint << "')\n"
        << "repo_id = 'nnInteractive/nnInteractive'\n"
        << "download_path = snapshot_download(\n"
        << "    repo_id = repo_id,\n"
        << "    allow_patterns = ['" << modelCheckpoint << "/*'],\n"
        << "    force_download = False\n"
        << ")\n"
        << "checkpoint_path = Path(download_path).joinpath('" << modelCheckpoint << "')\n";
    }

    pythonContext->Execute(pyCommands.str());
  }

  {
    // Set dummy nnU-Net paths to suppress warnings. These variables are required
    // by nnU-Net for training workflows but are not needed for inference here.
    std::ostringstream pyCommands; pyCommands
      << "os.environ.setdefault('nnUNet_raw', '/tmp/nnUNet/raw')\n"
      << "os.environ.setdefault('nnUNet_preprocessed', '/tmp/nnUNet/preprocessed')\n"
      << "os.environ.setdefault('nnUNet_results', '/tmp/nnUNet/results')\n";
    pythonContext->Execute(pyCommands.str());
  }

  {
    std::ostringstream pyCommands; pyCommands
      << "if Path(checkpoint_path).joinpath('inference_session_class.json').is_file():\n"
      << "    inference_class = load_json(\n"
      << "        Path(checkpoint_path).joinpath('inference_session_class.json'))\n"
      << "    if isinstance (inference_class, dict):\n"
      << "        inference_class = inference_class['inference_class']\n"
      << "else:\n"
      << "    inference_class = 'nnInteractiveInferenceSession'\n"
      << "inference_class = recursive_find_python_class(\n"
      << "    join(nnInteractive.__path__[0], 'inference'),\n"
      << "    inference_class,\n"
      << "    'nnInteractive.inference'\n"
      << ")\n";
    pythonContext->Execute(pyCommands.str());
  }

  m_Impl->ResetBackend();

  if (!useCUDADevice)
    this->SetAutoZoom(false);

  // torch.compile is only meaningful on Linux with a CUDA device (the library
  // itself forces it off on Windows, and it relies on Triton/NVIDIA). The
  // useCUDADevice guard also neutralizes a stale preference, e.g. enabled and
  // then the backend switched to CPU or auto-fell back to CPU.
  bool useTorchCompile = false;
#if defined(__linux__)
  useTorchCompile = useCUDADevice && prefs->GetBool("nnInteractive/useTorchCompile", false);
#endif

  // Storage backend for the interaction tensor. "auto" (the library default)
  // uses a dense tensor for smaller images and blosc2 for larger ones; blosc2
  // trades speed for lower RAM, tensor the other way around. Only applies to
  // local sessions; a remote server decides this server-side.
  auto storageBackend = prefs->Get("nnInteractive/interactionsStorage", "auto");
  if (storageBackend != "blosc2" && storageBackend != "tensor")
    storageBackend = "auto";

  {
    std::ostringstream pyCommands; pyCommands
      << "session = inference_class(\n"
      << "    device=torch.device('" << (useCUDADevice ? gpuBackendPref : "cpu") << "'),\n"
      << "    use_torch_compile=" << (useTorchCompile ? "True" : "False") << ",\n"
      << "    torch_n_threads=os.cpu_count(),\n"
      << "    verbose=False,\n"
      << "    do_autozoom=" << m_Impl->AutoZoom << ",\n"
      << "    interactions_storage='" << storageBackend << "'\n"
      << ")\n"
      << "session.initialize_from_trained_model_folder(checkpoint_path)\n";
    pythonContext->Execute(pyCommands.str());
  }

  m_Impl->SetBackend(useCUDADevice
    ? Backend::CUDA
    : Backend::CPU);

  this->BindSessionImageAndTargetBuffer();
}

void mitk::nnInteractiveTool::BindSessionImageAndTargetBuffer()
{
  auto pythonContext = m_Impl->GetPythonContext();

  auto image = this->GetToolManager()->GetReferenceData(0)->GetDataAs<Image>();

  const auto timePoint = this->GetToolManager()->GetCurrentTimePoint();
  const auto timeStep = image->GetTimeGeometry()->TimePointToTimeStep(timePoint);

  auto imageAtTimeStep = this->GetImageByTimeStep(image, timeStep);

  const auto maskPixelType = MultiLabelSegmentation::GetPixelType();
  m_Impl->TargetBuffer->Initialize(maskPixelType, *(imageAtTimeStep->GetTimeGeometry()));
  m_Impl->TargetBuffer->AllocateZeroedVolume();

  pythonContext->BindImage(imageAtTimeStep, "mitk_image");
  pythonContext->BindImage(m_Impl->TargetBuffer.GetPointer(), "mitk_target_buffer");

  std::ostringstream pyCommands; pyCommands
    << "image = mitk_image.as_numpy(writeable=True)\n"
    << "spacing = list(reversed(mitk_image.spacing))\n"
    << "target_buffer = mitk_target_buffer.as_numpy(writeable=True)\n";

  if (m_Impl->Remote)
  {
    // The remote session writes prediction diffs straight into the numpy view
    // of our C++ target buffer, so hand it the numpy array directly; unlike the
    // local path it needs no torch tensor wrapper (the server holds the model).
    pyCommands
      << "session.set_image(image[None], {'spacing': spacing})\n"
      << "session.set_target_buffer(target_buffer)\n";
  }
  else
  {
    pyCommands
      << "torch_target_buffer = torch.from_numpy(target_buffer)\n"
      << "session.set_image(image[None], {'spacing': spacing})\n"
      << "session.set_target_buffer(torch_target_buffer)\n";
  }

  m_Impl->ExecuteSession(pyCommands.str());

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

  if (m_Impl->Remote)
  {
    // Release the server lease right away so the slot frees up for other users
    // instead of waiting for the idle reaper. close() is best-effort and
    // idempotent server-side; swallow errors so teardown always completes.
    try
    {
      m_Impl->GetPythonContext()->Execute(
        "session.close()\n"
        "del session\n");
    }
    catch (const Exception& e)
    {
      MITK_WARN << "nnInteractive: error while releasing the remote session (ignored): "
                << e.GetDescription();
    }
  }
  else
  {
    std::ostringstream pyCommands; pyCommands
      << "session._reset_session()\n"
      << "del session.network\n"
      << "del session\n";

    if (m_Impl->GetBackend() == Backend::CUDA)
      pyCommands << "torch.cuda.empty_cache()\n";

    m_Impl->GetPythonContext()->Execute(pyCommands.str());
  }

  m_Impl->DestroyPythonContext();

  m_Impl->Remote = false;
  m_Impl->HeartbeatIntervalMs = 0;
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

int mitk::nnInteractiveTool::GetHeartbeatIntervalMs() const
{
  return m_Impl->HeartbeatIntervalMs;
}

void mitk::nnInteractiveTool::Heartbeat()
{
  if (!m_Impl->Remote || !this->IsSessionRunning())
    return;

  auto pythonContext = m_Impl->GetPythonContext();
  bool expired = false;

  try
  {
    // Mirror the library's own _heartbeat_loop tolerance: only a definitive
    // SessionExpiredError means the lease is gone. Everything else is non-fatal,
    // matching the library, which keeps the session alive and surfaces a real
    // expiry on the user's next action: a transient httpx error, a
    // ServerAtCapacityError (a 503 on a beat), and a malformed response
    // (json.JSONDecodeError) all just let the next beat retry. We classify inside
    // Python and read back a flag, so a transient blip never reaches the broad
    // remote guard (WrapInRemoteGuard) that would tear the session down.
    // SessionExpiredError was imported by ConstructRemoteSession() and persists in
    // the shared dictionary for the session's lifetime.
    pythonContext->Execute(
      "try:\n"
      "    session.heartbeat()\n"
      "    nni_hb_expired = False\n"
      "except SessionExpiredError:\n"
      "    nni_hb_expired = True\n"
      "except Exception:\n"
      "    nni_hb_expired = False\n");

    expired = pythonContext->GetVariableAsBool("nni_hb_expired").value_or(false);
    pythonContext->Execute("del nni_hb_expired\n");
  }
  catch (const Exception& e)
  {
    // Heartbeat() runs from a Qt timer slot, so never let an exception escape.
    MITK_WARN << "nnInteractive: heartbeat failed unexpectedly (ignored): " << e.GetDescription();
    return;
  }

  if (expired)
  {
    // The lease is gone server-side. Signal the GUI, which tears the dead
    // session down on the next event-loop tick (same deferred path used for a
    // connection loss detected mid-interaction).
    this->SessionExpiredEvent.Send();
  }
}

mitk::nnInteractiveTool::SupportedInteractions mitk::nnInteractiveTool::GetSupportedInteractions() const
{
  SupportedInteractions caps;

  if (!this->IsSessionRunning())
    return caps;

  try
  {
    auto pythonContext = m_Impl->GetPythonContext();

    pythonContext->Execute(
      "_nni_caps = session.supported_interactions\n"
      "support_points = bool(_nni_caps.get('points', False))\n"
      "support_box = bool(_nni_caps.get('bbox2d', False) or _nni_caps.get('bbox3d', False))\n"
      "support_scribble = bool(_nni_caps.get('scribble', False))\n"
      "support_lasso = bool(_nni_caps.get('lasso', False))\n"
      "support_mask = bool(session.supports_initial_label)\n");

    // value_or(true) is only the read-failure fallback: a genuine "unsupported"
    // answer already arrives as an explicit False from the bool(...) coercion
    // above, so a broken read-back fails open (keeps the button usable) rather
    // than silently disabling a supported interaction.
    caps.Point = pythonContext->GetVariableAsBool("support_points").value_or(true);
    caps.Box = pythonContext->GetVariableAsBool("support_box").value_or(true);
    caps.Scribble = pythonContext->GetVariableAsBool("support_scribble").value_or(true);
    caps.Lasso = pythonContext->GetVariableAsBool("support_lasso").value_or(true);
    caps.Mask = pythonContext->GetVariableAsBool("support_mask").value_or(true);

    pythonContext->Execute(
      "del _nni_caps, support_points, support_box, support_scribble, support_lasso, support_mask\n");
  }
  catch (const Exception& e)
  {
    MITK_WARN << "nnInteractive: could not read session capabilities: " << e.GetDescription();
  }

  return caps;
}

std::optional<std::string> mitk::nnInteractiveTool::GetModelLicense() const
{
  if (!this->IsSessionRunning())
    return std::nullopt;

  try
  {
    auto pythonContext = m_Impl->GetPythonContext();

    // session.license is a cached attribute on both local and remote sessions
    // (the remote one reads it from the server's /capabilities during
    // construction), so this is a plain local attribute read: no network call
    // and no remote guard needed.
    pythonContext->Execute("nni_license = getattr(session, 'license', None) or ''\n");
    const auto license = pythonContext->GetVariableAsString("nni_license").value_or("");
    pythonContext->Execute("del nni_license\n");

    if (license.empty())
      return std::nullopt;

    return license;
  }
  catch (const Exception& e)
  {
    MITK_WARN << "nnInteractive: could not read the model license: " << e.GetDescription();
    return std::nullopt;
  }
}

bool mitk::nnInteractiveTool::SupportsUndo() const
{
  if (!this->IsSessionRunning())
    return false;

  try
  {
    auto pythonContext = m_Impl->GetPythonContext();

    // supports_undo is a plain attribute on both local and remote sessions
    // (the remote one mirrors it from the server's capabilities), so this is a
    // local attribute read: no network call and no remote guard needed.
    pythonContext->Execute("nni_supports_undo = bool(getattr(session, 'supports_undo', False))\n");
    const auto supported = pythonContext->GetVariableAsBool("nni_supports_undo").value_or(false);
    pythonContext->Execute("del nni_supports_undo\n");

    return supported;
  }
  catch (const Exception& e)
  {
    MITK_WARN << "nnInteractive: could not read undo support: " << e.GetDescription();
    return false;
  }
}

bool mitk::nnInteractiveTool::IsRemoteConnectionError(const std::string& message) const
{
  // ExecuteSession() wraps remote session calls in a Python try/except that
  // catches httpx.HTTPError (the base of every httpx transport/status error)
  // and the typed lease errors, then re-raises our own stable sentinel. So a
  // robust check is just for that sentinel -- no guessing at httpx or OS error
  // wording.
  return m_Impl->Remote && message.find(REMOTE_CONNECTION_LOST_SENTINEL) != std::string::npos;
}

bool mitk::nnInteractiveTool::HandleSessionError(const std::string& errorMessage)
{
  if (!this->IsRemoteConnectionError(errorMessage))
    return false;

  // Do not tear down here: this runs while an interactor is mid-event, where
  // disabling/resetting interactors is unsafe. Signal the GUI, which calls
  // AbortSession() on the next event-loop tick.
  this->SessionExpiredEvent.Send();

  return true;
}

void mitk::nnInteractiveTool::AbortSession()
{
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
  // nothing to reconcile, and DoUpdatePreview() short-circuits when no
  // Python context exists, so we deliberately do not call the base class
  // handler (which would invoke UpdatePreview against a stale binding).
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
  this->ResetInteractions();
  this->ResetPreviewContent();
  this->EndSession();
}

bool mitk::nnInteractiveTool::IsSessionRunning() const
{
  auto pythonContext = m_Impl->GetPythonContext();

  if (pythonContext == nullptr)
    return false;
  
  return pythonContext->HasVariable("session");
}

void mitk::nnInteractiveTool::Impl::SetAutoZoom() const
{
  std::ostringstream pyCommands; pyCommands
    << "session.set_do_autozoom(" << (this->AutoZoom ? "True" : "False") << ")\n";

  this->ExecuteSession(pyCommands.str());
}

void mitk::nnInteractiveTool::Impl::AddPointInteraction(const Point3D& point, const Image* inputAtTimeStep) const
{
  itk::Index<3> index;
  inputAtTimeStep->GetGeometry()->WorldToIndex(point, index);

  std::ostringstream pyCommands; pyCommands
    << "session.add_point_interaction(\n"
    << "    [" << index[2] << ", " << index[1] << ", " << index[0] << "],\n"
    << "    include_interaction=" << (this->PromptType == PromptType::Positive ? "True" : "False") << '\n'
    << ")\n";

  this->ExecuteSession(pyCommands.str());
}

void mitk::nnInteractiveTool::Impl::AddBoxInteraction(const PlanarFigure* box, const Image* inputAtTimeStep) const
{
  const auto* geometry = inputAtTimeStep->GetGeometry();

  std::array<itk::Index<3>, 2> indices;
  geometry->WorldToIndex(box->GetWorldControlPoint(0), indices[0]);
  geometry->WorldToIndex(box->GetWorldControlPoint(2), indices[1]);

  std::ostringstream pyCommands; pyCommands
    << "session.add_bbox_interaction(\n"
    << "    [\n";

  for (int i = 2; i >= 0; --i)
  {
    // nnInteractive expects half-open bounding boxes [min, max).
    // Our indices are inclusive, so we add +1 to the upper bound.
    pyCommands
      << "        ["
      << std::min(indices[0][i], indices[1][i]) << ", "
      << std::max(indices[0][i], indices[1][i]) + 1
      << "],\n";
  }

  pyCommands
    << "    ],\n"
    << "    include_interaction=" << (this->PromptType == PromptType::Positive ? "True" : "False") << '\n'
    << ")\n";

  this->ExecuteSession(pyCommands.str());
}

void mitk::nnInteractiveTool::Impl::AddScribbleInteraction(const Image* mask, const InteractionBoundingBox* boundingBox) const
{
  m_PythonContext->BindImage(const_cast<Image*>(mask), "mitk_scribble_mask");

  std::ostringstream pyCommands; pyCommands
    << "scribble_mask = mitk_scribble_mask.as_numpy()\n"
    << "session.add_scribble_interaction(\n"
    << "    scribble_mask,\n"
    << "    include_interaction=" << (this->PromptType == PromptType::Positive ? "True" : "False");

  if (boundingBox != nullptr)
  {
    pyCommands
      << ",\n    interaction_bbox=["
      << '[' << (*boundingBox)[0][0] << ',' << (*boundingBox)[0][1] << "],"
      << '[' << (*boundingBox)[1][0] << ',' << (*boundingBox)[1][1] << "],"
      << '[' << (*boundingBox)[2][0] << ',' << (*boundingBox)[2][1] << "]]";
  }

  pyCommands << '\n'
    << ")\n"
    << "del scribble_mask\n";

  this->ExecuteSession(pyCommands.str());
}

void mitk::nnInteractiveTool::Impl::AddLassoInteraction(const Image* mask, const InteractionBoundingBox* boundingBox) const
{
  m_PythonContext->BindImage(const_cast<Image*>(mask), "mitk_lasso_mask");

  std::ostringstream pyCommands; pyCommands
    << "lasso_mask = mitk_lasso_mask.as_numpy()\n"
    << "session.add_lasso_interaction(\n"
    << "    lasso_mask,\n"
    << "    include_interaction=" << (this->PromptType == PromptType::Positive ? "True" : "False");

  if (boundingBox != nullptr)
  {
    pyCommands
      << ",\n    interaction_bbox=["
      << '[' << (*boundingBox)[0][0] << ',' << (*boundingBox)[0][1] << "],"
      << '[' << (*boundingBox)[1][0] << ',' << (*boundingBox)[1][1] << "],"
      << '[' << (*boundingBox)[2][0] << ',' << (*boundingBox)[2][1] << "]]";
  }

  pyCommands << '\n'
    << ")\n"
    << "del lasso_mask\n";

  this->ExecuteSession(pyCommands.str());
}

void mitk::nnInteractiveTool::Impl::AddInitialSegInteraction(MultiLabelSegmentation* previewImage, TimeStepType timeStep) const
{
  m_PythonContext->BindImage(this->InitialSeg, "mitk_initial_seg");

  std::ostringstream pyCommands; pyCommands
    << "initial_seg = mitk_initial_seg.as_numpy()\n"
    << "session.add_initial_seg_interaction(\n"
    << "    initial_seg.astype(np.uint8),\n"
    << "    run_prediction=" << (this->AutoRefine ? "True" : "False") << '\n'
    << ")\n"
    << "del initial_seg\n";

  this->ExecuteSession(pyCommands.str());

  previewImage->UpdateGroupImage(previewImage->GetActiveLayer(), this->TargetBuffer, timeStep, 0, ImageAccessorBase::IgnoreLock);
}

void mitk::nnInteractiveTool::Impl::ResetInteractions() const
{
  this->ExecuteSession("session.reset_interactions()\n");
}

bool mitk::nnInteractiveTool::Impl::Undo() const
{
  this->ExecuteSession("nni_undone = bool(session.undo())\n");

  const auto undone = m_PythonContext->GetVariableAsBool("nni_undone").value_or(false);
  m_PythonContext->Execute("del nni_undone\n");

  return undone;
}
