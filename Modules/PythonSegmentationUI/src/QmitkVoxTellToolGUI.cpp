/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkVoxTellToolGUI.h>
#include <ui_QmitkVoxTellToolGUI.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkPipPackageInfo.h>
#include <mitkPythonContext.h>
#include <mitkPythonHelper.h>
#include <mitkPythonPackageUpdatePrompt.h>
#include <mitkTorchDevice.h>
#include <mitkVoxTellInstall.h>
#include <mitkVoxTellTool.h>

#include <QmitkIconTheme.h>
#include <QmitkInfoCard.h>
#include <QmitkPipInstallDialog.h>

#include <QApplication>
#include <QCheckBox>
#include <QEventLoop>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QShortcut>
#include <QSignalBlocker>
#include <QStringList>
#include <QTimer>

#include <algorithm>
#include <memory>
#include <optional>
#include <set>

MITK_TOOL_GUI_MACRO(MITKPYTHONSEGMENTATIONUI_EXPORT, QmitkVoxTellToolGUI, "")

namespace
{
  constexpr auto LINE_HEIGHT_STYLE = "style='line-height: 1.25'";

  const QString PREFERENCES_NODE = "org.mitk.views.segmentation";
  const QString PREFERENCE_PAGE_ID = "org.mitk.gui.qt.application.VoxTellPreferencePage";

  constexpr auto AUTO_CONFIRM = "VoxTell/autoConfirm";
  constexpr auto CREATE_GROUPS_AS_NEEDED = "VoxTell/createGroupsAsNeeded";

  // The dialogs of this GUI are parented to the window rather than the GUI:
  // their event loop can delete the GUI, which then must not take a dialog
  // that lives on the stack with it.

  // Asks whether to go on with the installation, and whether to download the
  // text encoder with it. It is large and only needed for prompts that VoxTell
  // does not know, so it is left out unless asked for.
  //
  // Returns whether to download the text encoder, or nothing to cancel.
  std::optional<bool> AskToInstall(QWidget* window)
  {
    QMessageBox messageBox(QMessageBox::Question, "Install VoxTell", QString(
      "<p %1>The installation of VoxTell needs several GB of disk space and can take a while.</p>"
      "<p %1>Prompts that VoxTell does not know need an additional text model of about 8 GB. It can be "
      "downloaded now or the first time it is needed.</p>").arg(LINE_HEIGHT_STYLE),
      QMessageBox::NoButton, window);

    // The message box takes ownership.
    auto* textModelCheckBox = new QCheckBox("Download the text model now");
    messageBox.setCheckBox(textModelCheckBox);

    auto* continueButton = messageBox.addButton("Continue", QMessageBox::AcceptRole);
    messageBox.addButton(QMessageBox::Cancel);
    messageBox.setDefaultButton(continueButton);
    messageBox.exec();

    if (messageBox.clickedButton() != continueButton)
      return std::nullopt;

    return textModelCheckBox->isChecked();
  }

  // A message for an operation that blocks the application. It is shown for a
  // moment before the operation starts, so that it is painted in time, and it
  // closes when the returned pointer goes out of scope. Not modal, so that a
  // progress notification shown meanwhile can still be cancelled.
  std::unique_ptr<QMessageBox> ShowBlockingNotice(QWidget* window, const QString& text)
  {
    auto messageBox = std::make_unique<QMessageBox>(QMessageBox::Information, "VoxTell", text, QMessageBox::NoButton, window);

    // Set explicitly, since a message box without buttons gets an OK button
    // when it is shown. There is nothing to confirm while the operation runs.
    messageBox->setStandardButtons(QMessageBox::NoButton);
    messageBox->setWindowModality(Qt::NonModal);
    messageBox->show();

    QEventLoop loop;
    QTimer::singleShot(100, &loop, &QEventLoop::quit);
    loop.exec();

    return messageBox;
  }

  // The prompts as a comma-separated list, e.g. for a status line.
  QString JoinPrompts(const std::vector<std::string>& prompts)
  {
    QStringList list;

    for (const auto& prompt : prompts)
      list << QString::fromStdString(prompt);

    return list.join(", ");
  }

  // Tells the user that a download failed, with what it printed as details.
  void ShowDownloadError(QWidget* window, const QString& output)
  {
    QMessageBox messageBox(QMessageBox::Critical, "VoxTell", QString(
      "<p %1>The download failed. Check your internet connection and try again.</p>").arg(LINE_HEIGHT_STYLE),
      QMessageBox::Ok, window);

    messageBox.setDetailedText(output);
    messageBox.exec();
  }
}

QmitkVoxTellToolGUI::QmitkVoxTellToolGUI()
  : QmitkMultiLabelSegWithPreviewToolGUIBase(),
    m_Ui(std::make_unique<Ui::QmitkVoxTellToolGUI>())
{
  // Only allow confirming once a non-empty preview exists, so a cancelled or
  // failed run leaves the Confirm button disabled. The scope of the labels to
  // transfer still has to be satisfied, which is what the base class checks.
  m_EnableConfirmSegBtnFnc = [this, scopeFunction = m_EnableConfirmSegBtnFnc](bool enabled)
  {
    auto* tool = this->GetTool();

    if (tool == nullptr)
      return false;

    const auto* preview = tool->GetPreviewSegmentation();

    if (preview == nullptr || preview->GetAllLabelValues().empty())
      return false;

    return scopeFunction(enabled);
  };
}

QmitkVoxTellToolGUI::~QmitkVoxTellToolGUI()
{
  if (m_Preferences != nullptr)
  {
    m_Preferences->OnPropertyChanged -=
      mitk::MessageDelegate1<QmitkVoxTellToolGUI, const mitk::IPreferences::ChangeEvent&>(
        this, &QmitkVoxTellToolGUI::OnPreferenceChangedEvent);
  }
}

void QmitkVoxTellToolGUI::InitializeUI(QBoxLayout* mainLayout)
{
  auto wrapperWidget = new QWidget(this);
  mainLayout->addWidget(wrapperWidget);
  m_Ui->setupUi(wrapperWidget);

  m_Ui->segmentButton->setIcon(QmitkIconTheme::GetIcon(
    QStringLiteral(":/org_mitk_icons/icons/tango/scalable/actions/media-playback-start.svg")));

  connect(m_Ui->initializeButton, &QPushButton::toggled, this, &Self::OnInitializeButtonToggled);
  connect(m_Ui->settingsButton, &QPushButton::clicked, this, &Self::OnSettingsButtonClicked);
  connect(m_Ui->segmentButton, &QPushButton::clicked, this, &Self::OnSegmentButtonClicked);

  // Enter starts a new line of the prompts, so it cannot also start the run.
  for (const auto key : { Qt::Key_Return, Qt::Key_Enter })
  {
    auto* shortcut = new QShortcut(QKeySequence(Qt::CTRL | key), m_Ui->promptsTextEdit);
    shortcut->setContext(Qt::WidgetShortcut);
    connect(shortcut, &QShortcut::activated, m_Ui->segmentButton, &QPushButton::click);
  }

  auto prefService = mitk::CoreServices::GetPreferencesService();
  m_Preferences = prefService->GetSystemPreferences()->Node(PREFERENCES_NODE.toStdString());
  m_Preferences->OnPropertyChanged +=
    mitk::MessageDelegate1<QmitkVoxTellToolGUI, const mitk::IPreferences::ChangeEvent&>(
      this, &QmitkVoxTellToolGUI::OnPreferenceChangedEvent);

  m_Ui->autoConfirmCheckBox->setChecked(m_Preferences->GetBool(AUTO_CONFIRM, true));

  connect(m_Ui->autoConfirmCheckBox, &QCheckBox::toggled, this, [this](bool checked)
  {
    m_Preferences->PutBool(AUTO_CONFIRM, checked);
  });

  m_Ui->createGroupsCheckBox->setChecked(m_Preferences->GetBool(CREATE_GROUPS_AS_NEEDED, true));

  connect(m_Ui->createGroupsCheckBox, &QCheckBox::toggled, this, [this](bool checked)
  {
    m_Preferences->PutBool(CREATE_GROUPS_AS_NEEDED, checked);
  });

  this->UpdateInitializeButtonText();
  this->SetStatus("VoxTell is not initialized.");

  Superclass::InitializeUI(mainLayout);
}

mitk::VoxTellTool* QmitkVoxTellToolGUI::GetTool()
{
  return this->GetConnectedToolAs<mitk::VoxTellTool>();
}

void QmitkVoxTellToolGUI::EnableWidgets(bool enabled)
{
  const auto* tool = this->GetTool();

  // The label list of the preview asks for the controls to be enabled while
  // the run replaces its labels, and the base class while a model loads. The
  // Confirm button of the base class has to stay disabled then as well, so the
  // user cannot transfer labels that have no content yet.
  const bool idle = enabled && !m_IsBusy && (tool == nullptr || !tool->IsUpdating());

  Superclass::EnableWidgets(idle);

  // Called by the base class before the controls of this class exist.
  if (m_Ui->initializeButton == nullptr)
    return;

  const bool loaded = tool != nullptr && tool->IsModelLoaded();

  m_Ui->initializeButton->setEnabled(idle);
  m_Ui->settingsButton->setEnabled(idle);
  m_Ui->autoConfirmCheckBox->setEnabled(idle);
  m_Ui->createGroupsCheckBox->setEnabled(idle);
  m_Ui->promptsTextEdit->setEnabled(idle && loaded);
  m_Ui->segmentButton->setEnabled(idle && loaded);
}

void QmitkVoxTellToolGUI::OnInitializeButtonToggled(bool checked)
{
  if (!checked)
  {
    this->GetTool()->UnloadModel();
    this->OnModelUnloaded();
    this->SetStatus("VoxTell is not initialized.");
    return;
  }

  // Installing and loading run event loops, during which the controls must not
  // start anything else, and during which a tool change can delete this GUI.
  m_IsBusy = true;
  this->EnableWidgets(false);

  QPointer<QmitkVoxTellToolGUI> self(this);
  const bool installed = this->Install();

  if (self.isNull())
    return;

  if (!installed)
  {
    this->OnInitializationAborted();
    return;
  }

  this->LoadModel();
}

bool QmitkVoxTellToolGUI::Install()
{
  QPointer<QmitkVoxTellToolGUI> self(this);
  auto* tool = this->GetTool();
  const auto venvName = tool->GetVirtualEnvName();

  // If the venv already exists with VoxTell installed, skip the install dialog:
  // run the version check and offer an in-place update if one is needed.
  if (mitk::PythonHelper::VirtualEnvExists(venvName))
  {
    if (!tool->CreatePythonContext())
      return false;

    bool isInstalled = false;

    try
    {
      isInstalled = tool->IsInstalled();
    }
    catch (const mitk::Exception& e)
    {
      this->SetStatus(QString::fromLocal8Bit(e.GetDescription()), true);
      return false;
    }

    if (isInstalled)
    {
      // A reused virtual environment can hold a VoxTell that predates this MITK
      // build (the venv survives MITK upgrades).
      const auto outcome = mitk::PythonPackage::CheckVersionAndOfferUpdate(this->window(), *tool->GetPythonContext(),
        "VoxTell", mitk::VoxTell::DISTRIBUTION_NAME, mitk::VoxTell::SupportedVersions(), mitk::VoxTell::BuildUpgradeSpec(venvName));

      if (self.isNull() || outcome == mitk::PythonPackage::VersionCheckOutcome::Aborted)
        return false;

      // After an update the interpreter sees the upgraded packages only in a
      // fresh context. Safe, as an update is only offered while no VoxTell
      // modules are loaded.
      return outcome == mitk::PythonPackage::VersionCheckOutcome::KeptInstalled || tool->CreatePythonContext();
    }
  }

  std::optional<mitk::VoxTell::ModelSource> modelSource;

  try
  {
    modelSource = mitk::VoxTell::ParseModelSource(m_Preferences->Get("VoxTell/modelSource", "huggingface"));
  }
  catch (const mitk::Exception& e)
  {
    this->SetStatus(QString::fromLocal8Bit(e.GetDescription()), true);
    return false;
  }

  const auto includeTextModel = AskToInstall(this->window());

  if (self.isNull() || !includeTextModel.has_value())
    return false;

  QmitkPipInstallDialog dialog(mitk::VoxTell::BuildInstallSpec(modelSource.value(), venvName, includeTextModel.value()), this->window());

  if (dialog.exec() != QDialog::Accepted)
    return false;

  // The dialog populated the venv (and possibly created it). Create a fresh
  // context so the embedded interpreter picks up the newly installed packages.
  return tool->CreatePythonContext();
}

void QmitkVoxTellToolGUI::LoadModel()
{
  const auto initMessage = QString(
    "<h3 %1>Initializing VoxTell</h3>"
    "<p %1>Please wait while VoxTell is initialized.</p>").arg(LINE_HEIGHT_STYLE);

  // The notice, the download and the dialogs run event loops, during which a
  // tool change can delete this GUI.
  QPointer<QmitkVoxTellToolGUI> self(this);
  auto* window = this->window();

  // One notice for the whole initialization, a download included.
  auto notice = ShowBlockingNotice(window, initMessage);

  if (self.isNull())
    return;

  try
  {
    if (const auto files = this->GetTool()->GetModelFiles(); !files.empty())
    {
      QString output;
      const auto result = this->Download("Downloading VoxTell model", files, output);

      if (self.isNull())
        return;

      if (result != QmitkVenvProcess::StepResult::Succeeded)
      {
        notice.reset();

        if (result == QmitkVenvProcess::StepResult::Failed)
        {
          ShowDownloadError(window, output);

          if (self.isNull())
            return;
        }
        else
        {
          this->SetStatus("The download was cancelled.");
        }

        this->OnInitializationAborted();
        return;
      }
    }

    this->GetTool()->LoadModel();
  }
  catch (const mitk::Exception& e)
  {
    notice.reset();
    mitk::PythonPackage::ShowInitializationError(window, "VoxTell", e);

    if (self.isNull())
      return;

    this->OnInitializationAborted();
    return;
  }

  notice.reset();
  this->OnModelLoaded();
}

void QmitkVoxTellToolGUI::OnInitializationAborted()
{
  m_IsBusy = false;
  this->UncheckInitializeButton();
  this->EnableWidgets(true);
}

QmitkVenvProcess::StepResult QmitkVoxTellToolGUI::Download(const std::string& displayName, const std::vector<mitk::VoxTell::RepoFiles>& files, QString& output)
{
  const auto result = QmitkVenvProcess::RunStep(this->GetTool()->GetVirtualEnvName(), mitk::VoxTell::BuildDownloadStep(displayName, files), &output);

  if (result == QmitkVenvProcess::StepResult::Failed)
    MITK_ERROR << "VoxTell: " << displayName << " failed:\n" << output.toStdString();

  return result;
}

bool QmitkVoxTellToolGUI::EmbedUnknownPrompts(const std::vector<std::string>& prompts)
{
  // The download, the notice and the dialogs run event loops, during which a
  // tool change can delete this GUI.
  QPointer<QmitkVoxTellToolGUI> self(this);
  auto* window = this->window();

  m_IsBusy = true;
  this->EnableWidgets(false);

  const auto finish = [this, &self](bool succeeded)
  {
    if (self.isNull())
      return false;

    m_IsBusy = false;
    this->EnableWidgets(true);

    // There is no model left for the run then.
    if (this->UnloadIfSettingsChanged())
      return false;

    return succeeded;
  };

  try
  {
    if (!this->GetTool()->IsTextModelLoaded() && !this->GetTool()->IsTextModelCached())
    {
      if (const auto files = this->GetTool()->GetTextModelFiles(); !files.empty())
      {
        this->SetStatus("Downloading the text model...");

        QString output;
        const auto result = this->Download("Downloading VoxTell text model", files, output);

        if (self.isNull())
          return false;

        if (result != QmitkVenvProcess::StepResult::Succeeded)
        {
          this->SetStatus(result == QmitkVenvProcess::StepResult::Cancelled
            ? "The download of the text model was cancelled."
            : "The text model could not be downloaded.", true);

          if (result == QmitkVenvProcess::StepResult::Failed)
            ShowDownloadError(window, output);

          return finish(false);
        }
      }
    }

    this->SetStatus("Processing the prompts with the text model...");

    const auto notice = ShowBlockingNotice(window, QString(
      "<h3 %1>Processing unknown prompts</h3>"
      "<p %1>VoxTell uses its text model to understand the prompts it does not know. Loading the "
      "text model for the first time can take a minute, during which MITK does not respond.</p>")
      .arg(LINE_HEIGHT_STYLE));

    if (self.isNull())
      return false;

    this->GetTool()->EmbedPrompts(prompts);
  }
  catch (const mitk::Exception& e)
  {
    if (self.isNull())
      return false;

    QString message = QString::fromLocal8Bit(e.GetDescription());

    if (message.contains("out of memory", Qt::CaseInsensitive))
      message += " Try fewer prompts at once, or use the CPU in the VoxTell settings.";

    this->SetStatus(message, true);
    return finish(false);
  }

  return finish(true);
}

void QmitkVoxTellToolGUI::OnModelLoaded()
{
  m_IsBusy = false;
  m_SettingsChanged = false;
  m_UnknownPromptsConfirmed = false;

  this->UpdateInitializeButtonText();
  this->UpdatePromptCompletions();
  this->EnableWidgets(true);
  this->SetStatus("Ready. Enter one prompt per line and click Segment.");

  const auto backend = this->GetTool()->GetBackend();

  // Somebody who chose the CPU knows what to expect.
  if (backend != mitk::Torch::Backend::CPU || m_Preferences->Get("VoxTell/backend", "auto") == "cpu")
    return;

#if defined(__APPLE__)
  const QString message = QString(
    "<h3 %1>Running on CPU</h3>"
    "<p %1>VoxTell has no GPU acceleration on macOS and runs on the CPU, which is "
    "<em>significantly slower</em>.</p>").arg(LINE_HEIGHT_STYLE);
#else
  const auto minimum = mitk::Torch::MinimumComputeCapability();

  const QString message = QString(
    "<h3 %1>No compatible CUDA device detected</h3>"
    "<p %1>VoxTell runs on the CPU, which is <em>significantly slower</em>.</p>"
    "<p %1>For fast results, a compatible NVIDIA GPU (compute capability %2.%3 or higher) "
    "with sufficient memory is required. Select it in the VoxTell settings.</p>")
    .arg(LINE_HEIGHT_STYLE).arg(minimum.Major).arg(minimum.Minor);
#endif

  QMessageBox::warning(this->window(), "VoxTell", message);
}

void QmitkVoxTellToolGUI::OnModelUnloaded()
{
  this->UncheckInitializeButton();
  this->UpdatePromptCompletions();
  this->EnableWidgets(true);
}

void QmitkVoxTellToolGUI::OnSettingsButtonClicked()
{
  mitk::CoreServices::GetPreferencesService()->OpenPreferencesDialog(PREFERENCE_PAGE_ID.toStdString());

  // The preferences dialog is modal; on return the user may have uninstalled
  // VoxTell there, which fires no preference event.
  this->UpdateInitializeButtonText();
}

std::vector<std::string> QmitkVoxTellToolGUI::ReadPrompts() const
{
  std::vector<std::string> prompts;

  // VoxTell does not tell "Liver" from "liver", so neither does this.
  std::set<QString> seen;

  for (const auto& line : m_Ui->promptsTextEdit->toPlainText().split('\n'))
  {
    const auto prompt = line.simplified();

    if (prompt.isEmpty() || !seen.insert(prompt.toLower()).second)
      continue;

    prompts.push_back(prompt.toStdString());
  }

  return prompts;
}

void QmitkVoxTellToolGUI::UpdatePromptCompletions()
{
  QStringList completions;

  if (const auto* tool = this->GetTool(); tool != nullptr)
  {
    try
    {
      for (const auto& prompt : tool->GetKnownPrompts())
        completions << QString::fromStdString(prompt);
    }
    catch (const mitk::Exception&)
    {
      // The tool logged it. Prompts can still be typed, only without suggestions.
    }
  }

  m_Ui->promptsTextEdit->SetCompletions(completions);
}

void QmitkVoxTellToolGUI::OnSegmentButtonClicked()
{
  auto* tool = this->GetTool();

  if (tool == nullptr || !tool->IsModelLoaded())
    return;

  const auto enteredPrompts = this->ReadPrompts();

  if (enteredPrompts.empty())
  {
    this->SetStatus("Enter at least one prompt, one per line.", true);
    return;
  }

  // The dialogs, the text model and the run process events, during which a
  // tool change can delete this GUI. Detect that via a QPointer and bail out
  // before touching any members, rather than crashing on a freed 'this'.
  QPointer<QmitkVoxTellToolGUI> self(this);

  auto prompts = enteredPrompts;
  std::vector<std::string> unknownPrompts;
  bool isTextModelCached = false;

  try
  {
    unknownPrompts = tool->GetPromptsWithoutPrecomputedEmbedding(prompts);

    if (!unknownPrompts.empty())
      isTextModelCached = tool->IsTextModelCached();
  }
  catch (const mitk::Exception& e)
  {
    this->SetStatus(QString::fromLocal8Bit(e.GetDescription()), true);
    return;
  }

  // Only a text model that still has to be downloaded is worth asking about.
  // Whoever downloaded it, during the installation for example, wants it used.
  if (!unknownPrompts.empty() && !m_UnknownPromptsConfirmed && !isTextModelCached)
  {
    QString promptList;

    for (const auto& prompt : unknownPrompts)
      promptList += QString("<li>%1</li>").arg(QString::fromStdString(prompt).toHtmlEscaped());

    const bool hasKnownPrompts = unknownPrompts.size() < prompts.size();

    QMessageBox box(QMessageBox::Question, "VoxTell", QString(
      "<h3 %1>Prompts that VoxTell does not know</h3>"
      "<p %1>VoxTell does not know these prompts:</p>"
      "<ul %1>%2</ul>"
      "<p %1>To understand them, VoxTell uses a text model. It is downloaded once, about 8 GB, and "
      "loading it can take a minute, during which MITK does not respond. It needs about 8 GB of "
      "memory on the GPU, or about 16 GB on the CPU.</p>"
      "<p %1><b>Do you want to use the text model for these prompts?</b></p>")
      .arg(LINE_HEIGHT_STYLE).arg(promptList), QMessageBox::NoButton, this->window());

    auto* useTextModelButton = box.addButton("Use text model", QMessageBox::AcceptRole);
    QPushButton* knownOnlyButton = nullptr;

    if (hasKnownPrompts)
      knownOnlyButton = box.addButton("Known prompts only", QMessageBox::ActionRole);

    auto* cancelButton = box.addButton("Cancel", QMessageBox::RejectRole);

    box.setDefaultButton(cancelButton);
    box.setEscapeButton(cancelButton);
    box.exec();

    if (self.isNull())
      return;

    if (box.clickedButton() == useTextModelButton)
    {
      m_UnknownPromptsConfirmed = true;
    }
    else if (knownOnlyButton != nullptr && box.clickedButton() == knownOnlyButton)
    {
      // Asking again next time is intended: the unknown prompts stay in the
      // text field and are still unknown to VoxTell.
      std::erase_if(prompts, [&unknownPrompts](const std::string& prompt)
      {
        return std::find(unknownPrompts.begin(), unknownPrompts.end(), prompt) != unknownPrompts.end();
      });

      unknownPrompts.clear();
    }
    else
    {
      return;
    }
  }

  // The text model is downloaded and loaded before the run, where the user can
  // be told about it, instead of in the middle of the run.
  if (!unknownPrompts.empty() && !this->EmbedUnknownPrompts(unknownPrompts))
    return;

  // Re-fetch: the download for the text model ran an event loop.
  tool = this->GetTool();

  if (tool == nullptr)
    return;

  tool->SetPrompts(prompts);
  tool->SetCreateGroupsAsNeeded(m_Ui->createGroupsCheckBox->isChecked());

  this->SetStatus("Segmenting. This can take a while...");

  try
  {
    tool->UpdatePreview();
  }
  catch (const std::exception& e)
  {
    if (self.isNull())
      return;

    this->SetStatus(QString("Error: %1").arg(e.what()), true);
    this->UnloadIfSettingsChanged();
    return;
  }

  if (self.isNull())
    return;

  // Re-fetch: the connected tool could have changed while the run pumped events.
  tool = this->GetTool();

  if (tool == nullptr)
    return;

  auto* preview = tool->GetPreviewSegmentation();
  const auto promptsWithoutResult = tool->GetPromptsWithoutResult();

  if (preview != nullptr && !preview->GetAllLabelValues().empty())
  {
    this->SetLabelSetPreview(preview);
    this->ActualizePreviewLabelVisibility();

    QString status = "Segmentation finished.";
    bool confirmFailed = false;

    // Only what the Confirm button would transfer: with "Transfer selected
    // labels", the user picks the labels first.
    if (m_Ui->autoConfirmCheckBox->isChecked() && m_EnableConfirmSegBtnFnc(true))
    {
      try
      {
        this->OnAcceptPreview();
        status = "Segmentation finished and confirmed.";
      }
      catch (const std::exception& e)
      {
        status = QString("The result could not be confirmed: %1").arg(e.what());
        confirmFailed = true;
      }

      // Confirming computes the preview of further time steps if needed, which
      // processes events.
      if (self.isNull())
        return;
    }

    if (!confirmFailed && !promptsWithoutResult.empty())
      status += QString(" Nothing found for: %1.").arg(JoinPrompts(promptsWithoutResult));

    this->SetStatus(status, confirmFailed);
  }
  else if (!promptsWithoutResult.empty())
  {
    this->SetStatus(QString("Nothing found for: %1.").arg(JoinPrompts(promptsWithoutResult)), true);
  }
  else if (const auto& error = tool->GetLastErrorMessage(); error.empty())
  {
    // A cancellation by the user leaves no error message, unlike a failure.
    this->SetStatus("Segmentation was cancelled.");
  }
  else
  {
    QString message = QString::fromStdString(error);

    if (message.contains("out of memory", Qt::CaseInsensitive))
      message += " Try fewer prompts at once, or use the CPU in the VoxTell settings.";

    this->SetStatus(message, true);
  }

  // Prompts that the text encoder processed for this run are known now.
  if (!this->UnloadIfSettingsChanged())
    this->UpdatePromptCompletions();
}

bool QmitkVoxTellToolGUI::UnloadIfSettingsChanged()
{
  if (!m_SettingsChanged)
    return false;

  m_SettingsChanged = false;

  auto* tool = this->GetTool();

  if (tool == nullptr || !tool->IsModelLoaded())
    return false;

  tool->UnloadModel();
  this->OnModelUnloaded();
  this->SetStatus("The settings changed. Initialize VoxTell again to apply them.");

  return true;
}

void QmitkVoxTellToolGUI::UncheckInitializeButton()
{
  // Revert the toggle without re-entering OnInitializeButtonToggled, which would
  // immediately start or end something against the user's intent.
  {
    QSignalBlocker blocker(m_Ui->initializeButton);
    m_Ui->initializeButton->setChecked(false);
  }

  // Every path that reverts the toggle may have changed the install state just
  // before (install cancelled, or completed without loading), so re-evaluate
  // the label here.
  this->UpdateInitializeButtonText();
}

void QmitkVoxTellToolGUI::UpdateInitializeButtonText()
{
  // While a model is loaded the button unloads it. Otherwise it shows what the
  // next click will do: install first if VoxTell is missing, or initialize. The
  // venv check is a plain filesystem probe, cheap enough to run on every
  // update, and matches the first gate in Install().
  const auto* tool = this->GetTool();

  if (tool != nullptr && tool->IsModelLoaded())
  {
    m_Ui->initializeButton->setText("Uninitialize");
    return;
  }

  if (tool != nullptr && !mitk::PythonHelper::VirtualEnvExists(tool->GetVirtualEnvName()))
  {
    m_Ui->initializeButton->setText("Install VoxTell");
    return;
  }

  m_Ui->initializeButton->setText("Initialize");
}

void QmitkVoxTellToolGUI::SetStatus(const QString& message, bool isError)
{
  // Prompts and errors from Python can contain angle brackets.
  m_Ui->statusCard->SetMessage(message.toHtmlEscaped(), isError ? QmitkInfoCard::Severity::Error : QmitkInfoCard::Severity::Info);
}

void QmitkVoxTellToolGUI::OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent& event)
{
  auto* tool = this->GetTool();

  if (tool == nullptr || !tool->IsModelLoaded())
    return;

  // A change to a setting baked into the loaded model (backend, model source)
  // makes it stale, so unload it; the user then initializes again with the new
  // settings. SetProperty fires this only on an actual value change, so clicking
  // OK without edits is a no-op.
  const auto& key = event.GetProperty();
  const auto& sessionDefiningKeys = mitk::VoxTellTool::GetSessionDefiningPreferences();

  if (std::none_of(sessionDefiningKeys.begin(), sessionDefiningKeys.end(), [&key](const auto& entry) { return entry.first == key; }))
    return;

  // The model folder is baked in only while the model is loaded from it.
  if (key == "VoxTell/localModelPath" && tool->GetModelSource() != mitk::VoxTell::ModelSource::Local)
    return;

  m_SettingsChanged = true;

  // The settings can be changed while a run or its preparation process events.
  // The model cannot be unloaded while it is in use, so that waits until the
  // run or its preparation is over.
  if (!tool->IsUpdating() && !m_IsBusy)
    this->UnloadIfSettingsChanged();
}
