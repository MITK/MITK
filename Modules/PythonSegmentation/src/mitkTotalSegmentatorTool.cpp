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

#include "mitkPythonWorkerModules.h"
#include "mitkStopHook.h"

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include <algorithm>
#include <array>
#include <atomic>

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

  // A failed, cancelled or aborted run of one time step ends the update, which
  // runs once per time step for all of them.
  bool RunFailed = false;

  // Read by the work off the GUI thread, which it stops.
  std::atomic<bool> AbortRequested = false;

  // The start runs off the GUI thread, during which the tool may be
  // deactivated or asked to start again.
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

mitk::TotalSegmentatorTool::~TotalSegmentatorTool()
{
  // The work uses the worker.
  m_Impl->AbortRequested = true;
  this->WaitForWorkOffGUIThread();
}

void mitk::TotalSegmentatorTool::Activated()
{
  Superclass::Activated();
  this->SetLabelTransferScope(LabelTransferScope::AllLabels);
  this->SetLabelTransferMode(LabelTransferMode::AddLabel);
}

void mitk::TotalSegmentatorTool::Deactivated()
{
  // The worker cannot be destroyed while work off the GUI thread uses it. The
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

void mitk::TotalSegmentatorTool::Initialize(std::shared_ptr<ProgressTask> task)
{
  if (this->IsInitialized() || m_Impl->Initializing)
    return;

  PythonWorker* worker = nullptr;

  try
  {
    worker = &this->GetWorker();
  }
  catch (const Exception& e)
  {
    this->InitializationFailedEvent.Send(Description(e));
    return;
  }

  m_Impl->Initializing = true;
  m_Impl->AbortRequested = false;

  const auto progress = MakeStopHook(this, task.get(), [this]() { return m_Impl->AbortRequested.load(); });
  auto info = std::make_shared<nlohmann::json>();

  // No other work runs off the GUI thread here: a run needs an initialized
  // tool, and a second start returns above.
  this->RunOffGUIThread(
    [worker, progress, info]() {
      const auto version = worker->Call("totalsegmentator_worker.version", {}, {}, progress);

      if (!version.is_string() || version.get<std::string>() != TotalSegmentator::REQUIRED_VERSION)
      {
        mitkThrow() << "TotalSegmentator " << (version.is_string() ? version.get<std::string>() : std::string("of an unknown release"))
                    << " is installed, but MITK needs " << TotalSegmentator::REQUIRED_VERSION
                    << ". Uninstall it in the settings of TotalSegmentator and install it again.";
      }

      *info = worker->Call("totalsegmentator_worker.info", {}, {}, progress);
    },
    [this, task, info](std::exception_ptr error) {
      this->EndInitialization(error, *info, task.get());
    });
}

void mitk::TotalSegmentatorTool::EndInitialization(std::exception_ptr error, const nlohmann::json& info, const ProgressTask* task)
{
  m_Impl->Initializing = false;

  // Deactivated meanwhile: the worker goes.
  const bool aborted = m_Impl->AbortRequested.exchange(false);

  std::vector<TaskInfo> tasks;

  // A reply of another form than expected is a failed start like any other.
  if (error == nullptr)
  {
    try
    {
      for (const auto& json : info.at("tasks"))
        tasks.push_back(ParseTask(json));
    }
    catch (...)
    {
      error = std::current_exception();
    }
  }

  if (error != nullptr)
  {
    m_Impl->Worker.reset();

    std::string message;

    try
    {
      std::rethrow_exception(error);
    }
    catch (const PythonWorkerError& e)
    {
      MITK_ERROR << "TotalSegmentator: " << e.GetTraceback();
      message = "TotalSegmentator does not start: " + Description(e);
    }
    catch (const Exception& e)
    {
      message = Description(e);
    }
    catch (const std::exception& e)
    {
      message = e.what();
    }

    // The user stopped it: nothing to report.
    if (aborted || (task != nullptr && task->IsCancelRequested()))
      message.clear();
    else
      MITK_ERROR << message;

    this->InitializationFailedEvent.Send(message);
    return;
  }

  std::stable_sort(tasks.begin(), tasks.end(), [](const TaskInfo& a, const TaskInfo& b) {
    return GetSortRank(a) != GetSortRank(b) ? GetSortRank(a) < GetSortRank(b) : a.Name < b.Name;
  });

  m_Impl->Tasks = std::move(tasks);

  // The task chosen before the start may not exist in this release.
  if (const auto* current = this->FindTask(m_Impl->Task); current == nullptr || !current->HasSpeed(m_Impl->TaskSpeed))
  {
    m_Impl->Task = this->FindTask("total") != nullptr || m_Impl->Tasks.empty() ? std::string("total") : m_Impl->Tasks.front().Name;
    m_Impl->TaskSpeed = Speed::Standard;
    m_Impl->RoiSubset.clear();
  }

  // The tasks stay known, the worker of a deactivated tool goes.
  if (aborted)
    m_Impl->Worker.reset();

  this->InitializedEvent.Send();
}

bool mitk::TotalSegmentatorTool::IsInitialized() const
{
  return !m_Impl->Tasks.empty();
}

bool mitk::TotalSegmentatorTool::IsInitializing() const
{
  return m_Impl->Initializing;
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
  m_Impl->RunFailed = false;
  m_Impl->LastErrorMessage.clear();
  this->RemoveAllPreviewLabels();
}

void mitk::TotalSegmentatorTool::ConfirmCleanUp()
{
  this->RemoveAllPreviewLabels();
  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::TotalSegmentatorTool::DoUpdatePreview(const Image *inputAtTimeStep,
                                                 const Image * /*oldSegAtTimeStep*/,
                                                 MultiLabelSegmentation *previewImage,
                                                 TimeStepType timeStep)
{
  // The failure stays what the GUI reports, and no worker starts again only to
  // be stopped.
  if (m_Impl->RunFailed)
    return;

  m_Impl->LastRunSucceeded = false;
  auto* task = m_ProgressCommand->GetProgressTask();

  // The base UpdatePreview would forward a failure to ErrorMessage, whose
  // listeners present it in a message box apart from the tool. So catch every
  // failure locally and record it; the GUI shows that in its status.
  try
  {
    if (!this->IsInitialized())
      mitkThrow() << "TotalSegmentator has not started yet.";

    if (this->FindTask(m_Impl->Task)->Licensed && !HasLicense())
      mitkThrow() << "The TotalSegmentator task \"" << m_Impl->Task << "\" requires a license. Set it in the settings of TotalSegmentator.";

    auto* worker = &this->GetWorker();

    std::shared_ptr<SharedBuffer> imageBuffer = SharedBuffer::CopyOf(inputAtTimeStep);
    auto labelBuffer = std::make_shared<SharedBuffer>(MakeScalarPixelType<Label::PixelType>(), imageBuffer->GetShape());
    const auto labelImage = labelBuffer->AsImage(*inputAtTimeStep->GetTimeGeometry());

    const auto taskName = m_Impl->Task;
    const auto speed = ToString(m_Impl->TaskSpeed);

    const nlohmann::json segmentParams = {
      { "index_to_world", GetIndexToWorld(inputAtTimeStep->GetGeometry()) },
      { "task", taskName },
      { "speed", speed },
      { "roi_subset", m_Impl->RoiSubset },
      { "device", GetDevice() }
    };

    // TotalSegmentator reports no progress. A cancellation or a deactivation
    // ends the worker at once.
    const auto progress = MakeStopHook(this, task, [this]() { return m_Impl->AbortRequested.load(); });
    auto classes = std::make_shared<nlohmann::json>();

    // Held for the result, as the tool can be deactivated meanwhile.
    const MultiLabelSegmentation::Pointer preview = previewImage;

    this->RunOffGUIThread(
      [this, worker, progress, imageBuffer, labelBuffer, classes, taskName, speed, segmentParams]() {
        const auto missing = worker->Call("totalsegmentator_worker.missing_weights",
          { { "task", taskName }, { "speed", speed } }, {}, progress).get<std::vector<int>>();

        for (std::size_t i = 0; i < missing.size(); ++i)
        {
          const auto phase = missing.size() > 1
            ? "TotalSegmentator: downloading model weights (" + std::to_string(i + 1) + " of " + std::to_string(missing.size()) + ")"
            : std::string("TotalSegmentator: downloading model weights");

          this->PostToGUIThread([this, phase]() {
            if (auto* updateTask = m_ProgressCommand->GetProgressTask(); updateTask != nullptr)
              updateTask->SetName(phase);
          });

          worker->Call("totalsegmentator_worker.download_weights", { { "task_id", missing[i] } }, {}, progress);
        }

        if (!missing.empty())
        {
          this->PostToGUIThread([this]() {
            if (auto* updateTask = m_ProgressCommand->GetProgressTask(); updateTask != nullptr)
              updateTask->SetName(this->GetName());
          });
        }

        *classes = worker->Call("totalsegmentator_worker.segment", segmentParams,
          { { "image", imageBuffer.get() }, { "labels", labelBuffer.get() } }, progress);
      },
      [this, task, preview, labelImage, classes, timeStep](std::exception_ptr error) {
        if (error != nullptr)
        {
          this->EndFailedRun(error, task);
          return;
        }

        // A reply of another form than expected fails the run like any other
        // failure, which removes the labels added so far.
        try
        {
          auto lookupTable = LookupTable::New();
          lookupTable->SetType(LookupTable::LookupTableType::MULTILABEL);

          for (const auto& [index, name] : classes->items())
          {
            const auto value = static_cast<Label::PixelType>(std::stoi(index));

            // Added by an earlier time step of the same update.
            if (preview->ExistLabel(value))
              continue;

            auto label = Label::New(value, name.get<std::string>());

            std::array<double, 3> lookupTableColor;
            lookupTable->GetColor(value, lookupTableColor.data());

            Color color;
            color.SetRed(lookupTableColor[0]);
            color.SetGreen(lookupTableColor[1]);
            color.SetBlue(lookupTableColor[2]);
            label->SetColor(color);

            preview->AddLabel(label, preview->GetActiveLayer(), false);
          }

          preview->UpdateGroupImage(preview->GetActiveLayer(), labelImage, timeStep);
        }
        catch (...)
        {
          this->EndFailedRun(std::current_exception(), task);
          return;
        }

        m_Impl->LastRunSucceeded = true;

        // Deactivated after the run had ended.
        if (m_Impl->AbortRequested)
          m_Impl->Worker.reset();
      });
  }
  catch (...)
  {
    this->EndFailedRun(std::current_exception(), task);
  }
}

void mitk::TotalSegmentatorTool::EndFailedRun(std::exception_ptr error, const ProgressTask* task)
{
  // Leave no half-populated preview behind: an empty preview is how the GUI
  // tells a failure apart from a successful run.
  this->RemoveAllPreviewLabels();
  m_Impl->RunFailed = true;

  try
  {
    std::rethrow_exception(error);
  }
  catch (const Exception& e)
  {
    // A cancellation ends the worker, which may not tell yet that it is gone,
    // and a failure may have left it in a state not worth keeping.
    m_Impl->Worker.reset();

    // The tool was deactivated, or the user cancelled: nothing to report.
    if (m_Impl->AbortRequested || (task != nullptr && task->IsCancelRequested()))
      return;

    if (const auto* workerError = dynamic_cast<const PythonWorkerError*>(&e))
      MITK_ERROR << "TotalSegmentator: " << workerError->GetTraceback();
    else
      MITK_ERROR << "TotalSegmentator: " << Description(e);

    m_Impl->LastErrorMessage = Description(e);
  }
  catch (const std::exception& e)
  {
    m_Impl->LastErrorMessage = e.what();
  }
}
