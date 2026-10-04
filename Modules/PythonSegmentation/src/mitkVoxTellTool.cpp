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
#include <mitkLabelSetImageHelper.h>
#include <mitkProgressTask.h>
#include <mitkPythonContext.h>
#include <mitkPythonUtil.h>
#include <mitkRenderingManager.h>
#include <mitkToolManager.h>

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include <functional>
#include <set>
#include <sstream>

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

  // The description of an error from Python is the exception ("Type: message")
  // followed by the frames it passed. Only the exception is of use to the user.
  std::string ShortenPythonError(std::string description)
  {
    static const std::string prefix = "An error occurred while executing Python code: ";

    if (description.starts_with(prefix))
      description.erase(0, prefix.size());

    if (const auto frames = description.find("\n\nAt:"); frames != std::string::npos)
      description.erase(frames);

    const auto end = description.find_last_not_of(" \t\r\n");
    description.erase(end == std::string::npos ? 0 : end + 1);

    return description;
  }

  // Runs code whose failure the caller has to tell the user about. The failure
  // is logged with the frames it passed and thrown with just the error.
  void ExecuteWithReadableError(mitk::PythonContext& context, const std::string& code)
  {
    try
    {
      context.Execute(code);
    }
    catch (const mitk::Exception& e)
    {
      MITK_ERROR << "VoxTell: " << Description(e);
      mitkThrow() << "VoxTell: " << ShortenPythonError(Description(e));
    }
  }

  // The Python code of the tool lives in resources, so it can be tested and
  // reused without the tool.
  std::string ReadPythonResource(const std::string& name)
  {
    auto module = us::GetModuleContext()->GetModule();
    auto resource = module->GetResource(name);

    if (!resource.IsValid())
      mitkThrow() << "VoxTell: The resource " << name << " could not be found.";

    us::ModuleResourceStream stream(resource);

    std::ostringstream code;
    code << stream.rdbuf();

    return code.str();
  }

  // Calls a function of the hub files code and reads its (repo_id, patterns)
  // pairs back from rows of tab-separated fields, the repository first.
  std::vector<mitk::VoxTell::RepoFiles> QueryRepoFiles(mitk::PythonContext& context, const std::string& function)
  {
    context.Execute("voxtell_hub_rows = '\\n'.join('\\t'.join([_r, *_p]) for _r, _p in " + function + "())\n");

    std::vector<mitk::VoxTell::RepoFiles> files;
    std::istringstream rows(context.GetVariableAsString("voxtell_hub_rows").value_or(""));

    for (std::string row; std::getline(rows, row);)
    {
      std::istringstream fields(row);
      mitk::VoxTell::RepoFiles repoFiles;

      std::getline(fields, repoFiles.RepoId, '\t');

      for (std::string pattern; std::getline(fields, pattern, '\t');)
        repoFiles.Patterns.push_back(pattern);

      if (!repoFiles.RepoId.empty())
        files.push_back(std::move(repoFiles));
    }

    return files;
  }

  // Python source of a list of strings, e.g. ['liver', 'left kidney'].
  std::string PyStringList(const std::vector<std::string>& strings)
  {
    std::ostringstream list;
    list << '[';

    for (const auto& string : strings)
      list << mitk::PyQuote(string) << ", ";

    list << ']';

    return list.str();
  }

  // The callable that VoxTell polls for cancellation lives in the context for
  // the duration of a run. Unbinding it afterwards keeps the tool that it
  // captured from being reachable once the run is over, and the images from
  // being kept alive by the context.
  class ScopedBindings
  {
  public:
    explicit ScopedBindings(mitk::PythonContext& context)
      : m_Context(context)
    {
    }

    ~ScopedBindings()
    {
      try
      {
        m_Context.BindFunction("progress_callback", {});
        m_Context.BindImage(nullptr, "mitk_image");
        m_Context.BindImage(nullptr, "mitk_target_buffer");
      }
      catch (...)
      {
        // The run has ended already. Not being able to unbind is not worth an
        // exception from a destructor.
      }
    }

    ScopedBindings(const ScopedBindings&) = delete;
    ScopedBindings& operator=(const ScopedBindings&) = delete;

  private:
    mitk::PythonContext& m_Context;
  };
}

std::string mitk::VoxTell::ReadHubFilesCode()
{
  return ReadPythonResource("VoxTell/voxtell_hub_files.py");
}

namespace mitk
{
  class VoxTellTool::Impl
  {
  public:
    Impl()
      : TargetBuffer(Image::New())
    {
    }

    /** What the target buffer holds the result of. */
    struct Run
    {
      std::vector<std::string> Prompts;
      unsigned long long ImageModifiedTime = 0;
      TimeStepType TimeStep = 0;
      std::string Device;

      bool operator==(const Run&) const = default;
    };

    std::unique_ptr<PythonContext> Context;
    std::optional<Torch::Backend> Backend;
    std::string Device;
    std::vector<std::string> Prompts;
    Image::Pointer TargetBuffer;
    std::optional<Run> CachedRun;

    /** The labels that are in the target buffer. */
    MultiLabelSegmentation::LabelValueVectorType CachedLabelValues;

    /** The labels with a result in any time step of the current update. */
    std::set<MultiLabelSegmentation::LabelValueType> LabelValuesWithResult;

    std::vector<std::string> PromptsWithoutResult;
    std::string LastErrorMessage;
    bool RunFailed = false;
    bool AbortRequested = false;
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
  // The model cannot be unloaded while Python still runs it. UpdateCleanUp()
  // does it once the run has stopped, which the progress callback makes happen.
  if (this->IsUpdating())
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

bool mitk::VoxTellTool::CreatePythonContext()
{
  try
  {
    m_Impl->Context = std::make_unique<PythonContext>(this->GetVirtualEnvName());

    // Activate without bindings: the install and version checks that run right
    // after creation must not map any venv native library (on Linux the venv is
    // sys.prefix, so importing NumPy would load it from there and make the
    // "loaded?" guard block the very update it is checking for). The bindings
    // are imported later, in LoadModel(), once the model needs them.
    m_Impl->Context->Activate(false);
  }
  catch (const Exception& e)
  {
    MITK_ERROR << Description(e);
    m_Impl->Context.reset();
    return false;
  }
  catch (const std::exception& e)
  {
    // The PythonContext constructor can throw a non-mitk exception (e.g. a
    // std::runtime_error from initializing the embedded interpreter for the
    // first time).
    MITK_ERROR << e.what();
    m_Impl->Context.reset();
    return false;
  }

  return true;
}

mitk::PythonContext* mitk::VoxTellTool::GetPythonContext() const
{
  return m_Impl->Context.get();
}

bool mitk::VoxTellTool::IsInstalled() const
{
  auto* context = m_Impl->Context.get();

  ExecuteWithReadableError(*context,
    "import importlib.util\n"
    "voxtell_is_installed = importlib.util.find_spec('voxtell') is not None\n");

  return context->GetVariableAsBool("voxtell_is_installed").value_or(false);
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

void mitk::VoxTellTool::PrepareContext()
{
  auto* context = m_Impl->Context.get();

  // The context was created lightweight (see CreatePythonContext). Running the
  // model needs the bindings (NumPy and the MITK module) to exchange images.
  // Activate() is idempotent, so this only adds them.
  context->Activate();
  context->Execute(ReadPythonResource("VoxTell/voxtell_bridge.py"));

  // Both are read when torch and nnU-Net are imported, so they have to be set
  // before VoxTell is. The allocator setting is what the napari plugin of
  // VoxTell uses, to keep the memory of the large buffers of a run from
  // fragmenting; PyTorch does not support it on Windows and warns about it
  // there. nnU-Net warns about paths that it needs for training only; dummies
  // keep it quiet.
  std::ostringstream environment; environment
    << "import os\n"
#if !defined(_WIN32)
    << "os.environ.setdefault('PYTORCH_CUDA_ALLOC_CONF', 'expandable_segments:True')\n"
#endif
    << "os.environ.setdefault('nnUNet_raw', '/tmp/nnUNet/raw')\n"
    << "os.environ.setdefault('nnUNet_preprocessed', '/tmp/nnUNet/preprocessed')\n"
    << "os.environ.setdefault('nnUNet_results', '/tmp/nnUNet/results')\n";

  context->Execute(environment.str());
  context->Execute(VoxTell::ReadHubFilesCode());
}

std::vector<mitk::VoxTell::RepoFiles> mitk::VoxTellTool::GetModelFiles()
{
  auto* context = m_Impl->Context.get();

  if (context == nullptr)
    mitkThrow() << "VoxTell: The files of the model cannot be determined without a Python context.";

  this->PrepareContext();

  // VoxTell loads the embeddings of the prompts it knows with a local model as well.
  std::vector<std::string> functions = { "voxtell_prompt_list_files" };

  if (GetPreferences()->Get("VoxTell/modelSource", "huggingface") != "local")
    functions.push_back("voxtell_model_files");

  std::vector<VoxTell::RepoFiles> files;

  for (const auto& function : functions)
  {
    try
    {
      const auto functionFiles = QueryRepoFiles(*context, function);
      files.insert(files.end(), functionFiles.begin(), functionFiles.end());
    }
    catch (const Exception& e)
    {
      // VoxTell downloads them itself then, only without telling how far it is.
      MITK_WARN << "VoxTell: " << function << "() failed: " << ShortenPythonError(Description(e));
    }
  }

  return files;
}

void mitk::VoxTellTool::LoadModel()
{
  auto* context = m_Impl->Context.get();

  if (context == nullptr)
    mitkThrow() << "VoxTell: The model cannot be loaded without a Python context.";

  if (this->IsModelLoaded())
    this->ReleaseModel();

  m_Impl->Backend.reset();
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

  const bool useLocalModel = prefs->Get("VoxTell/modelSource", "huggingface") == "local";
  const auto localModelPath = prefs->Get("VoxTell/localModelPath", "");

  if (useLocalModel && localModelPath.empty())
    mitkThrow() << "VoxTell: Local model mode is selected but no model folder is configured. "
                   "Set the path in Preferences -> Segmentation -> VoxTell.";

  try
  {
    this->PrepareContext();

    const auto selection = Torch::SelectDevice(*context,
      Torch::ParseBackendPreference(prefs->Get("VoxTell/backend", "auto")), prefs->Get("VoxTell/gpuBackend", "cuda:0"), MIN_GPU_MEMORY_MB);

    // Without a model folder VoxTell downloads its default model on first use
    // and takes it from its cache afterwards. The same goes for the embeddings
    // of the prompts it knows. Failures are mapped to a message for the user
    // instead of a traceback.
    std::ostringstream pyCommands; pyCommands
      << "import torch\n"
      << "from pathlib import Path\n"
      << "from voxtell.inference.predictor import VoxTellPredictor\n"
      << "voxtell_model_error = ''\n"
      << "try:\n"
      << "    voxtell_model_dir = " << (useLocalModel ? PyQuote(localModelPath) : std::string("None")) << "\n"
      << "    if voxtell_model_dir is not None and not Path(voxtell_model_dir).is_dir():\n"
      << "        raise FileNotFoundError('Folder not found: ' + voxtell_model_dir)\n"
      << "    voxtell_predictor = VoxTellPredictor(model_dir=voxtell_model_dir, device=torch.device("
      << PyQuote(selection.Device) << "))\n"
      << "    voxtell_known_prompts = set(voxtell_predictor.list_available_embeddings())\n"
      << "except Exception as _voxtell_e:\n"
      << "    voxtell_model_error = str(_voxtell_e) or type(_voxtell_e).__name__\n";

    context->Execute(pyCommands.str());

    const auto modelError = context->GetVariableAsString("voxtell_model_error").value_or("");

    if (!modelError.empty())
    {
      if (useLocalModel)
        mitkThrow() << "VoxTell: Could not load the model from the configured folder. (" << modelError << ")";

      mitkThrow() << "VoxTell: Could not obtain the model. Check your internet connection, or set a local "
                     "model folder in Preferences -> Segmentation -> VoxTell. ("
                  << modelError << ")";
    }

    m_Impl->Backend = selection.SelectedBackend;
    m_Impl->Device = selection.Device;
  }
  catch (...)
  {
    this->ReleaseModel();
    throw;
  }
}

void mitk::VoxTellTool::ReleaseModel()
{
  auto* context = m_Impl->Context.get();

  if (context == nullptr)
    return;

  std::ostringstream pyCommands; pyCommands
    << "try:\n"
    << "    del voxtell_predictor\n"
    << "except NameError:\n"
    << "    pass\n"
    << "import gc\n"
    << "gc.collect()\n";

  if (m_Impl->Backend == Torch::Backend::CUDA)
  {
    pyCommands
      << "import torch\n"
      << "torch.cuda.empty_cache()\n";
  }

  try
  {
    context->Execute(pyCommands.str());
  }
  catch (const Exception& e)
  {
    MITK_WARN << "VoxTell: error while releasing the model (ignored): " << Description(e);
  }
}

void mitk::VoxTellTool::UnloadModel()
{
  if (m_Impl->Context == nullptr)
    return;

  this->ReleaseModel();

  m_Impl->Context.reset();
  m_Impl->Backend.reset();
  m_Impl->Device.clear();
  m_Impl->CachedRun.reset();

  // The buffer is as large as the image.
  m_Impl->TargetBuffer = Image::New();
}

bool mitk::VoxTellTool::IsModelLoaded() const
{
  auto* context = m_Impl->Context.get();

  return context != nullptr && context->HasVariable("voxtell_predictor");
}

std::optional<mitk::Torch::Backend> mitk::VoxTellTool::GetBackend() const
{
  return m_Impl->Backend;
}

void mitk::VoxTellTool::SetPrompts(const std::vector<std::string>& prompts)
{
  m_Impl->Prompts = prompts;
}

const std::vector<std::string>& mitk::VoxTellTool::GetPrompts() const
{
  return m_Impl->Prompts;
}

std::vector<std::string> mitk::VoxTellTool::GetPromptsWithoutPrecomputedEmbedding(const std::vector<std::string>& prompts) const
{
  std::vector<std::string> unknown;

  if (!this->IsModelLoaded())
    return unknown;

  // VoxTell turns prompts into lower case before it looks them up. Asking it
  // here keeps this in agreement with what it does for any alphabet.
  auto* context = m_Impl->Context.get();

  std::ostringstream pyCommands; pyCommands
    << "voxtell_unknown_prompts = '\\n'.join(_p for _p in " << PyStringList(prompts)
    << " if _p.lower() not in voxtell_known_prompts)\n";

  ExecuteWithReadableError(*context, pyCommands.str());

  const auto joined = context->GetVariableAsString("voxtell_unknown_prompts").value_or("");
  std::istringstream lines(joined);

  for (std::string line; std::getline(lines, line);)
    unknown.push_back(line);

  return unknown;
}

std::vector<std::string> mitk::VoxTellTool::GetKnownPrompts() const
{
  std::vector<std::string> prompts;

  if (!this->IsModelLoaded())
    return prompts;

  auto* context = m_Impl->Context.get();
  ExecuteWithReadableError(*context, "voxtell_known_prompt_rows = '\\n'.join(sorted(voxtell_known_prompts))\n");

  std::istringstream rows(context->GetVariableAsString("voxtell_known_prompt_rows").value_or(""));

  for (std::string row; std::getline(rows, row);)
  {
    if (!row.empty())
      prompts.push_back(row);
  }

  return prompts;
}

std::vector<mitk::VoxTell::RepoFiles> mitk::VoxTellTool::GetTextModelFiles()
{
  if (!this->IsModelLoaded())
    mitkThrow() << "VoxTell is not initialized.";

  try
  {
    return QueryRepoFiles(*m_Impl->Context, "voxtell_text_model_files");
  }
  catch (const Exception& e)
  {
    // VoxTell downloads them itself then, only without telling how far it is.
    MITK_WARN << "VoxTell: voxtell_text_model_files() failed: " << ShortenPythonError(Description(e));
    return {};
  }
}

bool mitk::VoxTellTool::IsTextModelLoaded() const
{
  if (!this->IsModelLoaded())
    return false;

  auto* context = m_Impl->Context.get();
  ExecuteWithReadableError(*context, "voxtell_text_model_loaded = voxtell_predictor.text_backbone is not None\n");

  return context->GetVariableAsBool("voxtell_text_model_loaded").value_or(false);
}

bool mitk::VoxTellTool::IsTextModelCached() const
{
  if (!this->IsModelLoaded())
    return false;

  auto* context = m_Impl->Context.get();
  ExecuteWithReadableError(*context, "voxtell_text_model_is_cached = voxtell_text_model_cached()\n");

  return context->GetVariableAsBool("voxtell_text_model_is_cached").value_or(false);
}

void mitk::VoxTellTool::EmbedPrompts(const std::vector<std::string>& prompts)
{
  if (!this->IsModelLoaded())
    mitkThrow() << "VoxTell is not initialized.";

  if (prompts.empty())
    return;

  // VoxTell keeps the embeddings it computes for the session, so the run that
  // follows finds them.
  ExecuteWithReadableError(*m_Impl->Context,
    "voxtell_prompts = " + PyStringList(prompts) + "\n"
    "voxtell_predictor.embed_text_prompts(voxtell_prompts)\n"
    "voxtell_known_prompts.update(_p.lower() for _p in voxtell_prompts)\n");
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
  m_Impl->LabelValuesWithResult.clear();
  m_Impl->PromptsWithoutResult.clear();
  m_Impl->RunFailed = false;
  m_Impl->AbortRequested = false;

  // The labels are created here and not as the result arrives, so an update
  // that covers several time steps adds each of them only once.
  this->RemoveAllPreviewLabels();
  auto* preview = this->GetPreviewSegmentation();

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

    preview->AddLabel(label, preview->GetActiveLayer(), false, false);

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

void mitk::VoxTellTool::RemoveLabelsWithoutResult()
{
  auto* preview = this->GetPreviewSegmentation();

  // A failed or cancelled update has removed its labels already.
  if (preview == nullptr || m_Impl->RunFailed)
    return;

  // Only now that every time step of the update is done: a label can be
  // empty in one of them and not in another.
  const auto group = preview->GetActiveLayer();
  MultiLabelSegmentation::ConstLabelVectorType labelsWithResult;
  bool anyWithoutResult = false;

  for (const auto& label : preview->GetConstLabelsByValue(preview->GetLabelValuesByGroup(group)))
  {
    if (m_Impl->LabelValuesWithResult.count(label->GetValue()) != 0)
    {
      labelsWithResult.push_back(label);
    }
    else
    {
      m_Impl->PromptsWithoutResult.push_back(label->GetName());
      anyWithoutResult = true;
    }
  }

  if (!anyWithoutResult)
    return;

  // The labels without result have no pixels, which RemoveLabels() would still
  // erase in a pass over the whole preview for each of them.
  preview->ReplaceGroupLabels(group, labelsWithResult);
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
    if (!this->IsModelLoaded())
      mitkThrow() << "VoxTell is not initialized.";

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
    // changed. That is the case when the base class asks for an update only to
    // have a preview for all time steps, or because the active label of the
    // segmentation changed, which makes no difference to VoxTell. Running the
    // model again would take just as long as the first time.
    if (m_Impl->CachedRun != run)
    {
      m_Impl->CachedRun.reset();

      const auto timePoint = previewImage->GetTimeGeometry()->TimeStepToTimePoint(timeStep);
      const auto referenceTimeStep = referenceImage->GetTimeGeometry()->TimePointToTimeStep(timePoint);

      // Not the input of the base class: a region of interest is not supported
      // by the transfer of the preview into the segmentation, and the image is
      // bound to Python without const.
      auto imageAtTimeStep = this->GetImageByTimeStep(referenceImage, referenceTimeStep);

      m_Impl->TargetBuffer->Initialize(MultiLabelSegmentation::GetPixelType(), *(imageAtTimeStep->GetTimeGeometry()));
      m_Impl->TargetBuffer->AllocateZeroedVolume();

      auto* context = m_Impl->Context.get();
      const ScopedBindings bindings(*context);

      context->BindImage(imageAtTimeStep, "mitk_image");
      context->BindImage(m_Impl->TargetBuffer, "mitk_target_buffer");

      // Called by VoxTell after every patch of its sliding window. It is what
      // keeps the application responsive during the run, and the only chance to
      // stop it.
      context->BindFunction("progress_callback", [this, task](int done, int total)
      {
        if (total > 0)
          m_ProgressCommand->ReportFraction(static_cast<float>(done) / static_cast<float>(total));

        try
        {
          this->GUIProcessEventsMessage.Send();
        }
        catch (const std::exception& e)
        {
          MITK_WARN << "VoxTell: error while processing events (ignored): " << e.what();
        }

        return !m_Impl->AbortRequested && (task == nullptr || !task->IsCancelRequested());
      });

      // The text encoder of VoxTell is loaded here if a prompt needs it that
      // EmbedPrompts() did not process before. That takes long and cannot be
      // cancelled. The sliding window below can.
      if (task != nullptr)
        task->SetName("VoxTell: embedding prompts");

      this->GUIProcessEventsMessage.Send();

      context->Execute(
        "voxtell_prompts = " + PyStringList(m_Impl->Prompts) + "\n"
        "voxtell_embeddings = voxtell_predictor.embed_text_prompts(voxtell_prompts)\n");

      if (task != nullptr)
        task->SetName("VoxTell: segmenting");

      // Whatever the run leaves behind in the context, it is not kept: the
      // arrays are as large as the image, and the embeddings live on the device.
      context->Execute(
        "try:\n"
        "    voxtell_input, voxtell_ornt = voxtell_prepare_input(mitk_image)\n"
        "    voxtell_masks = voxtell_predictor.predict_single_image(\n"
        "        voxtell_input, text_embeddings=voxtell_embeddings, progress_callback=progress_callback)\n"
        "    voxtell_written = voxtell_write_masks(voxtell_masks, voxtell_ornt, mitk_target_buffer.as_numpy(writeable=True))\n"
        "    voxtell_written_labels = ' '.join(str(_v) for _v in voxtell_written)\n"
        "    voxtell_known_prompts.update(_p.lower() for _p in voxtell_prompts)\n"
        "finally:\n"
        "    voxtell_input = voxtell_embeddings = voxtell_masks = None\n");

      m_Impl->CachedLabelValues.clear();
      std::istringstream writtenLabels(context->GetVariableAsString("voxtell_written_labels").value_or(""));

      for (MultiLabelSegmentation::LabelValueType value; writtenLabels >> value;)
        m_Impl->CachedLabelValues.push_back(value);

      m_Impl->CachedRun = run;
    }

    previewImage->UpdateGroupImage(previewImage->GetActiveLayer(), m_Impl->TargetBuffer, timeStep, 0);
    m_Impl->LabelValuesWithResult.insert(m_Impl->CachedLabelValues.begin(), m_Impl->CachedLabelValues.end());
  }
  catch (const Exception& e)
  {
    // Labels without content would let the user confirm nothing, and the
    // buffer may be half written.
    this->RemoveAllPreviewLabels();
    m_Impl->CachedRun.reset();
    m_Impl->RunFailed = true;

    // The tool was deactivated while it computed. There is nobody left to tell.
    if (m_Impl->AbortRequested)
      return;

    // Let the base class treat it as the cancellation it is, instead of as an
    // error to report.
    if (task != nullptr && task->IsCancelRequested())
      throw;

    MITK_ERROR << "VoxTell: " << Description(e);
    m_Impl->LastErrorMessage = ShortenPythonError(Description(e));
  }
  catch (...)
  {
    this->RemoveAllPreviewLabels();
    m_Impl->CachedRun.reset();
    m_Impl->RunFailed = true;

    throw;
  }
}
