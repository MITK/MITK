/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkVoxTellTool.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkLabelGroupPlacer.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkProgressTask.h>
#include <mitkPythonHelper.h>
#include <mitkPythonWorker.h>
#include <mitkRenderingManager.h>
#include <mitkSharedBuffer.h>
#include <mitkToolManager.h>

#include "mitkInitializationHook.h"
#include "mitkPythonWorkerModules.h"

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include <chrono>
#include <map>
#include <optional>
#include <set>

namespace mitk
{
  MITK_TOOL_MACRO(MITKPYTHONSEGMENTATION_EXPORT, VoxTellTool, "VoxTell")
}

namespace
{
  // VoxTell does not state how much memory the device needs. This is a
  // conservative guess to be replaced by a measured value. A device that falls
  // short is replaced by the CPU, unless the GPU backend is enforced.
  constexpr int MIN_GPU_MEMORY_MB = 6000;

  // A run checks for a cancellation after every patch of its sliding window,
  // which takes a fraction of this even on the CPU. A run that takes longer
  // to stop hangs, e.g. in the GPU driver, and its worker is ended.
  constexpr auto STOP_GRACE_PERIOD = std::chrono::seconds(10);

  // The overlap tolerance of the LabelGroupPlacer for the results of VoxTell.
  // Measured on a chest CT with 20 prompts: the border of two neighbors took at
  // most 0.96% of the smaller one, while structures that contain one another or
  // genuinely overlap shared 4.9% to 100%.
  constexpr double OVERLAP_TOLERANCE = 0.02;

  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }

  // mitk::Exception::GetDescription() may return nullptr; treat that as an empty
  // message.
  std::string Description(const mitk::Exception& e)
  {
    const char* description = e.GetDescription();
    return description != nullptr ? description : "";
  }

  // What the user can make sense of: the Python exception without its frames,
  // or that the worker is gone.
  std::string DescribeFailure(const mitk::Exception& e)
  {
    if (dynamic_cast<const mitk::PythonWorkerExitedException*>(&e) != nullptr)
      return "VoxTell stopped unexpectedly.";

    return Description(e);
  }

  // Calls the worker for a result the caller has to tell the user about. The
  // failure is logged in full and thrown with just the error.
  nlohmann::json CallWithReadableError(mitk::PythonWorker& worker, const std::string& method, const nlohmann::json& params = nullptr)
  {
    try
    {
      return worker.Call(method, params);
    }
    catch (const mitk::PythonWorkerError& e)
    {
      MITK_ERROR << "VoxTell: " << e.GetTraceback();
      mitkThrow() << "VoxTell: " << DescribeFailure(e);
    }
    catch (const mitk::PythonWorkerExitedException& e)
    {
      MITK_ERROR << "VoxTell: " << Description(e);
      mitkThrow() << DescribeFailure(e);
    }
    catch (const mitk::Exception& e)
    {
      MITK_ERROR << "VoxTell: " << Description(e);
      mitkThrow() << "VoxTell: " << DescribeFailure(e);
    }
  }

  // Calls a function of the hub files module, which returns (repo_id, patterns) pairs.
  std::vector<mitk::VoxTell::RepoFiles> QueryRepoFiles(mitk::PythonWorker& worker, const std::string& function, const mitk::PythonWorker::ProgressHook& progress = {})
  {
    std::vector<mitk::VoxTell::RepoFiles> files;

    for (const auto& pair : worker.Call("voxtell_hub_files." + function, {}, {}, progress))
    {
      mitk::VoxTell::RepoFiles repoFiles;
      repoFiles.RepoId = pair.at(0).get<std::string>();
      repoFiles.Patterns = pair.at(1).get<std::vector<std::string>>();

      if (!repoFiles.RepoId.empty())
        files.push_back(std::move(repoFiles));
    }

    return files;
  }

  // The masks of a run are as large as the image and are not kept by the
  // worker once the run is over.
  class ScopedRun
  {
  public:
    explicit ScopedRun(mitk::PythonWorker& worker)
      : m_Worker(worker)
    {
    }

    ~ScopedRun()
    {
      try
      {
        if (m_Worker.IsRunning())
          m_Worker.Call("voxtell_worker.release_run");
      }
      catch (...)
      {
        // The run has ended already. Not being able to clean up is not worth
        // an exception from a destructor.
      }
    }

    ScopedRun(const ScopedRun&) = delete;
    ScopedRun& operator=(const ScopedRun&) = delete;

  private:
    mitk::PythonWorker& m_Worker;
  };
}

mitk::VoxTell::ModelSource mitk::VoxTell::ParseModelSource(const std::string& value)
{
  if (value == "huggingface")
    return ModelSource::HuggingFace;

  if (value == "local")
    return ModelSource::Local;

  mitkThrow() << "Unknown model source \"" << value << "\". Expected \"huggingface\" or \"local\".";
}

nlohmann::json mitk::VoxTell::IndexToWorld(const BaseGeometry* geometry)
{
  const auto* transform = geometry->GetIndexToWorldTransform();
  const auto& matrix = transform->GetMatrix();
  const auto& offset = transform->GetOffset();

  auto rows = nlohmann::json::array();

  for (unsigned int row = 0; row < 3; ++row)
    rows.push_back({ matrix[row][0], matrix[row][1], matrix[row][2], offset[row] });

  rows.push_back({ 0.0, 0.0, 0.0, 1.0 });

  return rows;
}

std::string mitk::VoxTell::ReadHubFilesCode()
{
  return ReadWorkerModule("VoxTell/voxtell_hub_files.py").Source;
}

namespace mitk
{
  class VoxTellTool::Impl
  {
  public:
    /** What the cached masks are the result of. */
    struct Run
    {
      std::vector<std::string> Prompts;
      unsigned long long ImageModifiedTime = 0;
      TimeStepType TimeStep = 0;
      std::string Device;

      bool operator==(const Run&) const = default;
    };

    std::unique_ptr<PythonWorker> Worker;
    bool ModelLoaded = false;
    std::optional<Torch::Backend> Backend;
    std::optional<VoxTell::ModelSource> ModelSource;
    std::string Device;
    std::vector<std::string> Prompts;
    bool CreateGroupsAsNeeded = true;
    std::optional<Run> CachedRun;

    /** The result of the cached run, one mask per prompt. */
    std::vector<MaskRuns> CachedMasks;

    /** One label per prompt, in their order, made for the current update. */
    std::vector<Label::Pointer> PreviewLabels;

    /** The groups of the preview that its labels went into in the current update. */
    std::map<MultiLabelSegmentation::LabelValueType, MultiLabelSegmentation::GroupIndexType> LabelGroups;

    /** The labels with a result in any time step of the current update. */
    std::set<MultiLabelSegmentation::LabelValueType> LabelValuesWithResult;

    std::vector<std::string> PromptsWithoutResult;
    std::string LastErrorMessage;
    bool RunFailed = false;
    bool AbortRequested = false;

    // GetModelFiles() and LoadModel() process events, during which the tool
    // may be deactivated. They unload the model once they have stopped.
    bool Initializing = false;
  };
}

mitk::VoxTellTool::VoxTellTool()
  : SegWithPreviewTool(true), // prevents auto-compute across all time steps
    m_Impl(std::make_unique<Impl>())
{
  // The prompts do not depend on the time point, and a run is too costly to be
  // repeated just because the user moved through time.
  this->IsTimePointChangeAwareOff();
  this->ResetsToEmptyPreviewOn();
  this->RequestDeactivationConfirmationOn();

  // Stay active after a segmentation is confirmed, so the user can run more
  // prompts without having to initialize the model again.
  this->KeepActiveAfterAcceptOn();
  this->RequiresExistingLabelsOff();
  this->RequiresVolumetricReferenceOn();
  this->RequiresReferenceGeometryOn();

  // Results that overlap go into groups of their own (see SetCreateGroupsAsNeeded()).
  this->TransfersAllPreviewGroupsOn();
}

mitk::VoxTellTool::~VoxTellTool() = default;

const char* mitk::VoxTellTool::GetName() const
{
  return "VoxTell";
}

us::ModuleResource mitk::VoxTellTool::GetIconResource() const
{
  return us::GetModuleContext()->GetModule()->GetResource("VoxTell/VoxTell.svg");
}

void mitk::VoxTellTool::Activated()
{
  m_Impl->Prompts.clear();
  m_Impl->CachedRun.reset();
  m_Impl->LastErrorMessage.clear();
  m_Impl->RunFailed = false;
  m_Impl->AbortRequested = false;

  Superclass::Activated();

  this->SetLabelTransferScope(LabelTransferScope::AllLabels);
  this->SetLabelTransferMode(LabelTransferMode::AddLabel);
}

void mitk::VoxTellTool::Deactivated()
{
  // The worker cannot be destroyed while a call of it is on the stack. The
  // progress callback ends the worker at once, and UpdateCleanUp() or the
  // initialization unloads the model once the call has unwound.
  if (this->IsUpdating() || m_Impl->Initializing)
    m_Impl->AbortRequested = true;
  else
    this->UnloadModel();

  Superclass::Deactivated();
}

bool mitk::VoxTellTool::IsCancelable() const
{
  return true;
}

mitk::Label::AlgorithmType mitk::VoxTellTool::GetAlgorithmType() const
{
  return Label::AlgorithmType::AUTOMATIC;
}

std::string mitk::VoxTellTool::GetVirtualEnvName() const
{
  return VoxTell::VENV_NAME;
}

const std::vector<std::pair<std::string, std::string>>& mitk::VoxTellTool::GetSessionDefiningPreferences()
{
  // Key, default-as-stored. The defaults must match what LoadModel() reads and
  // what the preference page writes, so that seeding leaves the stored value
  // equal to the effective one.
  static const std::vector<std::pair<std::string, std::string>> keys = {
    { "VoxTell/backend", "auto" },
    { "VoxTell/gpuBackend", "cuda:0" },
    { "VoxTell/modelSource", "huggingface" },
    { "VoxTell/localModelPath", "" },
  };

  return keys;
}

mitk::PythonWorker& mitk::VoxTellTool::GetWorker()
{
  if (m_Impl->Worker == nullptr || !m_Impl->Worker->IsRunning())
  {
    m_Impl->Worker.reset();
    m_Impl->ModelLoaded = false;

    const auto python = PythonHelper::GetVirtualEnvExecutablePath(this->GetVirtualEnvName());

    if (python.empty())
      mitkThrow() << "VoxTell is not installed.";

    m_Impl->Worker = std::make_unique<PythonWorker>(python, std::vector<PythonWorker::Module> {
      ReadWorkerModule("torch_device.py"),
      ReadWorkerModule("VoxTell/voxtell_bridge.py"),
      ReadWorkerModule("VoxTell/voxtell_hub_files.py"),
      ReadWorkerModule("VoxTell/voxtell_worker.py")
    }, "VoxTell");
  }

  return *m_Impl->Worker;
}

std::vector<mitk::VoxTell::RepoFiles> mitk::VoxTellTool::GetModelFiles(ProgressTask* task)
{
  m_Impl->Initializing = true;
  m_Impl->AbortRequested = false;
  const auto progress = MakeInitializationHook(this, task, [this]() { return m_Impl->AbortRequested; });

  try
  {
    auto& worker = this->GetWorker();

    // VoxTell loads the embeddings of the prompts it knows with a local model as well.
    std::vector<std::string> functions = { "voxtell_prompt_list_files" };

    if (VoxTell::ParseModelSource(GetPreferences()->Get("VoxTell/modelSource", "huggingface")) == VoxTell::ModelSource::HuggingFace)
      functions.push_back("voxtell_model_files");

    std::vector<VoxTell::RepoFiles> files;

    for (const auto& function : functions)
    {
      try
      {
        const auto functionFiles = QueryRepoFiles(worker, function, progress);
        files.insert(files.end(), functionFiles.begin(), functionFiles.end());
      }
      catch (const PythonWorkerError& e)
      {
        // VoxTell downloads them itself then, only without telling how far it is.
        // A cancelled call or a worker that is gone has to reach the caller.
        MITK_WARN << "VoxTell: " << function << "() failed: " << DescribeFailure(e);
      }
    }

    m_Impl->Initializing = false;
    return files;
  }
  catch (...)
  {
    this->EndFailedInitialization();
    throw;
  }
}

void mitk::VoxTellTool::EndFailedInitialization()
{
  m_Impl->Worker.reset();
  m_Impl->Initializing = false;

  // Deactivated meanwhile, see Deactivated().
  if (m_Impl->AbortRequested)
  {
    m_Impl->AbortRequested = false;
    this->UnloadModel();
  }
}

void mitk::VoxTellTool::LoadModel(ProgressTask* task)
{
  // A fresh process frees whatever the previous model held.
  if (this->IsModelLoaded())
    m_Impl->Worker.reset();

  m_Impl->ModelLoaded = false;
  m_Impl->Backend.reset();
  m_Impl->ModelSource.reset();
  m_Impl->Device.clear();
  m_Impl->CachedRun.reset();

  auto* prefs = GetPreferences();

  // Materialize the session-defining preferences with their effective values so a
  // later no-op preferences "OK" does not register as a value change and unload
  // the model (mitk::Preferences fires its change event only on an actual
  // change, and a default written into a never-stored key counts as one). Safe
  // here: no model is loaded yet, so the GUI's observer is a no-op for these
  // writes.
  for (const auto& [key, defaultValue] : GetSessionDefiningPreferences())
    prefs->Put(key, prefs->Get(key, defaultValue));

  const auto modelSource = VoxTell::ParseModelSource(prefs->Get("VoxTell/modelSource", "huggingface"));
  const bool useLocalModel = modelSource == VoxTell::ModelSource::Local;
  const auto localModelPath = prefs->Get("VoxTell/localModelPath", "");

  if (useLocalModel && localModelPath.empty())
    mitkThrow() << "VoxTell: Local model mode is selected but no model folder is configured. "
                   "Set the path in Preferences -> Segmentation -> VoxTell.";

  m_Impl->Initializing = true;
  m_Impl->AbortRequested = false;
  const auto progress = MakeInitializationHook(this, task, [this]() { return m_Impl->AbortRequested; });

  try
  {
    auto& worker = this->GetWorker();

    const auto selection = Torch::SelectDevice(worker,
      Torch::ParseBackendPreference(prefs->Get("VoxTell/backend", "auto")), prefs->Get("VoxTell/gpuBackend", "cuda:0"), MIN_GPU_MEMORY_MB, progress);

    // Without a model folder VoxTell downloads its default model on first use
    // and takes it from its cache afterwards. The same goes for the embeddings
    // of the prompts it knows. Failures come back as a message for the user
    // instead of a traceback.
    const auto result = worker.Call("voxtell_worker.load_model", {
      { "model_dir", useLocalModel ? nlohmann::json(localModelPath) : nlohmann::json(nullptr) },
      { "device", selection.Device }
    }, {}, progress);

    if (const auto modelError = result.value("error", std::string()); !modelError.empty())
    {
      if (useLocalModel)
        mitkThrow() << "VoxTell: Could not load the model from the configured folder. (" << modelError << ")";

      mitkThrow() << "VoxTell: Could not obtain the model. Check your internet connection, or set a local "
                     "model folder in Preferences -> Segmentation -> VoxTell. ("
                  << modelError << ")";
    }

    m_Impl->ModelLoaded = true;
    m_Impl->Backend = selection.SelectedBackend;
    m_Impl->ModelSource = modelSource;
    m_Impl->Device = selection.Device;
    m_Impl->Initializing = false;
  }
  catch (...)
  {
    this->EndFailedInitialization();
    throw;
  }
}

void mitk::VoxTellTool::UnloadModel()
{
  // Ending the process frees the memory of the device.
  m_Impl->Worker.reset();
  m_Impl->ModelLoaded = false;

  m_Impl->Backend.reset();
  m_Impl->ModelSource.reset();
  m_Impl->Device.clear();
  m_Impl->CachedRun.reset();
  m_Impl->CachedMasks.clear();
}

bool mitk::VoxTellTool::IsModelLoaded() const
{
  return m_Impl->ModelLoaded && m_Impl->Worker != nullptr && m_Impl->Worker->IsRunning();
}

std::string mitk::VoxTellTool::GetWorkerExitDescription() const
{
  return m_Impl->Worker != nullptr ? m_Impl->Worker->GetExitDescription() : std::string();
}

std::optional<mitk::Torch::Backend> mitk::VoxTellTool::GetBackend() const
{
  return m_Impl->Backend;
}

std::optional<mitk::VoxTell::ModelSource> mitk::VoxTellTool::GetModelSource() const
{
  return m_Impl->ModelSource;
}

void mitk::VoxTellTool::SetPrompts(const std::vector<std::string>& prompts)
{
  m_Impl->Prompts = prompts;
}

const std::vector<std::string>& mitk::VoxTellTool::GetPrompts() const
{
  return m_Impl->Prompts;
}

void mitk::VoxTellTool::SetCreateGroupsAsNeeded(bool createGroupsAsNeeded)
{
  m_Impl->CreateGroupsAsNeeded = createGroupsAsNeeded;
}

bool mitk::VoxTellTool::GetCreateGroupsAsNeeded() const
{
  return m_Impl->CreateGroupsAsNeeded;
}

std::vector<std::string> mitk::VoxTellTool::GetPromptsWithoutPrecomputedEmbedding(const std::vector<std::string>& prompts) const
{
  if (!this->IsModelLoaded())
    return {};

  // VoxTell turns prompts into lower case before it looks them up. Asking it
  // keeps this in agreement with what it does for any alphabet.
  return CallWithReadableError(*m_Impl->Worker, "voxtell_worker.unknown_prompts", { { "prompts", prompts } })
    .get<std::vector<std::string>>();
}

std::vector<std::string> mitk::VoxTellTool::GetKnownPrompts() const
{
  if (!this->IsModelLoaded())
    return {};

  return CallWithReadableError(*m_Impl->Worker, "voxtell_worker.known_prompts").get<std::vector<std::string>>();
}

std::vector<mitk::VoxTell::RepoFiles> mitk::VoxTellTool::GetTextModelFiles()
{
  if (!this->IsModelLoaded())
    mitkThrow() << "VoxTell is not initialized.";

  try
  {
    return QueryRepoFiles(*m_Impl->Worker, "voxtell_text_model_files");
  }
  catch (const Exception& e)
  {
    // VoxTell downloads them itself then, only without telling how far it is.
    MITK_WARN << "VoxTell: voxtell_text_model_files() failed: " << DescribeFailure(e);
    return {};
  }
}

bool mitk::VoxTellTool::IsTextModelLoaded() const
{
  if (!this->IsModelLoaded())
    return false;

  return CallWithReadableError(*m_Impl->Worker, "voxtell_worker.is_text_model_loaded").get<bool>();
}

bool mitk::VoxTellTool::IsTextModelCached() const
{
  if (!this->IsModelLoaded())
    return false;

  return CallWithReadableError(*m_Impl->Worker, "voxtell_hub_files.voxtell_text_model_cached").get<bool>();
}

void mitk::VoxTellTool::EmbedPrompts(const std::vector<std::string>& prompts, ProgressTask* task)
{
  if (!this->IsModelLoaded())
    mitkThrow() << "VoxTell is not initialized.";

  if (prompts.empty())
    return;

  m_Impl->Initializing = true;
  m_Impl->AbortRequested = false;
  const auto progress = MakeInitializationHook(this, task, [this]() { return m_Impl->AbortRequested; });

  try
  {
    // VoxTell keeps the embeddings it computes for the session, so the run that
    // follows finds them.
    m_Impl->Worker->Call("voxtell_worker.embed_prompts", { { "prompts", prompts } }, {}, progress);
    m_Impl->Initializing = false;
  }
  catch (const PythonWorkerError& e)
  {
    // The worker and its model survive an exception in Python, e.g. for lack
    // of memory.
    m_Impl->Initializing = false;
    MITK_ERROR << "VoxTell: " << e.GetTraceback();
    mitkThrow() << "VoxTell: " << DescribeFailure(e);
  }
  catch (...)
  {
    // A cancellation ended the worker, or it is gone. It is kept, so that
    // GetWorkerExitDescription() can tell why.
    m_Impl->Initializing = false;

    // Deactivated meanwhile, see Deactivated().
    if (m_Impl->AbortRequested)
    {
      m_Impl->AbortRequested = false;
      this->UnloadModel();
    }

    throw;
  }
}

const std::string& mitk::VoxTellTool::GetLastErrorMessage() const
{
  return m_Impl->LastErrorMessage;
}

void mitk::VoxTellTool::InitiateToolByInput()
{
  this->RemoveAllPreviewLabels();
}

void mitk::VoxTellTool::UpdatePrepare()
{
  Superclass::UpdatePrepare();

  m_Impl->LastErrorMessage.clear();
  m_Impl->PreviewLabels.clear();
  m_Impl->LabelGroups.clear();
  m_Impl->LabelValuesWithResult.clear();
  m_Impl->PromptsWithoutResult.clear();
  m_Impl->RunFailed = false;
  m_Impl->AbortRequested = false;

  // The labels are made here and not as the result arrives, so an update that
  // covers several time steps makes each of them only once. They are added to
  // the preview once it is known which group they go into.
  this->RemoveAllPreviewLabels();

  // Confirming copies the labels with their colors into the segmentation, so
  // the colors are chosen to be distinct from its labels as well as from each
  // other.
  std::vector<Color> usedColors;

  if (const auto* segmentation = this->GetTargetSegmentation(); segmentation != nullptr)
  {
    for (const auto& label : segmentation->GetLabels())
      usedColors.push_back(label->GetColor());
  }

  MultiLabelSegmentation::LabelValueType value = 1;

  for (const auto& prompt : m_Impl->Prompts)
  {
    const auto color = LabelSetImageHelper::SuggestNewLabelColor(usedColors);
    usedColors.push_back(color);

    auto label = Label::New(value, prompt);
    label->SetColor(color);

    m_Impl->PreviewLabels.push_back(label);

    ++value;
  }
}

void mitk::VoxTellTool::UpdateCleanUp()
{
  Superclass::UpdateCleanUp();

  if (m_Impl->AbortRequested)
  {
    m_Impl->AbortRequested = false;
    this->UnloadModel();
    return;
  }

  this->RemoveLabelsWithoutResult();
}

void mitk::VoxTellTool::ConfirmCleanUp()
{
  this->RemoveAllPreviewLabels();
  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::VoxTellTool::RemoveLabelsWithoutResult()
{
  auto* preview = this->GetPreviewSegmentation();

  // A failed or cancelled update has removed its labels already.
  if (preview == nullptr || m_Impl->RunFailed)
    return;

  // Only now that every time step of the update is done: a label can be
  // empty in one of them and not in another.
  const auto hasResult = [this](MultiLabelSegmentation::LabelValueType value)
  {
    return m_Impl->LabelValuesWithResult.count(value) != 0;
  };

  for (const auto& label : m_Impl->PreviewLabels)
  {
    if (!hasResult(label->GetValue()))
      m_Impl->PromptsWithoutResult.push_back(label->GetName());
  }

  // A prompt that VoxTell found nothing for has no label in the preview. One
  // whose voxels other labels took has.
  std::map<MultiLabelSegmentation::GroupIndexType, MultiLabelSegmentation::ConstLabelVectorType> changedGroups;

  for (const auto& [value, group] : m_Impl->LabelGroups)
  {
    if (!hasResult(value))
      changedGroups[group];
  }

  if (changedGroups.empty())
    return;

  for (auto& [group, labelsWithResult] : changedGroups)
  {
    for (const auto& label : preview->GetConstLabelsByValue(preview->GetLabelValuesByGroup(group)))
    {
      if (hasResult(label->GetValue()))
        labelsWithResult.push_back(label);
    }
  }

  // The labels without result have no pixels, which RemoveLabels() would still
  // erase in a pass over the whole preview for each of them.
  preview->ReplaceGroupLabels(changedGroups);
  RenderingManager::GetInstance()->RequestUpdateAll();
}

const std::vector<std::string>& mitk::VoxTellTool::GetPromptsWithoutResult() const
{
  return m_Impl->PromptsWithoutResult;
}

void mitk::VoxTellTool::DoUpdatePreview(const Image* /*inputAtTimeStep*/, const Image* /*oldSegAtTimeStep*/, MultiLabelSegmentation* previewImage, TimeStepType timeStep)
{
  if (m_Impl->Prompts.empty())
  {
    this->ResetPreviewContentAtTimeStep(timeStep);
    return;
  }

  // A failed or aborted run leaves no labels in the preview, so the remaining
  // time steps of the same update must not write pixels without them.
  if (m_Impl->RunFailed)
    return;

  auto* task = m_ProgressCommand->GetProgressTask();

  try
  {
    auto* referenceNode = this->GetToolManager()->GetReferenceData(0);
    auto* referenceImage = referenceNode != nullptr ? referenceNode->GetDataAs<Image>() : nullptr;

    if (referenceImage == nullptr)
      mitkThrow() << "VoxTell needs an image to segment.";

    const Impl::Run run{
      m_Impl->Prompts,
      static_cast<unsigned long long>(referenceImage->GetMTime()),
      timeStep,
      m_Impl->Device
    };

    // The result of the last run is still around if nothing it depends on has
    // changed. That is the case when the base class asks for an update of the
    // same time step because the active label of the segmentation changed,
    // which makes no difference to VoxTell. Running the model again would take
    // just as long as the first time. Where the results go is decided anew, as
    // the segmentation may have changed. Only the last run is kept, so every
    // other time step of a dynamic image is a run of its own. Placing cached
    // masks needs no worker, so they survive a worker that ended meanwhile.
    if (m_Impl->CachedRun != run)
    {
      if (!this->IsModelLoaded())
        mitkThrow() << "VoxTell is not initialized.";

      m_Impl->CachedRun.reset();
      m_Impl->CachedMasks.clear();

      const auto timePoint = previewImage->GetTimeGeometry()->TimeStepToTimePoint(timeStep);
      const auto referenceTimeStep = referenceImage->GetTimeGeometry()->TimePointToTimeStep(timePoint);

      // Not the input of the base class: a region of interest is not supported
      // by the transfer of the preview into the segmentation.
      auto imageAtTimeStep = this->GetImageByTimeStep(referenceImage, referenceTimeStep);

      auto& worker = *m_Impl->Worker;
      const ScopedRun scopedRun(worker);

      const auto imageSegment = SharedBuffer::CopyOf(imageAtTimeStep);
      SharedBuffer maskSegment(MakeScalarPixelType<unsigned char>(), imageSegment->GetShape());
      const auto maskBuffer = maskSegment.AsImage(*imageAtTimeStep->GetTimeGeometry());

      const auto isStopRequested = [this, task]() {
        return m_Impl->AbortRequested || (task != nullptr && task->IsCancelRequested());
      };

      std::optional<std::chrono::steady_clock::time_point> stopRequestedAt;

      // Called with every progress report of VoxTell, after every patch of its
      // sliding window, and in between. It is what keeps the application
      // responsive during the run, and the only chance to stop it.
      const auto progress = [this, &isStopRequested, &stopRequestedAt](const nlohmann::json& fields)
      {
        if (const auto total = fields.is_object() ? fields.value("total", 0) : 0; total > 0)
          m_ProgressCommand->ReportFraction(static_cast<float>(fields.value("done", 0)) / static_cast<float>(total));

        try
        {
          this->GUIProcessEventsMessage.Send();
        }
        catch (const std::exception& e)
        {
          MITK_WARN << "VoxTell: error while processing events (ignored): " << e.what();
        }

        // The tool was deactivated. The model is unloaded anyway, so a throw,
        // which makes the worker end right away, loses nothing and does not
        // wait for VoxTell to check for a cancellation.
        if (m_Impl->AbortRequested)
          mitkThrow() << "VoxTell: the run was aborted.";

        if (!isStopRequested())
          return true;

        const auto now = std::chrono::steady_clock::now();

        if (!stopRequestedAt.has_value())
          stopRequestedAt = now;
        else if (now - *stopRequestedAt > STOP_GRACE_PERIOD)
          mitkThrow() << "VoxTell: the run did not stop when it was cancelled and was ended.";

        return false;
      };

      // The text encoder of VoxTell is loaded here if a prompt needs it that
      // EmbedPrompts() did not process before. That takes long and does not
      // check for a cancellation, so a cancellation ends the worker after the
      // grace period. The sliding window below stops on its own.
      if (task != nullptr)
        task->SetName("VoxTell: embedding prompts");

      worker.Call("voxtell_worker.embed_prompts", { { "prompts", m_Impl->Prompts } }, {}, progress);

      if (isStopRequested())
        mitkThrow() << "VoxTell: the run was cancelled.";

      if (task != nullptr)
        task->SetName("VoxTell: segmenting");

      worker.Call("voxtell_worker.segment",
        { { "prompts", m_Impl->Prompts }, { "index_to_world", VoxTell::IndexToWorld(imageAtTimeStep->GetGeometry()) } },
        { { "image", imageSegment.get() }, { "mask", &maskSegment } },
        progress);

      // Masks can overlap, so they cannot share a label image. Each of them is
      // kept as the runs of its voxels, which take far less memory than a
      // buffer per prompt.
      std::vector<MaskRuns> masks;

      for (std::size_t index = 0; index < m_Impl->Prompts.size(); ++index)
      {
        worker.Call("voxtell_worker.write_mask", { { "index", index } });
        masks.push_back(ExtractMaskRuns(maskBuffer));
      }

      m_Impl->CachedMasks = std::move(masks);
      m_Impl->CachedRun = run;
    }

    this->PlaceMasks(previewImage, timeStep);
  }
  catch (const Exception& e)
  {
    // Labels without content would let the user confirm nothing, and the
    // preview may be half written.
    this->RemoveAllPreviewLabels();
    m_Impl->CachedRun.reset();
    m_Impl->CachedMasks.clear();
    m_Impl->RunFailed = true;

    // The tool was deactivated while it computed. There is nobody left to tell.
    if (m_Impl->AbortRequested)
      return;

    // Let the base class treat it as the cancellation it is, instead of as an
    // error to report.
    if (task != nullptr && task->IsCancelRequested())
      throw;

    if (const auto* error = dynamic_cast<const PythonWorkerError*>(&e))
      MITK_ERROR << "VoxTell: " << error->GetTraceback();
    else
      MITK_ERROR << "VoxTell: " << Description(e);

    m_Impl->LastErrorMessage = DescribeFailure(e);
  }
  catch (...)
  {
    this->RemoveAllPreviewLabels();
    m_Impl->CachedRun.reset();
    m_Impl->CachedMasks.clear();
    m_Impl->RunFailed = true;

    throw;
  }
}

void mitk::VoxTellTool::PlaceMasks(MultiLabelSegmentation* preview, TimeStepType timeStep)
{
  const auto* segmentation = this->GetTargetSegmentation();

  if (segmentation == nullptr)
    mitkThrow() << "VoxTell needs a segmentation to add its results to.";

  // The groups of the preview stand for those of the segmentation, which the
  // user can add to while VoxTell computes.
  while (preview->GetNumberOfGroups() < segmentation->GetNumberOfGroups())
    preview->AddGroup();

  LabelGroupPlacer placer(segmentation, preview, timeStep, OVERLAP_TOLERANCE);

  const auto writeMode = m_Impl->CreateGroupsAsNeeded
    ? LabelGroupPlacer::WriteMode::KeepOccupiedVoxels
    : LabelGroupPlacer::WriteMode::OverwriteVoxels;

  for (std::size_t index = 0; index < m_Impl->CachedMasks.size(); ++index)
  {
    const auto& mask = m_Impl->CachedMasks[index];

    if (mask.VoxelCount == 0)
      continue;

    const auto& label = m_Impl->PreviewLabels[index];
    auto labelGroup = m_Impl->LabelGroups.find(label->GetValue());

    // Decided once per update, so that a label lies in the same group in every
    // time step.
    if (labelGroup == m_Impl->LabelGroups.end())
    {
      const auto group = m_Impl->CreateGroupsAsNeeded
        ? placer.FindGroup(mask)
        : segmentation->GetActiveLayer();

      if (group == preview->GetNumberOfGroups())
        preview->AddGroup();

      preview->AddLabel(label, group, false, false);
      labelGroup = m_Impl->LabelGroups.emplace(label->GetValue(), group).first;
    }

    placer.Write(mask, labelGroup->second, label->GetValue(), writeMode);
  }

  for (const auto& [value, group] : m_Impl->LabelGroups)
  {
    if (placer.GetVoxelCount(value) != 0)
      m_Impl->LabelValuesWithResult.insert(value);
  }
}
