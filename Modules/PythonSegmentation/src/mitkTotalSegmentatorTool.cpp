/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTotalSegmentatorTool.h>

#include <mitkCoreServices.h>
#include <mitkExceptionMacro.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkLog.h>
#include <mitkLookupTable.h>
#include <mitkProgressTask.h>
#include <mitkPythonHelper.h>
#include <mitkPythonWorker.h>
#include <mitkRenderingManager.h>
#include <mitkSharedBuffer.h>

#include "mitkInitializationHook.h"
#include "mitkPythonWorkerModules.h"

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include <algorithm>
#include <array>

namespace mitk
{
  MITK_TOOL_MACRO(MITKPYTHONSEGMENTATION_EXPORT, TotalSegmentatorTool, "Total Segmentator");
}

namespace
{
  using TaskInfo = mitk::TotalSegmentatorTool::TaskInfo;
  using Speed = mitk::TotalSegmentatorTool::Speed;

  mitk::IPreferences* GetPreferences()
  {
    return mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }

  // mitk::Exception::GetDescription() may return nullptr; treat that as an empty
  // message.
  std::string Description(const mitk::Exception& e)
  {
    const char* description = e.GetDescription();
    return description != nullptr ? description : "";
  }

  std::string ToString(Speed speed)
  {
    switch (speed)
    {
      case Speed::Fast:
        return "fast";
      case Speed::Fastest:
        return "fastest";
      default:
        return "standard";
    }
  }

  TaskInfo ParseTask(const nlohmann::json& json)
  {
    TaskInfo task;
    task.Name = json.at("name").get<std::string>();
    task.MR = json.at("modality").get<std::string>() == "MR";
    task.Licensed = json.at("license_required").get<bool>();
    task.SupportsRoiSubset = json.at("roi_subset").get<bool>();

    for (const auto& speed : json.at("speeds"))
    {
      const auto name = speed.at("name").get<std::string>();
      const auto value = name == "fast" ? Speed::Fast : (name == "fastest" ? Speed::Fastest : Speed::Standard);
      const auto& resolution = speed.at("resolution");

      task.Speeds.push_back({ value, resolution.is_number() ? resolution.get<double>() : 0.0 });
    }

    for (const auto& [index, name] : json.at("classes").items())
      task.Classes[static_cast<mitk::Label::PixelType>(std::stoi(index))] = name.get<std::string>();

    return task;
  }

  // Whole-body tasks first, then the free ones, then the licensed ones.
  int GetSortRank(const TaskInfo& task)
  {
    if (0 == task.Name.rfind("total", 0))
      return 0;

    return task.Licensed ? 2 : 1;
  }
}

class mitk::TotalSegmentatorTool::Impl
{
public:
  std::unique_ptr<PythonWorker> Worker;
  std::vector<TaskInfo> Tasks;
  std::string Task = "total";
  Speed TaskSpeed = Speed::Standard;
  std::vector<std::string> RoiSubset;
  std::string LastErrorMessage;
  bool LastRunSucceeded = false;
  bool AbortRequested = false;

  // Initialize() processes events, during which the tool may be deactivated or
  // asked to start again.
  bool Initializing = false;
};

bool mitk::TotalSegmentatorTool::IsInstalled()
{
  return !PythonHelper::GetVirtualEnvExecutablePath(TotalSegmentator::VENV_NAME).empty();
}

bool mitk::TotalSegmentatorTool::HasLicense()
{
  return GetPreferences()->GetBool("TotalSeg/hasLicense", false);
}

std::string mitk::TotalSegmentatorTool::GetDevice()
{
  const auto device = GetPreferences()->Get("TotalSeg/device", "gpu");
  return device.empty() ? std::string("gpu") : device;
}

mitk::TotalSegmentatorTool::TotalSegmentatorTool()
  : SegWithPreviewTool(true), // prevents auto-compute across all time steps
    m_Impl(std::make_unique<Impl>())
{
  this->IsTimePointChangeAwareOff();
  this->RequestDeactivationConfirmationOn();

  // Stay active after a segmentation is confirmed, so the user can run another
  // task without having to reselect the tool.
  this->KeepActiveAfterAcceptOn();
  this->RequiresExistingLabelsOff();
  this->RequiresVolumetricReferenceOn();
  this->RequiresReferenceGeometryOn();
}

mitk::TotalSegmentatorTool::~TotalSegmentatorTool() = default;

void mitk::TotalSegmentatorTool::Activated()
{
  Superclass::Activated();
  this->SetLabelTransferScope(LabelTransferScope::AllLabels);
  this->SetLabelTransferMode(LabelTransferMode::AddLabel);
}

void mitk::TotalSegmentatorTool::Deactivated()
{
  // The worker cannot be destroyed while a call of it is on the stack. The
  // progress hook ends it at once, see DoUpdatePreview() and Initialize().
  if (this->IsUpdating() || m_Impl->Initializing)
    m_Impl->AbortRequested = true;
  else
    m_Impl->Worker.reset();

  Superclass::Deactivated();
}

bool mitk::TotalSegmentatorTool::IsCancelable() const
{
  return true;
}

us::ModuleResource mitk::TotalSegmentatorTool::GetIconResource() const
{
  // The shared segmentation icon lives in MitkSegmentation, not in this module.
  auto module = us::GetModuleContext()->GetModule("MitkSegmentation");
  return module->GetResource("AI.svg");
}

const char *mitk::TotalSegmentatorTool::GetName() const
{
  return "TotalSegmentator";
}

mitk::Label::AlgorithmType mitk::TotalSegmentatorTool::GetAlgorithmType() const
{
  return mitk::Label::AlgorithmType::AUTOMATIC;
}

mitk::PythonWorker& mitk::TotalSegmentatorTool::GetWorker()
{
  if (m_Impl->Worker == nullptr || !m_Impl->Worker->IsRunning())
  {
    m_Impl->Worker.reset();

    const auto python = PythonHelper::GetVirtualEnvExecutablePath(TotalSegmentator::VENV_NAME);

    if (python.empty())
      mitkThrow() << "TotalSegmentator is not installed.";

    m_Impl->Worker = std::make_unique<PythonWorker>(python, std::vector<PythonWorker::Module> {
      ReadWorkerModule("TotalSegmentator/totalsegmentator_worker.py")
    }, "TotalSegmentator");
  }

  return *m_Impl->Worker;
}

void mitk::TotalSegmentatorTool::Initialize(ProgressTask* task)
{
  if (this->IsInitialized())
    return;

  if (m_Impl->Initializing)
    mitkThrow() << "TotalSegmentator is starting already.";

  m_Impl->Initializing = true;
  m_Impl->AbortRequested = false;

  try
  {
    const auto progress = MakeInitializationHook(this, task, [this]() { return m_Impl->AbortRequested; });
    auto& worker = this->GetWorker();

    const auto version = worker.Call("totalsegmentator_worker.version", {}, {}, progress);

    if (!version.is_string() || version.get<std::string>() != TotalSegmentator::REQUIRED_VERSION)
    {
      mitkThrow() << "TotalSegmentator " << (version.is_string() ? version.get<std::string>() : std::string("of an unknown release"))
                  << " is installed, but MITK needs " << TotalSegmentator::REQUIRED_VERSION
                  << ". Uninstall it in the settings of TotalSegmentator and install it again.";
    }

    const auto info = worker.Call("totalsegmentator_worker.info", {}, {}, progress);
    std::vector<TaskInfo> tasks;

    for (const auto& json : info.at("tasks"))
      tasks.push_back(ParseTask(json));

    std::stable_sort(tasks.begin(), tasks.end(), [](const TaskInfo& a, const TaskInfo& b) {
      return GetSortRank(a) != GetSortRank(b) ? GetSortRank(a) < GetSortRank(b) : a.Name < b.Name;
    });

    m_Impl->Tasks = std::move(tasks);
    m_Impl->Initializing = false;

    // The task chosen before the start may not exist in this release.
    if (const auto* current = this->FindTask(m_Impl->Task); current == nullptr || !current->HasSpeed(m_Impl->TaskSpeed))
    {
      m_Impl->Task = this->FindTask("total") != nullptr || m_Impl->Tasks.empty() ? std::string("total") : m_Impl->Tasks.front().Name;
      m_Impl->TaskSpeed = Speed::Standard;
      m_Impl->RoiSubset.clear();
    }

    // Deactivated meanwhile: the tasks stay known, the worker goes.
    if (m_Impl->AbortRequested)
    {
      m_Impl->AbortRequested = false;
      m_Impl->Worker.reset();
    }

    this->InitializedEvent.Send();
  }
  catch (const PythonWorkerError& e)
  {
    MITK_ERROR << "TotalSegmentator: " << e.GetTraceback();
    this->EndFailedInitialization();
    mitkThrow() << "TotalSegmentator does not start: " << Description(e);
  }
  catch (...)
  {
    this->EndFailedInitialization();
    throw;
  }
}

void mitk::TotalSegmentatorTool::EndFailedInitialization()
{
  m_Impl->Worker.reset();
  m_Impl->Initializing = false;
  m_Impl->AbortRequested = false;
}

bool mitk::TotalSegmentatorTool::IsInitialized() const
{
  return !m_Impl->Tasks.empty();
}

void mitk::TotalSegmentatorTool::Uninitialize()
{
  if (this->IsUpdating() || m_Impl->Initializing)
    mitkThrow() << "TotalSegmentator is busy.";

  m_Impl->Worker.reset();
  m_Impl->Tasks.clear();
}

const std::vector<mitk::TotalSegmentatorTool::TaskInfo>& mitk::TotalSegmentatorTool::GetTasks() const
{
  return m_Impl->Tasks;
}

const mitk::TotalSegmentatorTool::TaskInfo* mitk::TotalSegmentatorTool::FindTask(const std::string& name) const
{
  const auto it = std::find_if(m_Impl->Tasks.begin(), m_Impl->Tasks.end(), [&name](const TaskInfo& task) { return task.Name == name; });
  return it != m_Impl->Tasks.end() ? &*it : nullptr;
}

void mitk::TotalSegmentatorTool::SetTask(const std::string& task, Speed speed)
{
  if (!this->IsInitialized())
    mitkThrow() << "TotalSegmentator is not initialized.";

  const auto* info = this->FindTask(task);

  if (info == nullptr)
    mitkThrow() << "TotalSegmentator has no task \"" << task << "\".";

  if (!info->HasSpeed(speed))
    mitkThrow() << "The TotalSegmentator task \"" << task << "\" does not run at the speed " << ToString(speed) << ".";

  if (task == m_Impl->Task && speed == m_Impl->TaskSpeed && m_Impl->RoiSubset.empty())
    return;

  m_Impl->Task = task;
  m_Impl->TaskSpeed = speed;
  m_Impl->RoiSubset.clear();

  this->OptionsChangedEvent.Send();
}

const std::string& mitk::TotalSegmentatorTool::GetTask() const
{
  return m_Impl->Task;
}

mitk::TotalSegmentatorTool::Speed mitk::TotalSegmentatorTool::GetSpeed() const
{
  return m_Impl->TaskSpeed;
}

void mitk::TotalSegmentatorTool::SetRoiSubset(const std::vector<std::string>& classes)
{
  const auto* info = this->FindTask(m_Impl->Task);

  if (info == nullptr)
    mitkThrow() << "TotalSegmentator is not initialized.";

  if (!classes.empty() && !info->SupportsRoiSubset)
    mitkThrow() << "The TotalSegmentator task \"" << info->Name << "\" segments all of its classes only.";

  // In the order of the task, and each class once.
  std::vector<std::string> subset;

  for (const auto& [value, name] : info->Classes)
  {
    if (std::find(classes.begin(), classes.end(), name) != classes.end())
      subset.push_back(name);
  }

  for (const auto& name : classes)
  {
    if (std::find(subset.begin(), subset.end(), name) == subset.end())
      mitkThrow() << "The TotalSegmentator task \"" << info->Name << "\" has no class \"" << name << "\".";
  }

  if (subset == m_Impl->RoiSubset)
    return;

  m_Impl->RoiSubset = std::move(subset);
  this->OptionsChangedEvent.Send();
}

const std::vector<std::string>& mitk::TotalSegmentatorTool::GetRoiSubset() const
{
  return m_Impl->RoiSubset;
}

const std::string& mitk::TotalSegmentatorTool::GetLastErrorMessage() const
{
  return m_Impl->LastErrorMessage;
}

bool mitk::TotalSegmentatorTool::LastRunSucceeded() const
{
  return m_Impl->LastRunSucceeded;
}

void mitk::TotalSegmentatorTool::UpdatePrepare()
{
  Superclass::UpdatePrepare();
  m_Impl->AbortRequested = false;
  this->RemoveAllPreviewLabels();
}

void mitk::TotalSegmentatorTool::ConfirmCleanUp()
{
  this->RemoveAllPreviewLabels();
  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::TotalSegmentatorTool::DownloadMissingWeights(ProgressTask* task)
{
  const auto progress = MakeInitializationHook(this, task, [this]() { return m_Impl->AbortRequested; });
  auto& worker = this->GetWorker();

  const auto missing = worker.Call("totalsegmentator_worker.missing_weights",
    { { "task", m_Impl->Task }, { "speed", ToString(m_Impl->TaskSpeed) } }, {}, progress).get<std::vector<int>>();

  for (std::size_t i = 0; i < missing.size(); ++i)
  {
    if (task != nullptr)
    {
      task->SetName(missing.size() > 1
        ? "TotalSegmentator: downloading model weights (" + std::to_string(i + 1) + " of " + std::to_string(missing.size()) + ")"
        : std::string("TotalSegmentator: downloading model weights"));
    }

    worker.Call("totalsegmentator_worker.download_weights", { { "task_id", missing[i] } }, {}, progress);
  }

  if (task != nullptr)
    task->SetName(this->GetName());
}

void mitk::TotalSegmentatorTool::DoUpdatePreview(const Image *inputAtTimeStep,
                                                 const Image * /*oldSegAtTimeStep*/,
                                                 MultiLabelSegmentation *previewImage,
                                                 TimeStepType timeStep)
{
  m_Impl->LastErrorMessage.clear();
  m_Impl->LastRunSucceeded = false;
  auto* task = m_ProgressCommand->GetProgressTask();

  // The base UpdatePreview would forward a failure to ErrorMessage, whose
  // listeners present it in a message box apart from the tool. So catch every
  // failure locally and record it; the GUI shows that in its status.
  try
  {
    this->Initialize(task);

    if (this->FindTask(m_Impl->Task)->Licensed && !HasLicense())
      mitkThrow() << "The TotalSegmentator task \"" << m_Impl->Task << "\" requires a license. Set it in the settings of TotalSegmentator.";

    this->DownloadMissingWeights(task);

    auto& worker = this->GetWorker();

    const auto imageBuffer = SharedBuffer::CopyOf(inputAtTimeStep);
    SharedBuffer labelBuffer(MakeScalarPixelType<Label::PixelType>(), imageBuffer->GetShape());

    // TotalSegmentator reports no progress. The hook processes events every
    // 100 ms, and a cancellation or a deactivation ends the worker at once.
    const auto progress = MakeInitializationHook(this, task, [this]() { return m_Impl->AbortRequested; });

    const auto classes = worker.Call("totalsegmentator_worker.segment",
      {
        { "index_to_world", GetIndexToWorld(inputAtTimeStep->GetGeometry()) },
        { "task", m_Impl->Task },
        { "speed", ToString(m_Impl->TaskSpeed) },
        { "roi_subset", m_Impl->RoiSubset },
        { "device", GetDevice() }
      },
      { { "image", imageBuffer.get() }, { "labels", &labelBuffer } },
      progress);

    auto lookupTable = LookupTable::New();
    lookupTable->SetType(LookupTable::LookupTableType::MULTILABEL);

    for (const auto& [index, name] : classes.items())
    {
      const auto value = static_cast<Label::PixelType>(std::stoi(index));
      auto label = Label::New(value, name.get<std::string>());

      std::array<double, 3> lookupTableColor;
      lookupTable->GetColor(value, lookupTableColor.data());

      Color color;
      color.SetRed(lookupTableColor[0]);
      color.SetGreen(lookupTableColor[1]);
      color.SetBlue(lookupTableColor[2]);
      label->SetColor(color);

      previewImage->AddLabel(label, previewImage->GetActiveLayer(), false);
    }

    previewImage->UpdateGroupImage(previewImage->GetActiveLayer(), labelBuffer.AsImage(*inputAtTimeStep->GetTimeGeometry()), timeStep);
    m_Impl->LastRunSucceeded = true;
  }
  catch (const Exception& e)
  {
    // Leave no half-populated preview behind: an empty preview is how the GUI
    // tells a failure apart from a successful run.
    this->RemoveAllPreviewLabels();

    // A cancellation ends the worker, which may not tell yet that it is gone,
    // and a failure may have left it in a state not worth keeping.
    m_Impl->Worker.reset();

    // The tool was deactivated, or the user cancelled: nothing to report.
    if (m_Impl->AbortRequested || (task != nullptr && task->IsCancelRequested()))
      return;

    if (const auto* error = dynamic_cast<const PythonWorkerError*>(&e))
      MITK_ERROR << "TotalSegmentator: " << error->GetTraceback();
    else
      MITK_ERROR << "TotalSegmentator: " << Description(e);

    m_Impl->LastErrorMessage = Description(e);
  }
  catch (const std::exception& e)
  {
    this->RemoveAllPreviewLabels();
    m_Impl->LastErrorMessage = e.what();
  }
}
