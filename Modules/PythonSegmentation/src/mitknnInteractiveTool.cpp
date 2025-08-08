/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveTool.h>

#include <mitkImageReadAccessor.h>
#include <mitknnInteractiveBoxInteractor.h>
#include <mitknnInteractiveLassoInteractor.h>
#include <mitknnInteractivePointInteractor.h>
#include <mitknnInteractiveScribbleInteractor.h>
#include <mitkPlanarFigure.h>
#include <mitkPythonContext.h>
#include <mitkToolManager.h>

#include <usGetModuleContext.h>
#include <usModuleResource.h>

using namespace mitk::nnInteractive;

namespace
{
  // Allocate and initialize a 3D image volume with zeros, then transfer
  // ownership of the memory to the given image.
  void InitializeVolume(mitk::Image* image)
  {
    size_t numPixels = 1;

    for (int i = 0; i < 3; ++i)
      numPixels *= image->GetDimension(i);

    auto data = new mitk::Label::PixelType[numPixels];
    std::memset(data, 0, numPixels * sizeof(mitk::Label::PixelType));

    image->SetImportVolume(data, 0, 0, mitk::Image::ManageMemory);
  }
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

    ToolManager* GetToolManager() const
    {
      return m_ToolManager;
    }

    void SetToolManager(ToolManager* toolManager)
    {
      m_ToolManager = toolManager;
    }

    PythonContext* GetPythonContext() const
    {
      return m_PythonContext;
    }

    void SetPythonContext(PythonContext* pythonContext)
    {
      m_PythonContext = pythonContext;
    }

    // Methods that execute Python code are defined at the bottom of this file.
    bool IsCUDAAvailable() const;
    void SetAutoZoom() const;
    void AddPointInteraction(const Point3D& point, const Image* inputAtTimeStep) const;
    void AddBoxInteraction(const PlanarFigure* box, const Image* inputAtTimeStep) const;
    void AddScribbleInteraction(const Image* mask) const;
    void AddLassoInteraction(const Image* mask) const;
    void AddInitialSegInteraction(MultiLabelSegmentation* previewImage, TimeStepType timeStep) const;
    void ResetInteractions() const;

    Image::Pointer TargetBuffer;
    nnInteractive::PromptType PromptType;
    InteractorMap Interactors;
    Image::Pointer InitialSeg;
    bool AutoZoom;
    bool AutoRefine;

  private:
    std::optional<Backend> m_Backend;
    PythonContext::Pointer m_PythonContext;
    ToolManager::Pointer m_ToolManager;
  };
}

mitk::nnInteractiveTool::nnInteractiveTool()
  : m_Impl(std::make_unique<Impl>())
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

const char** mitk::nnInteractiveTool::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::nnInteractiveTool::GetIconResource() const
{
  auto segmentationModule = us::GetModuleContext()->GetModule("MitkSegmentation");
  auto iconResource = segmentationModule->GetResource("AI.svg");
  return iconResource;
}

bool mitk::nnInteractiveTool::CanHandle(const BaseData* referenceData, const BaseData* workingData) const
{
  if (!Superclass::CanHandle(referenceData, workingData))
    return false;

  if (static_cast<const Image*>(referenceData)->GetDimension() > 3)
    return false;

  return true;
}

void mitk::nnInteractiveTool::Deactivated()
{
  this->DisableInteractor();
  this->ResetInteractions();

  this->EndSession();

  Superclass::Deactivated();
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

  // Set reference image through our own tool manager for interactors.
  m_Impl->GetToolManager()->SetReferenceData(this->GetToolManager()->GetReferenceData(0));

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

  m_Impl->GetToolManager()->SetReferenceData(nullptr);
}

void mitk::nnInteractiveTool::ResetInteractions()
{
  for (auto& [interactionType, interactor] : m_Impl->Interactors)
    interactor->Reset();

  m_Impl->InitialSeg = nullptr;

  if (this->IsSessionRunning())
    m_Impl->ResetInteractions();

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

bool mitk::nnInteractiveTool::GetAutoZoom() const
{
  return m_Impl->AutoZoom;
}

void mitk::nnInteractiveTool::SetAutoZoom(bool autoZoom)
{
  m_Impl->AutoZoom = autoZoom;

  if (this->IsSessionRunning())
    m_Impl->SetAutoZoom();
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

  auto ownToolManager = ToolManager::New(toolManager->GetDataStorage());
  m_Impl->SetToolManager(ownToolManager);

  for (auto& [interactionType, interactor] : m_Impl->Interactors)
    interactor->SetToolManager(ownToolManager);
}

void mitk::nnInteractiveTool::InitializeSessionWithMask(Image* mask)
{
  if (mask == nullptr)
    return;

  m_Impl->InitialSeg = mask;
  this->UpdatePreview();
}

void mitk::nnInteractiveTool::DoUpdatePreview(const Image* inputAtTimeStep, const Image* /*oldSegAtTimeStep*/, MultiLabelSegmentation* previewImage, TimeStepType timeStep)
{
  // This method assumes it is only called when an interaction has occurred or
  // when a session should be (re)initialized with a label mask. Otherwise,
  // calling this method will reset any existing preview content.

  if (previewImage == nullptr || m_Impl->GetPythonContext() == nullptr)
    return;

  const auto* interactor = m_Impl->GetEnabledInteractor();

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
        auto mask = static_cast<const ScribbleInteractor*>(interactor)->GetLastScribbleMask();
        m_Impl->AddScribbleInteraction(mask);
        break;
      }
      case InteractionType::Lasso:
      {
        auto mask = static_cast<const LassoInteractor*>(interactor)->GetLastLassoMask();
        m_Impl->AddLassoInteraction(mask);
        break;
      }
      default:
        MITK_ERROR << "Cannot update preview because of unknown interaction type!";
        return;
    }

    previewImage->UpdateGroupImage(previewImage->GetActiveLayer(), m_Impl->TargetBuffer, timeStep);
  }
  else if (m_Impl->InitialSeg.IsNotNull())
  {
    m_Impl->AddInitialSegInteraction(previewImage, timeStep);
  }
  else
  {
    this->ResetPreviewContentAtTimeStep(timeStep);
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
    auto pythonContext = PythonContext::New(this->GetVirtualEnvName());
    pythonContext->Activate();

    m_Impl->SetPythonContext(pythonContext);
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
    << "nninteractive_is_installed = importlib.util.find_spec('nnInteractive') is not None\n";

  auto pythonContext = m_Impl->GetPythonContext();
  pythonContext->ExecuteString(pyCommands.str());

  return pythonContext->GetVariableAs<bool>("nninteractive_is_installed").value_or(false);
}

void mitk::nnInteractiveTool::StartSession()
{
  constexpr auto MODEL_CHECKPOINT = "nnInteractive_v1.0";

  if (this->IsSessionRunning())
    this->EndSession();

  auto pythonContext = m_Impl->GetPythonContext();

  {
    std::ostringstream pyCommands; pyCommands
      << "import torch\n"
      << "import nnInteractive\n"
      << "from importlib.metadata import version\n"
      << "from pathlib import Path\n"
      << "from nnunetv2.utilities.find_class_by_name import recursive_find_python_class\n"
      << "from batchgenerators.utilities.file_and_folder_operations import join, load_json\n"
      << "from huggingface_hub import snapshot_download\n"
      << "print(f'nnInteractive version: {version(\"nnInteractive\")}')\n"
      << "print('Model checkpoint: " << MODEL_CHECKPOINT << "')\n"
      << "repo_id = 'nnInteractive/nnInteractive'\n"
      << "download_path = snapshot_download(\n"
      << "    repo_id = repo_id,\n"
      << "    allow_patterns = ['" << MODEL_CHECKPOINT << "/*'],\n"
      << "    force_download = False\n"
      << ")\n"
      << "checkpoint_path = Path(download_path).joinpath('" << MODEL_CHECKPOINT << "')\n";
    pythonContext->ExecuteString(pyCommands.str());
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
    pythonContext->ExecuteString(pyCommands.str());
  }

  m_Impl->ResetBackend();

  bool isCUDAAvailable = m_Impl->IsCUDAAvailable();
  const std::string torchDevice = isCUDAAvailable
    ? "cuda:0"
    : "cpu";

  if (!isCUDAAvailable)
    this->SetAutoZoom(false);

  {
    std::ostringstream pyCommands; pyCommands
      << "session = inference_class(\n"
      << "    device=torch.device('" << torchDevice << "'),\n"
      << "    use_torch_compile=False,\n"
      << "    torch_n_threads=os.cpu_count(),\n"
      << "    verbose=False,\n"
      << "    do_autozoom=" << m_Impl->AutoZoom << '\n'
      << ")\n"
      << "session.initialize_from_trained_model_folder(checkpoint_path)\n";
    pythonContext->ExecuteString(pyCommands.str());
  }

  m_Impl->SetBackend(isCUDAAvailable
    ? Backend::CUDA
    : Backend::CPU);

  auto image = this->GetToolManager()->GetReferenceData(0)->GetDataAs<Image>();
  
  const auto timePoint = this->GetToolManager()->GetCurrentTimePoint();
  const auto timeStep = image->GetTimeGeometry()->TimePointToTimeStep(timePoint);
  
  auto imageAtTimeStep = this->GetImageByTimeStep(image, timeStep);
  const auto spacing = imageAtTimeStep->GetGeometry()->GetSpacing();

  const auto maskPixelType = MultiLabelSegmentation::GetPixelType();
  m_Impl->TargetBuffer->Initialize(maskPixelType, *(imageAtTimeStep->GetTimeGeometry()));
  InitializeVolume(m_Impl->TargetBuffer);

  pythonContext->TransferBaseDataToPython(imageAtTimeStep, "mitk_image");
  pythonContext->TransferBaseDataToPython(m_Impl->TargetBuffer.GetPointer(), "mitk_target_buffer");

  {
    std::ostringstream pyCommands; pyCommands
      << "image = mitk_image.GetAsNumpy()\n"
      << "spacing = [\n"
      << std::to_string(spacing[2]) << ", "
      << std::to_string(spacing[1]) << ", "
      << std::to_string(spacing[0]) << "]\n"
      << "target_buffer = mitk_target_buffer.GetAsNumpy()\n"
      << "torch_target_buffer = torch.from_numpy(target_buffer)\n"
      << "session.set_image(image[None], {'spacing': spacing})\n"
      << "session.set_target_buffer(torch_target_buffer)\n";
    pythonContext->ExecuteString(pyCommands.str());
  }
}

void mitk::nnInteractiveTool::EndSession()
{
  if (!this->IsSessionRunning())
    return;

  std::ostringstream pyCommands; pyCommands
    << "session._reset_session()\n"
    << "del session.network\n"
    << "del session\n";

  if (m_Impl->GetBackend() == Backend::CUDA)
    pyCommands << "torch.cuda.empty_cache()\n";

  m_Impl->GetPythonContext()->ExecuteString(pyCommands.str());
  m_Impl->SetPythonContext(nullptr);
}

bool mitk::nnInteractiveTool::IsSessionRunning() const
{
  auto pythonContext = m_Impl->GetPythonContext();

  if (pythonContext == nullptr)
    return false;
  
  return pythonContext->HasVariable("session");
}

bool mitk::nnInteractiveTool::Impl::IsCUDAAvailable() const
{
  std::ostringstream pyCommands; pyCommands
    << "import torch\n"
    << "try:\n"
    << "    torch.cuda.init()\n"
    << "    torch.empty(1, device='cuda')\n"
    << "    cuda_is_available = True\n"
    << "except Exception:\n"
    << "    cuda_is_available = False\n";

  m_PythonContext->ExecuteString(pyCommands.str());

  return m_PythonContext->GetVariableAs<bool>("cuda_is_available").value_or(false);
}

void mitk::nnInteractiveTool::Impl::SetAutoZoom() const
{
  std::ostringstream pyCommands; pyCommands
    << "session.set_do_autozoom(" << (this->AutoZoom ? "True" : "False") << ")\n";

  m_PythonContext->ExecuteString(pyCommands.str());
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

  m_PythonContext->ExecuteString(pyCommands.str());
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
    pyCommands
      << "        ["
      << std::min(indices[0][i], indices[1][i]) << ", "
      << std::max(indices[0][i], indices[1][i])
      << "],\n";
  }

  pyCommands
    << "    ],\n"
    << "    include_interaction=" << (this->PromptType == PromptType::Positive ? "True" : "False") << '\n'
    << ")\n";

  m_PythonContext->ExecuteString(pyCommands.str());
}

void mitk::nnInteractiveTool::Impl::AddScribbleInteraction(const Image* mask) const
{
  m_PythonContext->TransferBaseDataToPython(const_cast<Image*>(mask), "mitk_scribble_mask");

  std::ostringstream pyCommands; pyCommands
    << "scribble_mask = mitk_scribble_mask.GetAsNumpy()\n"
    << "session.add_scribble_interaction(\n"
    << "    scribble_mask.astype(numpy.uint8),\n"
    << "    include_interaction=" << (this->PromptType == PromptType::Positive ? "True" : "False") << '\n'
    << ")\n";

  m_PythonContext->ExecuteString(pyCommands.str());
}

void mitk::nnInteractiveTool::Impl::AddLassoInteraction(const Image* mask) const
{
  m_PythonContext->TransferBaseDataToPython(const_cast<Image*>(mask), "mitk_lasso_mask");

  std::ostringstream pyCommands; pyCommands
    << "lasso_mask = mitk_lasso_mask.GetAsNumpy()\n"
    << "session.add_lasso_interaction(\n"
    << "    lasso_mask.astype(numpy.uint8),\n"
    << "    include_interaction=" << (this->PromptType == PromptType::Positive ? "True" : "False") << '\n'
    << ")\n";

  m_PythonContext->ExecuteString(pyCommands.str());
}

void mitk::nnInteractiveTool::Impl::AddInitialSegInteraction(MultiLabelSegmentation* previewImage, TimeStepType timeStep) const
{
  m_PythonContext->TransferBaseDataToPython(this->InitialSeg, "mitk_initial_seg");

  std::ostringstream pyCommands; pyCommands
    << "initial_seg = mitk_initial_seg.GetAsNumpy()\n"
    << "session.add_initial_seg_interaction(\n"
    << "    initial_seg.astype(numpy.uint8),\n"
    << "    run_prediction=" << (this->AutoRefine ? "True" : "False") << '\n'
    << ")\n";

  m_PythonContext->ExecuteString(pyCommands.str());

  previewImage->UpdateGroupImage(previewImage->GetActiveLayer(), this->TargetBuffer, timeStep);
}

void mitk::nnInteractiveTool::Impl::ResetInteractions() const
{
  m_PythonContext->ExecuteString("session.reset_interactions()\n");
}
