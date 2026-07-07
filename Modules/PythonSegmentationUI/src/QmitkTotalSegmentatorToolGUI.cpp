/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkTotalSegmentatorToolGUI.h>
#include <ui_QmitkTotalSegmentatorToolGUI.h>

#include <mitkTotalSegmentatorInstall.h>
#include <mitkTotalSegmentatorTool.h>

#include <mitkCoreServices.h>
#include <mitkFileSystem.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkPythonHelper.h>

#include <QmitkPipInstallDialog.h>
#include <QmitkStyleManager.h>
#include <QmitkVenvProcess.h>

#include <QApplication>
#include <QComboBox>
#include <QEventLoop>
#include <QIcon>
#include <QPointer>
#include <QProcess>
#include <QProgressDialog>
#include <QStringList>

#include <nlohmann/json.hpp>

#include <fstream>

MITK_TOOL_GUI_MACRO(MITKPYTHONSEGMENTATIONUI_EXPORT, QmitkTotalSegmentatorToolGUI, "")

namespace
{
  const QString PREFERENCES_NODE = "org.mitk.views.segmentation";
  const QString PREFERENCE_PAGE_ID = "org.mitk.gui.qt.application.TotalSegmentatorPreferencePage";

  // Combo-box item-data roles.
  constexpr int TaskRole = Qt::UserRole;         // QString: the bare task id
  constexpr int SpeedRole = Qt::UserRole + 1;    // int: TotalSegmentatorTool::Speed
  constexpr int LicensedRole = Qt::UserRole + 2; // bool: task needs a license

  // Single source of truth for the runnable TotalSegmentator tasks of the pinned
  // 2.x line (the released package exposes no machine-readable task export).
  // 'licensed' tasks stay visible but disabled until a license is set; 'multiRes'
  // tasks additionally offer the fast (3 mm) and fastest (6 mm) variants (the
  // others are single-resolution and TotalSegmentator rejects --fast for them).
  // Modality is derived from the "_mr" suffix; label names come from the per-task
  // JSON dumped at install time.
  //
  // Entries are listed in display order: the general-purpose "total"/"total_mr"
  // first, then the remaining free tasks alphabetically, then the licensed tasks
  // alphabetically. Keep this ordering when adding tasks.
  struct TaskInfo
  {
    const char* name;
    bool licensed;
    bool multiRes;
  };

  const TaskInfo TASKS[] = {
    {"total", false, true},
    {"total_mr", false, true},
    {"abdominal_muscles", false, false},
    {"body", false, false},
    {"body_mr", false, false},
    {"brain_aneurysm", false, false},
    {"breasts", false, false},
    {"cerebral_bleed", false, false},
    {"craniofacial_structures", false, false},
    {"head_glands_cavities", false, false},
    {"head_muscles", false, false},
    {"headneck_bones_vessels", false, false},
    {"headneck_muscles", false, false},
    {"hip_implant", false, false},
    {"kidney_cysts", false, false},
    {"liver_lesions", false, false},
    {"liver_lesions_mr", false, false},
    {"liver_segments", false, false},
    {"liver_segments_mr", false, false},
    {"liver_vessels", false, false},
    {"lung_nodules", false, false},
    {"lung_vessels", false, false},
    {"oculomotor_muscles", false, false},
    {"pleural_pericard_effusion", false, false},
    {"teeth", false, false},
    {"trunk_cavities", false, false},
    {"ventricle_parts", false, false},
    {"vertebrae_mr", false, false},
    {"aortic_sinuses", true, false},
    {"appendicular_bones", true, false},
    {"appendicular_bones_mr", true, false},
    {"brain_structures", true, false},
    {"coronary_arteries", true, false},
    {"face", true, false},
    {"face_mr", true, false},
    {"heartchambers_highres", true, false},
    {"thigh_shoulder_muscles", true, false},
    {"thigh_shoulder_muscles_mr", true, false},
    {"tissue_4_types", true, false},
    {"tissue_types", true, false},
    {"tissue_types_mr", true, false},
    {"vertebrae_body", true, false},
    {"vertebrae_pp", true, false},
    {"vertebrae_pp_refined", true, false}
  };

  // Absolute path to the managed environment's TotalSegmentator console script,
  // or an empty path if the environment location cannot be resolved.
  fs::path ExecutablePath()
  {
    return mitk::PythonHelper::GetVirtualEnvScriptPath(mitk::TotalSegmentator::VENV_NAME, "TotalSegmentator");
  }

  // Reads the { labelId: labelName } map for the given task from the JSON dumped at
  // install time. Returns an empty map when the file or task is missing; the tool
  // auto-names any label ids that are not covered.
  std::map<mitk::Label::PixelType, std::string> ReadLabelNames(const fs::path& venvDir, const std::string& task)
  {
    std::map<mitk::Label::PixelType, std::string> result;

    if (venvDir.empty())
      return result;

    const auto jsonPath = venvDir / "mitk_totalseg_tasks.json";
    std::error_code error;
    if (!fs::exists(jsonPath, error))
      return result;

    try
    {
      std::ifstream stream(jsonPath);
      nlohmann::json data;
      stream >> data;

      if (!data.contains(task))
        return result;

      for (auto it = data[task].begin(); it != data[task].end(); ++it)
      {
        const int id = std::stoi(it.key());
        result[static_cast<mitk::Label::PixelType>(id)] = it.value().get<std::string>();
      }
    }
    catch (...)
    {
      result.clear();
    }

    return result;
  }
}

QmitkTotalSegmentatorToolGUI::QmitkTotalSegmentatorToolGUI()
  : QmitkMultiLabelSegWithPreviewToolGUIBase(),
    m_Ui(std::make_unique<Ui::QmitkTotalSegmentatorToolGUI>())
{
  // Only allow confirming once a non-empty preview exists, so a cancelled or
  // empty run leaves the Confirm button disabled.
  m_EnableConfirmSegBtnFnc = [this](bool enabled)
  {
    auto tool = this->GetTool();
    if (tool == nullptr)
      return false;

    auto preview = tool->GetPreviewSegmentation();
    if (preview == nullptr || preview->GetAllLabelValues().empty())
      return false;

    return enabled;
  };
}

QmitkTotalSegmentatorToolGUI::~QmitkTotalSegmentatorToolGUI()
{
  if (m_Preferences != nullptr)
  {
    m_Preferences->OnPropertyChanged -=
      mitk::MessageDelegate1<QmitkTotalSegmentatorToolGUI, const mitk::IPreferences::ChangeEvent&>(
        this, &QmitkTotalSegmentatorToolGUI::OnPreferenceChangedEvent);
  }
}

void QmitkTotalSegmentatorToolGUI::InitializeUI(QBoxLayout* mainLayout)
{
  auto wrapperWidget = new QWidget(this);
  mainLayout->addWidget(wrapperWidget);
  m_Ui->setupUi(wrapperWidget);

  // Keep the task combo the same height as the Settings button they share the top
  // row with; a QComboBox's default height is otherwise slightly shorter.
  m_Ui->taskComboBox->setMinimumHeight(m_Ui->settingsButton->sizeHint().height());

  m_Ui->runButton->setIcon(QmitkStyleManager::ThemeIcon(
    QStringLiteral(":/org_mitk_icons/icons/tango/scalable/actions/media-playback-start.svg")));

  connect(m_Ui->installButton, &QPushButton::clicked, this, &Self::OnInstallButtonClicked);
  connect(m_Ui->settingsButton, &QPushButton::clicked, this, &Self::OnSettingsButtonClicked);
  connect(m_Ui->runButton, &QPushButton::clicked, this, &Self::OnRunButtonClicked);
  connect(m_Ui->taskComboBox, &QComboBox::currentIndexChanged, this, &Self::UpdateRunButtonState);

  auto prefService = mitk::CoreServices::GetPreferencesService();
  m_Preferences = prefService->GetSystemPreferences()->Node(PREFERENCES_NODE.toStdString());
  if (m_Preferences != nullptr)
  {
    m_Preferences->OnPropertyChanged +=
      mitk::MessageDelegate1<QmitkTotalSegmentatorToolGUI, const mitk::IPreferences::ChangeEvent&>(
        this, &QmitkTotalSegmentatorToolGUI::OnPreferenceChangedEvent);
  }

  this->RefreshInstallState();

  Superclass::InitializeUI(mainLayout);
}

mitk::TotalSegmentatorTool* QmitkTotalSegmentatorToolGUI::GetTool()
{
  return this->GetConnectedToolAs<mitk::TotalSegmentatorTool>();
}

void QmitkTotalSegmentatorToolGUI::RefreshInstallState()
{
  const bool installed =
    mitk::PythonHelper::VirtualEnvExists(mitk::TotalSegmentator::VENV_NAME) && fs::exists(ExecutablePath());

  m_Ui->installButton->setVisible(!installed);
  m_Ui->taskComboBox->setVisible(installed);
  m_Ui->runButton->setVisible(installed);

  if (installed)
  {
    // PopulateTasks ends by calling UpdateRunButtonState, which sets the status
    // (ready, or a license hint if the restored task needs one).
    this->PopulateTasks();
  }
  else
  {
    this->SetStatus("TotalSegmentator is not installed. Click 'Install TotalSegmentator'.");
  }
}

void QmitkTotalSegmentatorToolGUI::PopulateTasks()
{
  // Preserve the current selection across the rebuild: PopulateTasks runs on every
  // RefreshInstallState (e.g. after merely opening Settings), so without this the
  // user's chosen task would silently reset to "total".
  const QString previousTask = m_Ui->taskComboBox->currentData(TaskRole).toString();
  const int previousSpeed = m_Ui->taskComboBox->currentData(SpeedRole).toInt();

  const bool hasLicense = m_Preferences != nullptr && m_Preferences->GetBool("TotalSeg/hasLicense", false);

  // Rebuild without emitting currentIndexChanged for each intermediate state; the
  // final UpdateRunButtonState() below reflects the restored selection once.
  m_Ui->taskComboBox->blockSignals(true);
  m_Ui->taskComboBox->clear();

  auto addItem = [&](const QString& label, const QString& task, int speed, bool licensed)
  {
    m_Ui->taskComboBox->addItem(label);
    const int index = m_Ui->taskComboBox->count() - 1;
    m_Ui->taskComboBox->setItemData(index, task, TaskRole);
    m_Ui->taskComboBox->setItemData(index, speed, SpeedRole);
    m_Ui->taskComboBox->setItemData(index, licensed, LicensedRole);

    // Licensed tasks stay selectable. Disabling them made the drop-down misbehave
    // (clicking a disabled entry jumped to an arbitrary enabled one); instead they
    // get a "License required: " prefix and the Run button is gated in
    // UpdateRunButtonState.
    if (licensed && !hasLicense)
      m_Ui->taskComboBox->setItemData(
        index, QStringLiteral("Requires a TotalSegmentator license. Set it in Settings."), Qt::ToolTipRole);
  };

  for (const auto& info : TASKS)
  {
    const QString task = QString::fromLatin1(info.name);

    // Display-only annotations; the stored task id (TaskRole) stays the bare name
    // that TotalSegmentator expects.
    const QString modality = task.endsWith("_mr") ? QStringLiteral(" (MR)") : QString();
    const QString prefix = info.licensed ? QStringLiteral("License required: ") : QString();

    addItem(prefix + task + modality, task, 0, info.licensed);

    if (info.multiRes)
    {
      addItem(prefix + task + " - fast (3 mm)" + modality, task, 1, info.licensed);
      addItem(prefix + task + " - fastest (6 mm)" + modality, task, 2, info.licensed);
    }
  }

  // Restore the previous selection, but not a licensed task that is no longer
  // runnable (e.g. the license was just removed) - fall back to the first entry.
  int restored = -1;
  for (int i = 0; i < m_Ui->taskComboBox->count(); ++i)
  {
    if (m_Ui->taskComboBox->itemData(i, TaskRole).toString() == previousTask &&
        m_Ui->taskComboBox->itemData(i, SpeedRole).toInt() == previousSpeed)
    {
      const bool licensed = m_Ui->taskComboBox->itemData(i, LicensedRole).toBool();
      if (!(licensed && !hasLicense))
        restored = i;
      break;
    }
  }

  m_Ui->taskComboBox->setCurrentIndex(restored >= 0 ? restored : 0);
  m_Ui->taskComboBox->blockSignals(false);

  this->UpdateRunButtonState();
}

void QmitkTotalSegmentatorToolGUI::UpdateRunButtonState()
{
  const int index = m_Ui->taskComboBox->currentIndex();
  if (index < 0)
  {
    m_Ui->runButton->setEnabled(false);
    return;
  }

  const bool licensed = m_Ui->taskComboBox->itemData(index, LicensedRole).toBool();
  const bool hasLicense = m_Preferences != nullptr && m_Preferences->GetBool("TotalSeg/hasLicense", false);

  if (licensed && !hasLicense)
  {
    m_Ui->runButton->setEnabled(false);
    this->SetStatus("This task requires a TotalSegmentator license. Set it in Settings.", true);
  }
  else
  {
    m_Ui->runButton->setEnabled(true);
    this->SetStatus("TotalSegmentator is ready. Select a task and run.");
  }
}

void QmitkTotalSegmentatorToolGUI::OnInstallButtonClicked()
{
  auto spec = mitk::TotalSegmentator::BuildInstallSpec(mitk::TotalSegmentator::VENV_NAME);

  QmitkPipInstallDialog dialog(spec, this);

  if (dialog.exec() == QDialog::Accepted)
    this->RefreshInstallState();
}

void QmitkTotalSegmentatorToolGUI::OnSettingsButtonClicked()
{
  mitk::CoreServices::GetPreferencesService()->OpenPreferencesDialog(PREFERENCE_PAGE_ID.toStdString());

  // The preferences dialog is modal; on return the user may have uninstalled,
  // cleared weights, or changed the license, so re-evaluate what the tool shows.
  this->RefreshInstallState();
}

void QmitkTotalSegmentatorToolGUI::OnRunButtonClicked()
{
  auto tool = this->GetTool();
  if (tool == nullptr)
    return;

  const int index = m_Ui->taskComboBox->currentIndex();
  if (index < 0)
    return;

  // Defensive: the Run button is already disabled for a licensed task without a
  // license, but guard here too in case the cached license state lags a change.
  const bool licensed = m_Ui->taskComboBox->itemData(index, LicensedRole).toBool();
  const bool hasLicense = m_Preferences != nullptr && m_Preferences->GetBool("TotalSeg/hasLicense", false);
  if (licensed && !hasLicense)
  {
    this->SetStatus("This task requires a TotalSegmentator license. Set it in Settings.", true);
    return;
  }

  const auto exe = ExecutablePath();
  if (!fs::exists(exe))
  {
    this->SetStatus("TotalSegmentator executable not found. Please reinstall.", true);
    this->RefreshInstallState();
    return;
  }

  const QString task = m_Ui->taskComboBox->itemData(index, TaskRole).toString();
  const int speed = m_Ui->taskComboBox->itemData(index, SpeedRole).toInt();
  const std::string device = m_Preferences != nullptr ? m_Preferences->Get("TotalSeg/device", "gpu") : std::string("gpu");
  const auto venvDir = mitk::PythonHelper::GetVirtualEnvPath(mitk::TotalSegmentator::VENV_NAME);

  tool->SetTask(task.toStdString());
  tool->SetSpeed(static_cast<mitk::TotalSegmentatorTool::Speed>(speed));
  tool->SetDevice(device);
  // toStdString() yields UTF-8, which RunProcess decodes losslessly with
  // QString::fromStdString; fs::path::string() would corrupt non-ASCII paths.
  tool->SetExecutablePath(QmitkVenvProcess::ToQString(exe).toStdString());
  tool->SetLabelNameLookup(ReadLabelNames(venvDir, task.toStdString()));
  tool->SetCommandRunner(
    [this](const std::string& executable, const std::vector<std::string>& args)
    { return this->RunProcess(executable, args); });

  // UpdatePreview() spins a nested event loop (in RunProcess), during which a
  // tool-manager change can delete this GUI. Detect that via a QPointer and bail
  // out before touching any members, rather than crashing on a freed 'this'.
  QPointer<QmitkTotalSegmentatorToolGUI> self(this);

  m_Ui->runButton->setEnabled(false);
  this->SetStatus("Running TotalSegmentator. This can take a while...");
  qApp->processEvents();
  if (self.isNull())
    return;

  try
  {
    tool->UpdatePreview();
  }
  catch (const std::exception& e)
  {
    if (self.isNull())
      return;
    this->SetStatus(QString("Error: %1").arg(e.what()), true);
    m_Ui->runButton->setEnabled(true);
    return;
  }
  catch (...)
  {
    if (self.isNull())
      return;
    this->SetStatus("An unknown error occurred while running TotalSegmentator.", true);
    m_Ui->runButton->setEnabled(true);
    return;
  }

  if (self.isNull())
    return;

  // Re-fetch: the connected tool could have changed while the nested event loop
  // inside RunProcess was running.
  tool = this->GetTool();
  if (tool == nullptr)
    return;

  m_Ui->runButton->setEnabled(true);

  auto preview = tool->GetPreviewSegmentation();
  if (preview != nullptr && !preview->GetAllLabelValues().empty())
  {
    this->SetLabelSetPreview(preview);
    this->ActualizePreviewLabelVisibility();
    this->SetStatus("Segmentation finished.");
  }
  else
  {
    // Distinguish a genuine failure (which the tool records) from a user
    // cancellation, which leaves no error message.
    const auto& error = tool->GetLastErrorMessage();
    if (!error.empty())
      this->SetStatus(QString::fromStdString(error), true);
    else
      this->SetStatus("Segmentation was cancelled or produced no result.", true);
  }
}

bool QmitkTotalSegmentatorToolGUI::RunProcess(const std::string& executable, const std::vector<std::string>& args)
{
  // The executable is UTF-8 (set from ToQString(...).toStdString()) and is decoded
  // below with QString::fromStdString. The arguments include filesystem paths that
  // IOUtil produces in the platform's native narrow encoding, so decode those with
  // fromLocal8Bit to match (ASCII flags decode identically either way).
  QStringList arguments;
  for (const auto& arg : args)
    arguments << QString::fromLocal8Bit(arg.c_str());

  QProcess process;
  process.setProcessEnvironment(QmitkVenvProcess::CleanEnvironment());
  process.setProcessChannelMode(QProcess::MergedChannels); // progress is printed to stderr

  // No fixed timeout: a run can legitimately take many minutes (and download
  // model weights on first use). Cancellation is offered through the dialog.
  // Parent to the top-level window, not to this tool GUI: a tool change delivered
  // during the nested event loop below can delete this GUI, and a child dialog on
  // the stack would then be double-freed by the parent's child cleanup.
  QProgressDialog dialog("Running TotalSegmentator...", "Cancel", 0, 0, this->window());
  dialog.setWindowTitle("TotalSegmentator");
  dialog.setWindowModality(Qt::WindowModal);
  dialog.setMinimumDuration(0);

  QEventLoop loop;
  bool downloadingSeen = false;

  QObject::connect(&process, &QProcess::readyRead, &process, [&]()
  {
    const QString chunk = QString::fromLocal8Bit(process.readAll());
    MITK_INFO << chunk.toStdString();

    // The first weights download for a task can take minutes; say so instead of
    // leaving the user staring at an unexplained wait.
    if (!downloadingSeen && chunk.contains("Downloading", Qt::CaseInsensitive))
    {
      downloadingSeen = true;
      dialog.setLabelText("Downloading model weights (first run for this task)...");
    }
  });

  QObject::connect(&process, &QProcess::finished, &loop, &QEventLoop::quit);
  QObject::connect(&process, &QProcess::errorOccurred, &loop, &QEventLoop::quit);
  QObject::connect(&dialog, &QProgressDialog::canceled, &loop, &QEventLoop::quit);

  process.start(QString::fromStdString(executable), arguments);

  if (!process.waitForStarted(15000))
  {
    process.kill();
    process.waitForFinished(2000);
    return false;
  }

  dialog.show();

  // The process can finish between waitForStarted() and here; only enter the loop
  // while it is still running, otherwise quit() would never arrive.
  if (process.state() != QProcess::NotRunning)
    loop.exec();

  if (process.state() != QProcess::NotRunning)
  {
    // Cancelled: stop the still-running process.
    process.kill();
    process.waitForFinished(2000);
    return false;
  }

  return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

void QmitkTotalSegmentatorToolGUI::SetStatus(const QString& message, bool isError)
{
  m_Ui->statusLabel->setText(message);
  m_Ui->statusLabel->setStyleSheet(isError ? "color: red;" : QString());
}

void QmitkTotalSegmentatorToolGUI::OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent& event)
{
  // Re-enable licensed tasks as soon as a license is applied via the settings.
  if (event.GetProperty() == "TotalSeg/hasLicense")
    this->RefreshInstallState();
}
