/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitknnInteractiveToolGUI.h>
#include <ui_QmitknnInteractiveToolGUI.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkLabelSetImageConverter.h>
#include <mitknnInteractiveInstall.h>
#include <mitknnInteractiveInteractor.h>
#include <mitknnInteractiveModel.h>
#include <mitknnInteractiveVersion.h>
#include <mitkPythonContext.h>
#include <mitkPythonHelper.h>
#include <mitkToolManagerProvider.h>

#include <QmitkMultiLabelInspector.h>
#include <QmitknnInteractiveInstallModeDialog.h>
#include <QmitkPipInstallDialog.h>
#include <mitkPipPackageInfo.h>
#include <QmitkStyleManager.h>

#include <QApplication>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QColor>
#include <QCoreApplication>
#include <QIcon>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QScopeGuard>
#include <QShortcut>
#include <QTimer>
#include <QWidget>

#include <string>

MITK_TOOL_GUI_MACRO(MITKPYTHONSEGMENTATIONUI_EXPORT, QmitknnInteractiveToolGUI, "")

namespace
{
  constexpr auto LINE_HEIGHT_STYLE = "style='line-height: 1.25'";

  // Qt::Key_A..Qt::Key_Z are 0x41..0x5a and coincide with the ASCII codes
  // of the uppercase letters, so the same constant drives the QShortcut,
  // the tooltip hint, and the label suffix.
  constexpr Qt::Key RESET_KEY       = Qt::Key_R;
  constexpr Qt::Key UNDO_KEY        = Qt::Key_U;
  constexpr Qt::Key CONFIRM_KEY     = Qt::Key_C;
  constexpr Qt::Key PROMPT_TYPE_KEY = Qt::Key_T;
  constexpr Qt::Key POINT_KEY       = Qt::Key_P;
  constexpr Qt::Key BOX_KEY         = Qt::Key_B;
  constexpr Qt::Key SCRIBBLE_KEY    = Qt::Key_S;
  constexpr Qt::Key LASSO_KEY       = Qt::Key_L;

  QChar KeyChar(Qt::Key key)
  {
    return QChar(static_cast<int>(key));
  }

  QString LabelWithShortcut(const QString& baseText, Qt::Key key)
  {
    return QString("%1 (%2)").arg(baseText, KeyChar(key));
  }

  void BindShortcut(QWidget* parent, Qt::Key key, QPushButton* button,
                    const QString& tooltipTemplate)
  {
    button->setToolTip(tooltipTemplate.arg(KeyChar(key)));
    auto shortcut = new QShortcut(QKeySequence(key), parent);
    QObject::connect(shortcut, &QShortcut::activated, button, &QPushButton::click);
  }

  void SetIcon(QAbstractButton* button, const char* icon)
  {
    button->setIcon(QmitkStyleManager::ThemeIcon(QString(":/nnInteractive/%1").arg(icon)));
  }

  QCursor CreateCursor(const std::string& svg)
  {
    QPixmap pixmap;
    pixmap.loadFromData(QByteArray(svg.data(), svg.size()));
    return QCursor(pixmap, 0, 0);
  }

  QIcon ColorSwatchIcon(const mitk::Color& color)
  {
    // Keep the icon size fixed but draw a smaller centered square so the swatch
    // reads as a compact color chip rather than filling the whole icon area.
    constexpr int iconSize = 16;
    constexpr int swatchSize = 10;
    constexpr int offset = (iconSize - swatchSize) / 2;

    QPixmap pixmap(iconSize, iconSize);
    pixmap.fill(Qt::transparent);

    {
      QPainter painter(&pixmap);
      painter.fillRect(offset, offset, swatchSize, swatchSize, QColor(
        static_cast<int>(color.GetRed() * 255),
        static_cast<int>(color.GetGreen() * 255),
        static_cast<int>(color.GetBlue() * 255)));
    }

    return QIcon(pixmap);
  }

  mitk::TimeStepType GetCurrentTimeStep(const mitk::BaseData* data)
  {
    const auto* renderingManager = mitk::RenderingManager::GetInstance();
    const auto* timeNavigationController = renderingManager->GetTimeNavigationController();
    const auto timePoint = timeNavigationController->GetSelectedTimePoint();
    const auto geometry = data->GetTimeGeometry();

    return geometry->TimePointToTimeStep(timePoint);
  }

  // True for the preferences baked into a session at initialization (the single
  // source of truth lives in nnInteractiveTool). Changing any of them while a
  // session runs makes it stale, so the GUI ends the session (see
  // OnPreferenceChangedEvent).
  bool IsSessionDefiningPreference(const std::string& key)
  {
    for (const auto& entry : mitk::nnInteractiveTool::GetSessionDefiningPreferences())
    {
      if (entry.first == key)
        return true;
    }

    return false;
  }
}

QmitknnInteractiveToolGUI::QmitknnInteractiveToolGUI()
  : QmitkSegWithPreviewToolGUIBase(false, false),
    m_Ui(std::make_unique<Ui::QmitknnInteractiveToolGUI>()),
    m_PromptTypeButtonGroup(new QButtonGroup(this)),
    m_PromptType(PromptType::Positive)
{
  m_EnableConfirmSegBtnFnc = [this](bool enabled)
  {
    if (auto tool = this->GetTool(); tool != nullptr)
    {
      if (!tool->HasInteractions())
        return false;
    }

    return enabled;
  };
}

QmitknnInteractiveToolGUI::~QmitknnInteractiveToolGUI()
{
  this->UncheckOtherInteractorButtons(nullptr); // Ensure override cursor restoration.

  // Note: unused-auto-label cleanup is handled on the tool's DeactivatedEvent
  // (deferred via QTimer::singleShot), not here. Running RemoveLabel in the
  // destructor is unsafe because Qt widgets and itk::Object observers may be
  // in a partially-destructed state during application shutdown, which
  // causes crashes when segmentation events fan out to dead observers.

  if (auto* tool = this->GetTool())
  {
    tool->DeactivatedEvent -= mitk::MessageDelegate<QmitknnInteractiveToolGUI>(
      this, &QmitknnInteractiveToolGUI::OnToolDeactivated);

    tool->SessionEndedEvent -= mitk::MessageDelegate<QmitknnInteractiveToolGUI>(
      this, &QmitknnInteractiveToolGUI::OnSessionEnded);

    tool->SessionExpiredEvent -= mitk::MessageDelegate<QmitknnInteractiveToolGUI>(
      this, &QmitknnInteractiveToolGUI::OnSessionExpired);

    tool->PreviewUpdatedEvent -= mitk::MessageDelegate<QmitknnInteractiveToolGUI>(
      this, &QmitknnInteractiveToolGUI::OnPreviewUpdated);

    tool->ConfirmCleanUpEvent -= mitk::MessageDelegate1<QmitknnInteractiveToolGUI, bool>(
      this, &QmitknnInteractiveToolGUI::OnConfirmCleanUp);
  }

  if (m_Preferences != nullptr)
  {
    m_Preferences->OnPropertyChanged -=
      mitk::MessageDelegate1<QmitknnInteractiveToolGUI, const mitk::IPreferences::ChangeEvent&>(
        this, &QmitknnInteractiveToolGUI::OnPreferenceChangedEvent);
  }
}

void QmitknnInteractiveToolGUI::InitializeUI(QBoxLayout* mainLayout)
{
  auto wrapperWidget = new QWidget(this);
  mainLayout->addWidget(wrapperWidget);
  m_Ui->setupUi(wrapperWidget);

  SetIcon(m_Ui->settingsButton, "Gear");
  SetIcon(m_Ui->resetButton, "Reset");
  SetIcon(m_Ui->undoButton, "Undo");
  SetIcon(m_Ui->positiveButton, "Positive");
  SetIcon(m_Ui->negativeButton, "Negative");

  connect(m_Ui->initializeButton, &QPushButton::toggled, this, &Self::OnInitializeButtonToggled);
  connect(m_Ui->settingsButton, &QPushButton::clicked, this, &Self::OnSettingsButtonClicked);
  connect(m_Ui->resetButton, &QPushButton::clicked, this, &Self::OnResetInteractionsButtonClicked);
  connect(m_Ui->undoButton, &QPushButton::clicked, this, &Self::OnUndoButtonClicked);

  this->InitializePromptType();
  this->InitializeInteractorButtons();

  m_Ui->autoRefineCheckBox->setChecked(this->GetTool()->GetAutoRefine());
  connect(m_Ui->autoRefineCheckBox, &QCheckBox::toggled, this, &Self::OnAutoRefineCheckBoxToggled);

  m_Ui->autoZoomCheckBox->setChecked(this->GetTool()->GetAutoZoom());
  connect(m_Ui->autoZoomCheckBox, &QCheckBox::toggled, this, &Self::OnAutoZoomCheckBoxToggled);

  this->GetTool()->ConfirmCleanUpEvent += mitk::MessageDelegate1<QmitknnInteractiveToolGUI, bool>(
    this, &QmitknnInteractiveToolGUI::OnConfirmCleanUp);

  this->GetTool()->PreviewUpdatedEvent += mitk::MessageDelegate<QmitknnInteractiveToolGUI>(
    this, &QmitknnInteractiveToolGUI::OnPreviewUpdated);

  this->GetTool()->DeactivatedEvent += mitk::MessageDelegate<QmitknnInteractiveToolGUI>(
    this, &QmitknnInteractiveToolGUI::OnToolDeactivated);

  this->GetTool()->SessionEndedEvent += mitk::MessageDelegate<QmitknnInteractiveToolGUI>(
    this, &QmitknnInteractiveToolGUI::OnSessionEnded);

  this->GetTool()->SessionExpiredEvent += mitk::MessageDelegate<QmitknnInteractiveToolGUI>(
    this, &QmitknnInteractiveToolGUI::OnSessionExpired);

  // Drives the remote keep-alive heartbeat. Started on a successful remote
  // initialization (see OnInitializeButtonToggled) and stopped on session end
  // or expiry.
  m_HeartbeatTimer = new QTimer(this);
  connect(m_HeartbeatTimer, &QTimer::timeout, this, &Self::OnHeartbeatTimeout);

  this->UpdateModelLicenseDisplay(std::nullopt);

  Superclass::InitializeUI(mainLayout);

  // TODO: Once we agree on a common shortcut concept, the confirm binding
  // should be moved to the base class.

  auto confirmButton = this->GetConfirmSegmentationButton();

  BindShortcut(this, RESET_KEY, m_Ui->resetButton, "Press %1 to reset all interactions");
  BindShortcut(this, UNDO_KEY, m_Ui->undoButton, "Press %1 to undo the last interaction");
  BindShortcut(this, CONFIRM_KEY, confirmButton, "Press %1 to confirm a segmentation");

  // Cache the base label of each shortcut-bound widget as seen from the
  // .ui file. The cache is the single source of truth for ApplyShortcutLabels,
  // so repeated invocations never accumulate suffixes. The confirm button is
  // handled separately by UpdateConfirmButtonLabel, whose text also reflects
  // the active target label.

  m_ShortcutLabels = {
    { m_Ui->resetButton,    RESET_KEY,    m_Ui->resetButton->text() },
    { m_Ui->undoButton,     UNDO_KEY,     m_Ui->undoButton->text() },
    { m_Ui->pointButton,    POINT_KEY,    m_Ui->pointButton->text() },
    { m_Ui->boxButton,      BOX_KEY,      m_Ui->boxButton->text() },
    { m_Ui->scribbleButton, SCRIBBLE_KEY, m_Ui->scribbleButton->text() },
    { m_Ui->lassoButton,    LASSO_KEY,    m_Ui->lassoButton->text() },
  };
  m_PromptTypeBaseTitle = m_Ui->promptTypeGroupBox->title();

  auto prefService = mitk::CoreServices::GetPreferencesService();
  m_Preferences = prefService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  m_Preferences->OnPropertyChanged +=
    mitk::MessageDelegate1<QmitknnInteractiveToolGUI, const mitk::IPreferences::ChangeEvent&>(
      this, &QmitknnInteractiveToolGUI::OnPreferenceChangedEvent);

  this->ApplyShortcutLabels();
  this->UpdateConfirmButtonLabel();
  this->UpdateInitializeButtonText();
}

void QmitknnInteractiveToolGUI::EnableInitializeButtons(bool enabled)
{
  m_Ui->initializeButton->setEnabled(enabled);
  m_Ui->settingsButton->setEnabled(enabled);
}

void QmitknnInteractiveToolGUI::OnAutoRefineCheckBoxToggled(bool checked)
{
  this->GetTool()->SetAutoRefine(checked);
}

void QmitknnInteractiveToolGUI::OnAutoZoomCheckBoxToggled(bool checked)
{
  this->GetTool()->SetAutoZoom(checked);
}

void QmitknnInteractiveToolGUI::InitializePromptType()
{
  m_PromptType = m_Ui->positiveButton->isChecked()
    ? PromptType::Positive
    : PromptType::Negative;

  m_PromptTypeButtonGroup->addButton(m_Ui->positiveButton);
  m_PromptTypeButtonGroup->setId(m_Ui->positiveButton, static_cast<int>(PromptType::Positive));

  m_PromptTypeButtonGroup->addButton(m_Ui->negativeButton);
  m_PromptTypeButtonGroup->setId(m_Ui->negativeButton, static_cast<int>(PromptType::Negative));

  connect(m_PromptTypeButtonGroup, &QButtonGroup::idClicked, [this](int id) {
    if (id != static_cast<int>(m_PromptType))
    {
      m_PromptType = static_cast<PromptType>(id);
      this->OnPromptTypeChanged();
    }
  });

  // Set shortcut to toggle the prompt type.

  const QString toolTip = QString("Press %1 to switch the prompt types").arg(KeyChar(PROMPT_TYPE_KEY));
  m_Ui->positiveButton->setToolTip(toolTip);
  m_Ui->negativeButton->setToolTip(toolTip);

  auto togglePromptType = new QShortcut(QKeySequence(PROMPT_TYPE_KEY), this);

  connect(togglePromptType, &QShortcut::activated, this, [this]() {
    if (m_Ui->positiveButton->isChecked())
    {
      m_PromptTypeButtonGroup->button(static_cast<int>(PromptType::Negative))->click();
    }
    else
    {
      m_PromptTypeButtonGroup->button(static_cast<int>(PromptType::Positive))->click();
    }
  });
}

void QmitknnInteractiveToolGUI::InitializeInteractorButtons()
{
  // Set shortcuts to toggle interactor buttons.

  BindShortcut(this, POINT_KEY,    m_Ui->pointButton,    "Press %1 to toggle the point interaction");
  BindShortcut(this, BOX_KEY,      m_Ui->boxButton,      "Press %1 to toggle the box interaction");
  BindShortcut(this, SCRIBBLE_KEY, m_Ui->scribbleButton, "Press %1 to toggle the scribble interaction");
  BindShortcut(this, LASSO_KEY,    m_Ui->lassoButton,    "Press %1 to toggle the lasso interaction");

  m_InteractorButtons[InteractionType::Point] = m_Ui->pointButton;
  m_InteractorButtons[InteractionType::Box] = m_Ui->boxButton;
  m_InteractorButtons[InteractionType::Scribble] = m_Ui->scribbleButton;
  m_InteractorButtons[InteractionType::Lasso] = m_Ui->lassoButton;

  for (const auto& [interactionType, button] : m_InteractorButtons)
  {
    auto interactor = this->GetTool()->GetInteractor(interactionType);
    auto icon = interactor->GetIcon();

    if (!icon.empty())
      button->setIcon(QmitkStyleManager::ThemeIcon(QByteArray(icon.data(), icon.size())));

    connect(button, &QPushButton::toggled, [this, interactionType](bool checked) {
      this->OnInteractorToggled(interactionType, checked);
    });
  }
}

bool QmitknnInteractiveToolGUI::Install()
{
  const auto venvName = this->GetTool()->GetVirtualEnvName();

  // If the venv already exists with nnInteractive installed, skip the install
  // dialog: reconcile the recorded install mode, run the version check, and offer
  // an in-place update if one is needed.
  if (mitk::PythonHelper::VirtualEnvExists(venvName))
  {
    if (!this->GetTool()->CreatePythonContext())
      return false;

    if (this->GetTool()->IsInstalled())
    {
      // A context exists here, so the find_spec probe is cheap. Reconcile the
      // recorded install mode with what is actually installed (e.g. a venv built
      // by a different MITK version) so the GUI and preferences adapt correctly.
      const bool localAvailable = this->GetTool()->IsLocalInferenceAvailable();
      const std::string installMode = localAvailable ? "full" : "client";

      if (m_Preferences->Get("nnInteractive/installMode", "full") != installMode)
        m_Preferences->Put("nnInteractive/installMode", installMode);

      // Client-only can only run remote sessions; keep the inference mode consistent.
      if (!localAvailable && m_Preferences->Get("nnInteractive/inferenceMode", "local") != "remote")
        m_Preferences->Put("nnInteractive/inferenceMode", "remote");

      const std::string distributionName = localAvailable ? "nnInteractive" : "nninteractive-client";

      // A reused virtual environment can hold an nnInteractive that predates this
      // MITK build (the venv survives MITK upgrades). The offline minimum check
      // runs on every initialize; the online "newer release available" check runs
      // at most once per initialized session so an offline user never waits on the
      // PyPI timeout repeatedly. The guard is reset on session teardown (see
      // OnSessionEnded), so a later reinitialize checks again.
      const bool checkForUpdate = !m_OnlineUpdateCheckDone;
      const auto versionCheck = mitk::nnInteractive::CheckInstalledVersion(
        *this->GetTool()->GetPythonContext(), checkForUpdate, distributionName);

      // A non-empty Latest means PyPI was actually reached; an empty one means the
      // query was skipped or the network was unreachable. Only mark the check done
      // when it really ran, so an offline failure retries on the next initialize.
      if (checkForUpdate && !versionCheck.Latest.empty())
        m_OnlineUpdateCheckDone = true;

      if (versionCheck.Status == mitk::nnInteractive::VersionStatus::BelowMinimum)
      {
        if (!this->OfferInPlaceUpdate(versionCheck, !localAvailable, true))
          return false;
      }
      else if (versionCheck.Status == mitk::nnInteractive::VersionStatus::UpdateAvailable)
      {
        if (!this->OfferInPlaceUpdate(versionCheck, !localAvailable, false))
          return false;
      }

      // Offer to adopt a newer recommended model checkpoint (full + local only).
      this->MaybePromptModelSwitch(localAvailable);

      return true;
    }
  }

  // Fresh install: let the user choose between a full and a client-only install.
  QmitknnInteractiveInstallModeDialog modeDialog(this);

  if (modeDialog.exec() != QDialog::Accepted)
    return false;

  const bool clientOnly =
    modeDialog.SelectedMode() == QmitknnInteractiveInstallModeDialog::Mode::ClientOnly;

  auto spec = mitk::nnInteractive::BuildInstallSpec(m_Preferences, venvName, clientOnly);

  QmitkPipInstallDialog dialog(spec, this);

  if (dialog.exec() != QDialog::Accepted)
    return false;

  // Record the chosen mode so the GUI and preferences adapt; a client-only install
  // can only run remote sessions, so force the inference mode accordingly.
  m_Preferences->Put("nnInteractive/installMode", clientOnly ? "client" : "full");

  if (clientOnly)
  {
    m_Preferences->Put("nnInteractive/inferenceMode", "remote");

    // Client-only runs inference on a remote server. If none is configured yet
    // (the common case right after a first install), guide the user to set one now
    // rather than letting StartSession() fail with a bare "no server URL" error.
    if (m_Preferences->Get("nnInteractive/serverUrl", "").empty())
    {
      QMessageBox::information(nullptr, "nnInteractive",
        QString(
          "<h3 %1>Configure a server</h3>"
          "<p %1>Client-only mode runs nnInteractive on a remote server. Set the "
          "server URL (and an API key, if the server requires one) to continue.</p>")
          .arg(LINE_HEIGHT_STYLE));

      // Opens the nnInteractive preference page modally; the user can set the
      // server URL here.
      this->OnSettingsButtonClicked();

      // Still unset: stop before starting a session that would only fail. The user
      // can configure the server and click Initialize again.
      if (m_Preferences->Get("nnInteractive/serverUrl", "").empty())
        return false;
    }
  }

  // The dialog populated the venv (and possibly created it). Create a fresh
  // context so the embedded interpreter picks up the newly installed packages.
  // PythonContext checks Py_IsInitialized internally, so calling this a second
  // time after the early-return path above is safe.
  return this->GetTool()->CreatePythonContext();
}

bool QmitknnInteractiveToolGUI::RunUpdate(bool clientOnly)
{
  const auto venvName = this->GetTool()->GetVirtualEnvName();
  auto spec = mitk::nnInteractive::BuildUpgradeSpec(venvName, clientOnly);

  QmitkPipInstallDialog dialog(spec, this, QmitkPipInstallDialog::Mode::Update);

  if (dialog.exec() != QDialog::Accepted)
    return false;

  // Recreate the context so the interpreter sees the upgraded packages. Safe
  // because an in-place update is only reached when no nnInteractive modules are
  // loaded (see OfferInPlaceUpdate).
  return this->GetTool()->CreatePythonContext();
}

bool QmitknnInteractiveToolGUI::OfferInPlaceUpdate(const mitk::nnInteractive::VersionCheckResult& versionCheck, bool clientOnly, bool belowMinimum)
{
  const auto venvName = this->GetTool()->GetVirtualEnvName();
  const bool modulesLoaded = mitk::PythonHelper::IsAnyVirtualEnvModuleLoaded(venvName);

  const auto appName = QCoreApplication::applicationName();
  const auto restartTarget = appName.isEmpty() ? QStringLiteral("the application") : appName;
  const auto installed = QString::fromStdString(versionCheck.Installed);

  if (belowMinimum)
  {
    const auto minimum = QString(mitk::nnInteractive::MINIMUM_VERSION);

    if (modulesLoaded)
    {
      QMessageBox::warning(nullptr, "nnInteractive",
        QString(
          "<h3 %1>nnInteractive is outdated</h3>"
          "<p %1>The installed nnInteractive %2 is older than the version this "
          "application requires (%3 or newer) and may not work correctly.</p>"
          "<p %1>nnInteractive is currently loaded, so it cannot be updated right now. "
          "Restart %4 and initialize again to update.</p>")
          .arg(LINE_HEIGHT_STYLE).arg(installed).arg(minimum).arg(restartTarget));
      return false;
    }

    QMessageBox messageBox(QMessageBox::Warning, "nnInteractive",
      QString(
        "<h3 %1>nnInteractive is outdated</h3>"
        "<p %1>The installed nnInteractive %2 is older than the version this "
        "application requires (%3 or newer) and may not work correctly.</p>"
        "<p %1>Click <em>Update now</em> to update to a compatible version.</p>")
        .arg(LINE_HEIGHT_STYLE).arg(installed).arg(minimum));
    auto* updateButton = messageBox.addButton("Update now", QMessageBox::AcceptRole);
    messageBox.addButton(QMessageBox::Cancel);
    messageBox.setDefaultButton(updateButton);
    messageBox.exec();

    if (messageBox.clickedButton() != updateButton)
      return false;

    return this->RunUpdate(clientOnly);
  }

  // UpdateAvailable: the installed version still works, so updating is optional.
  const auto latest = QString::fromStdString(versionCheck.Latest);

  if (modulesLoaded)
  {
    QMessageBox messageBox(QMessageBox::Information, "nnInteractive",
      QString(
        "<h3 %1>A newer nnInteractive is available</h3>"
        "<p %1>nnInteractive %2 is installed; %3 is available.</p>"
        "<p %1>nnInteractive is currently loaded, so it cannot be updated right now. "
        "Restart %4 and initialize again to update, or click <em>Continue</em> to keep "
        "using the installed version.</p>")
        .arg(LINE_HEIGHT_STYLE).arg(installed).arg(latest).arg(restartTarget));
    auto* continueButton = messageBox.addButton("Continue", QMessageBox::AcceptRole);
    messageBox.addButton(QMessageBox::Cancel);
    messageBox.setDefaultButton(continueButton);
    // An optional update that can't be applied right now: dismissing (Esc or the
    // window close button) should continue with the working installed version, not
    // abort initialization.
    messageBox.setEscapeButton(continueButton);
    messageBox.exec();

    return messageBox.clickedButton() == continueButton;
  }

  QMessageBox messageBox(QMessageBox::Information, "nnInteractive",
    QString(
      "<h3 %1>A newer nnInteractive is available</h3>"
      "<p %1>nnInteractive %2 is installed; %3 is available.</p>"
      "<p %1>Click <em>Update now</em> to update, or <em>Continue with installed</em> "
      "to keep using %2.</p>")
      .arg(LINE_HEIGHT_STYLE).arg(installed).arg(latest));
  auto* updateButton = messageBox.addButton("Update now", QMessageBox::AcceptRole);
  auto* continueButton = messageBox.addButton("Continue with installed", QMessageBox::AcceptRole);
  messageBox.addButton(QMessageBox::Cancel);
  messageBox.setDefaultButton(continueButton);
  // Optional update: dismissing (Esc or the window close button) continues with
  // the installed version rather than aborting initialization.
  messageBox.setEscapeButton(continueButton);
  messageBox.exec();

  if (messageBox.clickedButton() == updateButton)
    return this->RunUpdate(clientOnly);

  return messageBox.clickedButton() == continueButton;
}

void QmitknnInteractiveToolGUI::MaybePromptModelSwitch(bool localAvailable)
{
  if (!localAvailable || m_Preferences == nullptr)
    return;

  // Only relevant for local inference with the managed (Hugging Face) model source;
  // a remote server and a local checkpoint folder both pick their own model.
  if (m_Preferences->Get("nnInteractive/inferenceMode", "local") == "remote")
    return;

  if (m_Preferences->Get("nnInteractive/modelSource", "huggingface") == "local")
    return;

  // The manifest refresh hits the network, so check at most once per initialized
  // session; the guard is reset on session teardown (see OnSessionEnded) so a
  // later reinitialize checks again.
  if (m_ModelSwitchCheckDone)
    return;

  const auto selectedId = m_Preferences->Get("nnInteractive/modelCheckpoint", "");

  // CheckModelUpdate spins up a subprocess and refreshes the manifest over the
  // network; it drives that behind a modal progress dialog (parented to this
  // GUI) so the Workbench stays responsive and the user can cancel.
  const auto check = mitk::nnInteractive::CheckModelUpdate(this->GetTool()->GetVirtualEnvName(), selectedId, this);

  // Unknown means the manifest could not be refreshed (offline, no model
  // management). Leave the guard unset so the check retries next time rather than
  // marking a network failure as done.
  if (check.Status == mitk::nnInteractive::ModelUpdateStatus::Unknown)
    return;

  m_ModelSwitchCheckDone = true;

  if (check.Status != mitk::nnInteractive::ModelUpdateStatus::UpdateAvailable)
    return;

  const auto current = check.CurrentId.empty()
    ? QStringLiteral("the default")
    : QString::fromStdString(check.CurrentId);

  QMessageBox messageBox(QMessageBox::Information, "nnInteractive",
    QString(
      "<h3 %1>A newer model checkpoint is available</h3>"
      "<p %1>nnInteractive now recommends the model <em>%2</em>; you are using <em>%3</em>.</p>"
      "<p %1>Switch to the recommended model? It will be downloaded during "
      "initialization if it is not already available.</p>")
      .arg(LINE_HEIGHT_STYLE)
      .arg(QString::fromStdString(check.RecommendedId))
      .arg(current));
  auto* switchButton = messageBox.addButton("Switch", QMessageBox::AcceptRole);
  messageBox.addButton("Keep current", QMessageBox::RejectRole);
  messageBox.setDefaultButton(switchButton);
  messageBox.exec();

  if (messageBox.clickedButton() == switchButton)
  {
    // No session is running yet (this runs in Install(), before StartSession), so
    // the session-defining-preference observer is a harmless no-op here.
    m_Preferences->Put("nnInteractive/modelCheckpoint", check.RecommendedId);
  }
}

void QmitknnInteractiveToolGUI::OnInitializeButtonToggled(bool checked)
{
  if (!checked)
  {
    // The button is a toggle: unchecking it uninitializes. AbortSession() ends the
    // session and also clears the interaction prompts and preview and refreshes the
    // views (a bare EndSession() would leave them on screen), then fires
    // SessionEndedEvent -> OnSessionEnded(), which reverts the session-dependent
    // controls and the button label. No confirmation prompt, matching Reset and
    // tool deactivation, which also discard unconfirmed work silently.
    this->GetTool()->AbortSession();
    return;
  }

  this->EnableInitializeButtons(false);

  if (!Install())
  {
    this->EnableInitializeButtons(true);
    this->UncheckInitializeButton();
    return;
  }

#if defined(__APPLE__) && !defined(__aarch64__)
  // Intel Macs have no local PyTorch/GPU support, but a client-only or remote
  // session offloads inference to a server, so only block when a local session
  // would actually run. This runs after Install(), which is where the install
  // mode is chosen (fresh install) or reconciled with what is actually present
  // (reused venv); only then do the preferences reliably reflect the real mode.
  {
    const bool clientOnly = m_Preferences->Get("nnInteractive/installMode", "full") == "client";
    const bool remote = clientOnly || m_Preferences->Get("nnInteractive/inferenceMode", "local") == "remote";

    if (!remote)
    {
      QMessageBox::information(
        nullptr,
        "nnInteractive",
        QString(
          "<h3 %1>Unsupported platform</h3>"
          "<p %1>Local nnInteractive inference requires an Apple Silicon Mac and is "
          "not available on Intel-based Macs.</p>"
          "<p %1>You can still use a remote nnInteractive server: select "
          "<em>Remote server</em> in the nnInteractive preferences under "
          "<em>Inference</em>.</p>")
          .arg(LINE_HEIGHT_STYLE),
        QMessageBox::Ok);

      // No session was started; release the toggle without re-entering this slot.
      this->EnableInitializeButtons(true);
      this->UncheckInitializeButton();
      return;
    }
  }
#endif

  const auto initMessage = QString(
    "<h3 %1>Initializing nnInteractive</h3>"
    "<p %1>Please wait while nnInteractive is initialized. If a model checkpoint "
    "still needs to be downloaded, this can take a few minutes.</p>").arg(LINE_HEIGHT_STYLE);
 
  auto messageBox = new QMessageBox(QMessageBox::Information, "nnInteractive", initMessage);
  messageBox->setStandardButtons(QMessageBox::NoButton);
  messageBox->setAttribute(Qt::WA_DeleteOnClose);
  messageBox->show();
  qApp->processEvents();

  QTimer::singleShot(100, this, [=, this]() {
    try
    {
      this->GetTool()->StartSession();
    }
    catch (const mitk::Exception& e)
    {
      messageBox->accept();

      const QString description = QString::fromLocal8Bit(e.GetDescription());

      // Errors thrown directly from C++ (a mapped remote connection failure or
      // a missing configuration) carry a clean, user-facing message and are
      // shown as-is. Errors bubbling up from the embedded Python interpreter
      // carry a traceback, so we keep a generic headline and tuck the traceback
      // into the (collapsed) details.
      const bool isPythonError = description.contains("An error occurred while executing Python code:");

      const QString headline = isPythonError
        ? QStringLiteral("nnInteractive reported an error during initialization (see details).")
        : description;

      MITK_ERROR << "nnInteractive initialization failed:\n" << e.GetDescription();

      // Escape the headline: for a non-Python error it is the raw exception
      // description, which can embed the server URL or other characters that
      // would otherwise be interpreted as HTML by the message box.
      auto errorMsgBox = new QMessageBox(QMessageBox::Critical, nullptr,
        QString("<p %1>%2</p>").arg(LINE_HEIGHT_STYLE).arg(headline.toHtmlEscaped()));

      if (isPythonError)
        errorMsgBox->setDetailedText(description);

      errorMsgBox->setTextInteractionFlags(Qt::TextSelectableByMouse);
      errorMsgBox->setAttribute(Qt::WA_DeleteOnClose, true);
      errorMsgBox->setModal(true);
      errorMsgBox->exec();

      this->EnableInitializeButtons(true);
      this->UncheckInitializeButton();
      return;
    }

    messageBox->accept();

    // Keep the Initialize button enabled so a second click uninitializes the
    // session; its label flips to "Uninitialize" while a session is running
    // (see UpdateInitializeButtonText). Re-enable the settings button too so the
    // user can adjust preferences that apply mid-session (e.g. interaction mode).
    m_Ui->initializeButton->setEnabled(true);
    this->UpdateInitializeButtonText();
    m_Ui->settingsButton->setEnabled(true);

    m_Ui->resetButton->setEnabled(true);
    m_Ui->promptTypeGroupBox->setEnabled(true);
    m_Ui->interactionToolsGroupBox->setEnabled(true);

    this->ApplyCapabilityGating();

    // Start the remote keep-alive heartbeat (see nnInteractiveTool::Heartbeat).
    // A zero interval means none is needed: a local session, or a server with
    // the liveness timeout disabled.
    const int heartbeatIntervalMs = this->GetTool()->GetHeartbeatIntervalMs();
    if (heartbeatIntervalMs > 0)
      m_HeartbeatTimer->start(heartbeatIntervalMs);

    // Surface the model's license terms now that a session is bound.
    this->UpdateModelLicenseDisplay(this->GetTool()->GetModelLicense());

    // While the session runs, selecting a non-empty label in the host's label
    // inspector seeds the session from that label's mask. Connect once
    // (Qt::UniqueConnection prevents repeated initializations from stacking
    // duplicate connections); Qt drops it when this GUI is destroyed on tool
    // deactivation. The inspector is null when hosted outside a segmentation view.
    if (auto* inspector = this->GetMultiLabelInspector(); inspector != nullptr)
    {
      connect(inspector, &QmitkMultiLabelInspector::CurrentSelectionChanged,
              this, &Self::OnActiveLabelChanged, Qt::UniqueConnection);
    }

    // Seed from the already-selected label at init time. Route it through the
    // same handler as a running-session selection change so both entry points
    // resolve and apply the label identically (set active label, seed the mask,
    // refresh the Confirm button). No inspector signal is pending here, so the
    // segmentation's active label is already authoritative.
    if (const auto* segmentation = this->GetTool()->GetTargetSegmentation(); segmentation != nullptr)
    {
      if (const auto* activeLabel = segmentation->GetActiveLabel(); activeLabel != nullptr)
        this->OnActiveLabelChanged({ activeLabel->GetValue() });
    }

    auto backend = this->GetTool()->GetBackend();

    if (!backend.has_value())
      return;

    if (backend == mitk::nnInteractive::Backend::CUDA)
      return;

    m_Ui->autoZoomCheckBox->setEnabled(false);
    m_Ui->autoZoomCheckBox->setChecked(false);
    m_Ui->autoZoomCheckBox->setToolTip("Auto-zoom is not available with CPU backend.");

  #if defined(__APPLE__)
    // On macOS nnInteractive has no GPU acceleration and always runs on the CPU,
    // so the NVIDIA-specific guidance in the non-Apple branch does not apply.
    // Point users at remote mode, which offloads inference to a server GPU.
    const QString macCpuBackendMessage = QString(
      "<h3 %1>Running on CPU</h3>"
      "<p %1>nnInteractive has no GPU acceleration on macOS and runs on the CPU, "
      "which is <em>significantly slower</em>.</p>"
      "<p %1>For fast response times, run inference on a remote nnInteractive server "
      "with a GPU. Enable it in the nnInteractive preferences under <em>Inference</em> "
      "by selecting <em>Remote server</em>.</p>")
      .arg(LINE_HEIGHT_STYLE);

    QMessageBox::warning(nullptr, "nnInteractive", macCpuBackendMessage);
  #else
    const QString cpuBackendMessage = QString(
      "<h3 %1>No compatible CUDA device detected</h3>"
      "<p %1>Falling back to CPU processing, which is <em>significantly slower</em>.</p>"
      "<p %1>For smooth performance and fast response times, a compatible NVIDIA GPU with at "
      "least 6 GB VRAM is required:</p>"
      "<ul %1>"
        "<li %1>Minimum: Pascal architecture (e.g., GeForce GTX 1060)</li>"
        "<li %1>Better: Turing architecture (e.g., GeForce RTX 2070)</li>"
        "<li %1>Best: Ampere or newer (e.g., GeForce RTX 3080)</li>"
      "</ul>"
      "<p %1>6 GB VRAM is the absolute minimum; 12 GB or more is recommended for optimal results.</p>"
      "<p %1>Alternatively, you can run inference on a remote nnInteractive server with a GPU. "
      "Enable it in the nnInteractive preferences under <em>Inference</em> by selecting "
      "<em>Remote server</em>.</p>")
      .arg(LINE_HEIGHT_STYLE);

    QMessageBox::warning(nullptr, "nnInteractive", cpuBackendMessage);
  #endif
  });
}

void QmitknnInteractiveToolGUI::OnSettingsButtonClicked()
{
  mitk::CoreServices::GetPreferencesService()->OpenPreferencesDialog("org.mitk.gui.qt.application.nnInteractionPreferencePage");
}

void QmitknnInteractiveToolGUI::OnResetInteractionsButtonClicked()
{
  // An explicit reset invalidates any pending auto-created-label tracking
  // so the empty label survives (the user chose to reset, not confirm).
  this->InvalidateAutoCreatedLabel();

  // Uncheck any interactor button.
  for (const auto& [interactor, button] : m_InteractorButtons)
    button->setChecked(false);

  // Reset any interactions.
  this->GetTool()->ResetInteractions();

  // Nothing is undoable after a reset.
  this->UpdateUndoButtonState();

  // Switch to positive prompt type.
  m_Ui->positiveButton->click();
}

void QmitknnInteractiveToolGUI::OnUndoButtonClicked()
{
  auto* tool = this->GetTool();

  if (tool == nullptr)
    return;

  tool->UndoLastInteraction();
  this->UpdateUndoButtonState();
}

void QmitknnInteractiveToolGUI::UpdateUndoButtonState()
{
  auto* tool = this->GetTool();
  m_Ui->undoButton->setEnabled(m_SupportsUndo && tool != nullptr && tool->CanUndo());
}

void QmitknnInteractiveToolGUI::UpdateConfirmButtonLabel()
{
  auto* confirmButton = this->GetConfirmSegmentationButton();

  if (confirmButton == nullptr)
    return;

  QString text = "Confirm Segmentation";
  QIcon icon;

  // While a session runs, name the target label the next Confirm writes into
  // and show its color as a swatch.
  auto* tool = this->GetTool();

  if (tool != nullptr && tool->IsSessionRunning())
  {
    if (auto* segmentation = tool->GetTargetSegmentation(); segmentation != nullptr)
    {
      if (const auto* activeLabel = segmentation->GetActiveLabel(); activeLabel != nullptr)
      {
        text = QString("Confirm %1").arg(QString::fromStdString(activeLabel->GetName()));
        icon = ColorSwatchIcon(activeLabel->GetColor());
      }
    }
  }

  if (this->AreShortcutsShownInLabels())
    text = LabelWithShortcut(text, CONFIRM_KEY);

  confirmButton->setIcon(icon);
  confirmButton->setText(text);
}

void QmitknnInteractiveToolGUI::OnPromptTypeChanged()
{
  for (const auto& [interactionType, interactor] : this->GetTool()->GetInteractors())
  {
    if (m_InteractorButtons[interactionType]->isChecked())
    {
      // If any interactor is already enabled, reenable it with the new prompt type.
      this->GetTool()->EnableInteractor(interactionType, m_PromptType);

      // Update the interactor cursor to reflect the changed prompt type.
      auto svg = this->GetTool()->GetInteractor(interactionType)->GetCursor(m_PromptType);
      QApplication::changeOverrideCursor(CreateCursor(svg));
      break;
    }
  }
}

void QmitknnInteractiveToolGUI::OnInteractorToggled(InteractionType interactionType, bool checked)
{
  if (checked)
  {
    // Ensure that only a single interactor is enabled at any time.
    this->UncheckOtherInteractorButtons(m_InteractorButtons[interactionType]);
    this->GetTool()->EnableInteractor(interactionType, m_PromptType);

    // Remember this button so automation can re-enable it after an auto-confirm.
    m_LastInteractorButton = m_InteractorButtons[interactionType];

    // Set the cursor to the interactor's cursor.
    auto svg = this->GetTool()->GetInteractor(interactionType)->GetCursor(m_PromptType);
    QApplication::setOverrideCursor(CreateCursor(svg));
  }
  else
  {
    // Disable the currently enabled interactor and restore the cursor.
    QApplication::restoreOverrideCursor();
    this->GetTool()->DisableInteractor(interactionType);
  }
}

void QmitknnInteractiveToolGUI::UncheckOtherInteractorButtons(QPushButton* interactorButton)
{
  for (const auto& [interactor, button] : m_InteractorButtons)
  {
    if (button != interactorButton)
      button->setChecked(false);
  }
}

mitk::nnInteractiveTool* QmitknnInteractiveToolGUI::GetTool()
{
  return this->GetConnectedToolAs<mitk::nnInteractiveTool>();
}

void QmitknnInteractiveToolGUI::OnActiveLabelChanged(const mitk::MultiLabelSegmentation::LabelValueVectorType& labels)
{
  // AutoCreateAndSelectNewLabel() drives the inspector's selection itself and
  // sets up the new-label state directly; ignore the selection signals it emits
  // so they do not reset or re-seed the just-confirmed session mid-confirm.
  if (m_SuppressActiveLabelChanged)
    return;

  // Only a single-label selection drives the active target label. Use the value
  // from the signal payload: the segmentation's active label is not guaranteed
  // to be updated yet when this slot runs (the host view sets it later in the
  // CurrentSelectionChanged chain).
  if (labels.size() != 1)
    return;

  auto* tool = this->GetTool();

  if (tool == nullptr)
    return;

  // Make the payload label the single source of truth up front, so every later
  // read of the active label (the tool's preview color in DoUpdatePreview, the
  // Confirm button below, and the mask seeding) resolves the new label rather
  // than the stale one. SetActiveLabel only marks the segmentation modified (no
  // CurrentSelectionChanged is re-emitted), so this does not re-enter this slot.
  if (auto* segmentation = tool->GetTargetSegmentation(); segmentation != nullptr)
  {
    const auto* activeLabel = segmentation->GetActiveLabel();
    if (activeLabel == nullptr || activeLabel->GetValue() != labels.front())
      segmentation->SetActiveLabel(labels.front());
  }

  this->MaybeInitializeWithLabelMask(labels.front());
  this->UpdateConfirmButtonLabel();
}

void QmitknnInteractiveToolGUI::MaybeInitializeWithLabelMask(mitk::MultiLabelSegmentation::LabelValueType labelValue)
{
  auto* tool = this->GetTool();

  // Rebasing needs a live session, and the model must advertise the mask
  // interaction. Outside a running session a selection change is a no-op.
  if (tool == nullptr || !tool->IsSessionRunning() || !tool->GetSupportedInteractions().Mask)
    return;

  auto* segmentation = tool->GetTargetSegmentation();

  if (segmentation == nullptr)
    return;

  auto label = segmentation->GetLabel(labelValue);

  if (label.IsNull())
    return;

  // Switching the target label discards the previous label's interactions and
  // preview, so the new label starts from a clean slate; in particular a mask
  // seeded from the previous label must not linger when switching to an empty
  // one. No confirmation: the selection has already changed, so the next
  // Confirm would write into the new label anyway.
  this->OnResetInteractionsButtonClicked();

  // Seed from the label's existing content; an empty label just starts fresh.
  if (!segmentation->IsEmpty(label, GetCurrentTimeStep(segmentation)))
    tool->InitializeSessionWithMask(mitk::CreateLabelMask(segmentation, labelValue));

  // The mask initialization is one undoable step (no PreviewUpdatedEvent is
  // emitted for it), so refresh the button state here.
  this->UpdateUndoButtonState();
}

void QmitknnInteractiveToolGUI::OnConfirmCleanUp(bool isConfirmed)
{
  if (!isConfirmed)
    return;

  // Read flags fresh so a mid-session preference change takes effect
  // on the next Confirm without requiring tool reactivation.
  const bool autoCreate = this->IsAutoCreateNextLabelEnabled();
  const bool autoConfirm = this->IsAutoConfirmEnabled();

  this->OnResetInteractionsButtonClicked();

  bool createdNewLabel = false;
  if (autoCreate)
    createdNewLabel = this->AutoCreateAndSelectNewLabel();

  // Re-enable the last interactor so either automation can continue without
  // the user having to re-select Point/Box/Scribble/Lasso after every Confirm.
  // Skip this if auto-create was attempted but failed (e.g. user canceled the
  // rename dialog), so the next interaction doesn't extend the just-confirmed
  // label instead of starting a fresh one.
  if (autoConfirm || createdNewLabel)
    this->ReEnableLastInteractor();

  // The active label may have changed (auto-create-next-label); reflect it.
  this->UpdateConfirmButtonLabel();
}

void QmitknnInteractiveToolGUI::OnToolDeactivated()
{
  if (!m_AutoCreatedLabelValue.has_value())
    return;

  // Capture state locally and clear our tracking. Lock() returns a
  // SmartPointer that keeps the segmentation alive for the deferred call.
  auto segmentationPtr = m_AutoCreatedLabelSegmentation.Lock();
  const auto value = *m_AutoCreatedLabelValue;
  const auto previousActive = m_PreviousActiveLabelValue;
  auto* inspector = this->GetMultiLabelInspector();
  this->InvalidateAutoCreatedLabel();

  if (segmentationPtr.IsNull())
    return;

  // Defer the destructive part to the next event-loop tick.
  //
  // - Normal user-initiated tool switch: the Qt event loop is alive, so the
  //   lambda fires a tick later, after any synchronous BlueBerry chatter
  //   from the tool-switch path has settled.
  //
  // - Workbench close: BlueBerry's Workbench::Close() runs synchronously and
  //   tears down views/widgets in a sequence that leaves some segmentation
  //   observers (renderer mappers, etc.) dangling. When it eventually quits
  //   the Qt event loop, pending single-shot timers are discarded, so our
  //   lambda never fires and RemoveLabel is never called. Harmless: the
  //   unused label vanishes with the application anyway.
  QTimer::singleShot(0, qApp, [segmentationPtr, value, previousActive, inspector]() {
    if (QCoreApplication::closingDown())
      return;

    auto* segmentation = segmentationPtr.GetPointer();

    if (segmentation == nullptr || !segmentation->ExistLabel(value))
      return;

    if (!segmentation->IsEmpty(value, 0))
      return;

    const auto* activeLabel = segmentation->GetActiveLabel();
    if (activeLabel != nullptr && activeLabel->GetValue() == value)
    {
      mitk::MultiLabelSegmentation::LabelValueType fallback = 0;
      bool haveFallback = false;

      if (previousActive.has_value() && *previousActive != value && segmentation->ExistLabel(*previousActive))
      {
        fallback = *previousActive;
        haveFallback = true;
      }
      else
      {
        for (const auto& label : segmentation->GetLabels())
        {
          if (label.IsNotNull() && label->GetValue() != value)
          {
            fallback = label->GetValue();
            haveFallback = true;
            break;
          }
        }
      }

      if (haveFallback)
      {
        segmentation->SetActiveLabel(fallback);

        if (inspector != nullptr)
          inspector->SetSelectedLabel(fallback);
      }
    }

    segmentation->RemoveLabel(value);
  });
}

void QmitknnInteractiveToolGUI::OnSessionEnded()
{
  // The session is gone; stop beating. Covers every teardown path (normal end,
  // time-point change, AbortSession after an expiry).
  m_HeartbeatTimer->stop();

  // Teardown is complete; re-arm the expiry handler for a future session.
  m_SessionExpiredHandled = false;

  // Clear the model license now that no session is bound.
  this->UpdateModelLicenseDisplay(std::nullopt);

  // Restore cursor and uncheck any active interactor button. The tool has
  // already disabled its interactor; this just keeps the GUI's check state in
  // sync.
  this->UncheckOtherInteractorButtons(nullptr);

  m_Ui->resetButton->setEnabled(false);
  m_Ui->undoButton->setEnabled(false);
  m_SupportsUndo = false;
  m_Ui->promptTypeGroupBox->setEnabled(false);
  m_Ui->interactionToolsGroupBox->setEnabled(false);

  // Re-enable Initialize and uncheck it without re-triggering
  // OnInitializeButtonToggled, then restore the idle label now that no session
  // is running (the session has already been torn down when this fires).
  this->UncheckInitializeButton();
  m_Ui->initializeButton->setEnabled(true);
  this->UpdateInitializeButtonText();
  m_Ui->settingsButton->setEnabled(true);

  // No session is running now; revert the Confirm button to its base label.
  this->UpdateConfirmButtonLabel();

  // Re-arm the once-per-session network checks so the next initialize re-runs the
  // version-update and model-switch prompts (covers uninitialize/reinitialize and
  // preference changes made between sessions).
  m_OnlineUpdateCheckDone = false;
  m_ModelSwitchCheckDone = false;
}

void QmitknnInteractiveToolGUI::OnHeartbeatTimeout()
{
  if (auto* tool = this->GetTool())
    tool->Heartbeat();
}

void QmitknnInteractiveToolGUI::UpdateModelLicenseDisplay(const std::optional<std::string>& license)
{
  auto* label = m_Ui->modelLicenseLabel;

  // Render as plain text: the license comes from the model checkpoint (for a
  // remote session, mirrored from the server's capabilities), so it must never
  // be interpreted as HTML markup that could hide or distort the terms.
  label->setTextFormat(Qt::PlainText);

  if (!license.has_value() || license->empty())
  {
    label->clear();
    label->setStyleSheet(QString());
    label->setVisible(false);
    return;
  }

  const auto text = QString::fromStdString(*license).trimmed();

  if (text == "!!MISSING!!")
  {
    // Mirror the napari plugin: an unknown license is a warning, not an error,
    // but it must stand out so the user does not assume unrestricted use.
    label->setText("Model license: UNKNOWN (warning!)");
    label->setStyleSheet("color: #d9534f; font-weight: bold;");
  }
  else
  {
    label->setText(QString("Model license: %1").arg(text));
    label->setStyleSheet(QString());
  }

  label->setVisible(true);
}

void QmitknnInteractiveToolGUI::OnSessionExpired()
{
  // The tool emits SessionExpiredEvent from two places (the heartbeat and a
  // mid-interaction failure). Handle only the first one: otherwise a second
  // event arriving before the deferred teardown below runs would stack a second
  // identical dialog. The flag is cleared in OnSessionEnded, so a later genuine
  // expiry of a new session still fires.
  if (m_SessionExpiredHandled)
    return;

  m_SessionExpiredHandled = true;

  // Stop beating immediately so a second timeout cannot queue another
  // teardown/dialog before the deferred AbortSession below runs. (OnSessionEnded
  // also stops the timer once the session is actually torn down.)
  m_HeartbeatTimer->stop();

  // A remote session was lost. Tear it down on the next event-loop tick rather
  // than now: when this fires from within an interactor's event handling (a
  // connection loss detected mid-interaction), disabling/resetting interactors
  // inline is unsafe; deferring is also harmless when it fires from the
  // heartbeat timer (a proactive expiry). AbortSession() ends the session,
  // clears all prompts and the preview, and -- via SessionEndedEvent ->
  // OnSessionEnded -- reverts the widget to its pre-init state, so the user just
  // clicks Initialize to reconnect.
  QTimer::singleShot(0, this, [this]() {
    if (QCoreApplication::closingDown())
      return;

    if (auto* tool = this->GetTool())
      tool->AbortSession();

    const auto message = QString(
      "<h3 %1>Remote session ended</h3>"
      "<p %1>The connection to the nnInteractive server was lost or the session "
      "expired (idle timeout, server restart, or the server is at capacity).</p>"
      "<p %1>Your interactions were cleared. Click <em>Initialize</em> to start a "
      "new session.</p>").arg(LINE_HEIGHT_STYLE);

    QMessageBox::warning(nullptr, "nnInteractive", message);
  });
}

void QmitknnInteractiveToolGUI::ApplyCapabilityGating()
{
  auto* tool = this->GetTool();
  if (tool == nullptr)
    return;

  const auto caps = tool->GetSupportedInteractions();

  m_Ui->pointButton->setEnabled(caps.Point);
  m_Ui->boxButton->setEnabled(caps.Box);
  m_Ui->scribbleButton->setEnabled(caps.Scribble);
  m_Ui->lassoButton->setEnabled(caps.Lasso);

  // Cache whether this session supports single-level undo (nnInteractive
  // >= 2.3.3). The Undo button stays disabled until the first interaction.
  m_SupportsUndo = tool->SupportsUndo();
  this->UpdateUndoButtonState();

  // A checkpoint that advertises no interactions at all would leave the user with
  // an initialized session and no usable controls. Say so, rather than presenting
  // a silently dead panel.
  if (!caps.Point && !caps.Box && !caps.Scribble && !caps.Lasso && !caps.Mask)
  {
    QMessageBox::warning(nullptr, "nnInteractive",
      QString("<p %1>The connected nnInteractive model reports no supported "
              "interactions, so there is nothing to interact with. This usually "
              "indicates a misconfigured server or model checkpoint.</p>")
        .arg(LINE_HEIGHT_STYLE));
  }
}

void QmitknnInteractiveToolGUI::OnPreviewUpdated()
{
  // A prompt interaction just landed, so it is now undoable. Update before the
  // auto-confirm early-outs below so the Undo button reflects every interaction.
  this->UpdateUndoButtonState();

  if (m_AutoConfirmInProgress)
    return;

  if (!this->IsAutoConfirmEnabled())
    return;

  auto tool = this->GetTool();
  if (tool == nullptr || !tool->HasInteractions())
    return;

  auto confirmButton = this->GetConfirmSegmentationButton();
  if (confirmButton == nullptr)
    return;

  m_AutoConfirmInProgress = true;

  // Defer the click to the next event-loop tick so the current
  // DoUpdatePreview call can fully unwind before Confirm fires.
  QTimer::singleShot(0, this, [this, confirmButton]() {
    confirmButton->click();
    m_AutoConfirmInProgress = false;
  });
}

bool QmitknnInteractiveToolGUI::IsAutoCreateNextLabelEnabled() const
{
  return m_Preferences->GetBool("nnInteractive/autoCreateNextLabel", true);
}

bool QmitknnInteractiveToolGUI::AreShortcutsShownInLabels() const
{
  return m_Preferences->GetBool("nnInteractive/showShortcutsInLabels", true);
}

void QmitknnInteractiveToolGUI::ApplyShortcutLabels()
{
  const bool show = this->AreShortcutsShownInLabels();

  for (const auto& entry : m_ShortcutLabels)
  {
    entry.button->setText(show
      ? LabelWithShortcut(entry.baseText, entry.key)
      : entry.baseText);
  }

  m_Ui->promptTypeGroupBox->setTitle(show
    ? LabelWithShortcut(m_PromptTypeBaseTitle, PROMPT_TYPE_KEY)
    : m_PromptTypeBaseTitle);
}

void QmitknnInteractiveToolGUI::OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent& event)
{
  const auto& property = event.GetProperty();

  // A change to a setting baked into the session at initialization (inference
  // mode, server, model, backend, storage) makes a running session stale, so tear
  // it down; the user then re-initializes with the new settings. AbortSession()
  // (rather than the bare EndSession()) also clears the now-orphaned interaction
  // prompts and preview and refreshes the views, so the canvas does not keep
  // showing a result that no longer has a session behind it. SetProperty fires
  // this only on an actual value change, so clicking OK without edits is a no-op.
  // It is idempotent when several such keys change in one OK: the first
  // AbortSession() tears the session down, the rest see no running session.
  if (IsSessionDefiningPreference(property))
  {
    if (auto* tool = this->GetTool(); tool != nullptr && tool->IsSessionRunning())
      tool->AbortSession();
  }

  if (property == "nnInteractive/showShortcutsInLabels")
  {
    this->ApplyShortcutLabels();
    this->UpdateConfirmButtonLabel();
  }
  else if (property == "nnInteractive/inferenceMode" || property == "nnInteractive/installMode")
    this->UpdateInitializeButtonText();
}

void QmitknnInteractiveToolGUI::UpdateInitializeButtonText()
{
  // While a session is running the button uninitializes, so it reads
  // "Uninitialize". Otherwise it shows what the next click will do, reflecting
  // the configured inference mode. This is the single source of truth and is
  // also called on inference-mode preference changes, so the running-session
  // guard keeps it from clobbering "Uninitialize" mid-session.
  auto* tool = this->GetTool();
  if (tool != nullptr && tool->IsSessionRunning())
  {
    m_Ui->initializeButton->setText("Uninitialize");
    return;
  }

  // A client-only install can only run remote sessions, so the button always
  // reflects remote initialization regardless of the stored inference mode.
  const bool clientOnly = m_Preferences->Get("nnInteractive/installMode", "full") == "client";
  const bool remote = clientOnly || m_Preferences->Get("nnInteractive/inferenceMode", "local") == "remote";
  m_Ui->initializeButton->setText(remote ? "Initialize (remote server)" : "Initialize");
}

void QmitknnInteractiveToolGUI::UncheckInitializeButton()
{
  // Revert the toggle without re-entering OnInitializeButtonToggled, which would
  // immediately start or end a session against the user's intent.
  QSignalBlocker blocker(m_Ui->initializeButton);
  m_Ui->initializeButton->setChecked(false);
}

bool QmitknnInteractiveToolGUI::IsAutoConfirmEnabled() const
{
  return m_Preferences->GetBool("nnInteractive/autoConfirm", false);
}

bool QmitknnInteractiveToolGUI::IsNamingPromptSkippedOnAutoCreate() const
{
  return m_Preferences->GetBool("nnInteractive/autoCreateNextLabelSkipNamingPrompt", true);
}

bool QmitknnInteractiveToolGUI::AutoCreateAndSelectNewLabel()
{
  // Delegate to the host's QmitkMultiLabelInspector so the "default label
  // naming" and "enforce suggestions" preferences are honored (including the
  // naming/color dialog), matching the behavior of the "New label" button in
  // the Segmentation plugin.
  auto* inspector = this->GetMultiLabelInspector();
  if (inspector == nullptr)
    return false;

  auto toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  if (toolManager == nullptr)
    return false;

  auto workingNode = toolManager->GetWorkingData(0);
  if (workingNode == nullptr)
    return false;

  auto segmentation = workingNode->GetDataAs<mitk::MultiLabelSegmentation>();
  if (segmentation == nullptr)
    return false;

  const auto* activeLabel = segmentation->GetActiveLabel();
  if (activeLabel == nullptr)
    return false;

  const auto previousActiveValue = activeLabel->GetValue();

  // Driving the inspector's selection emits CurrentSelectionChanged
  // synchronously, which would re-enter OnActiveLabelChanged and tear down or
  // re-seed the just-confirmed session. Suppress our slot until the selection
  // settles on the new label; the scope guard restores it even if AddNewLabel
  // throws.
  m_SuppressActiveLabelChanged = true;
  auto restoreSuppression = qScopeGuard([this] { m_SuppressActiveLabelChanged = false; });

  // Align the inspector's selection with the active label so that
  // AddNewLabel() derives the correct group for the new label.
  inspector->SetSelectedLabel(previousActiveValue);
  auto* addedLabel = inspector->AddNewLabel(this->IsNamingPromptSkippedOnAutoCreate());

  // Dialog canceled or creation failed; keep the previous label active.
  if (addedLabel == nullptr)
    return false;

  m_AutoCreatedLabelValue = addedLabel->GetValue();
  m_PreviousActiveLabelValue = previousActiveValue;
  m_AutoCreatedLabelSegmentation = segmentation;
  m_AutoCreatedLabelSegmentation.SetDeleteEventCallback(
    [this] { this->OnAutoCreatedSegmentationDeleted(); });

  return true;
}

void QmitknnInteractiveToolGUI::InvalidateAutoCreatedLabel()
{
  m_AutoCreatedLabelValue.reset();
  m_PreviousActiveLabelValue.reset();
  m_AutoCreatedLabelSegmentation = nullptr;
}

void QmitknnInteractiveToolGUI::OnAutoCreatedSegmentationDeleted()
{
  // Delegate to keep a single canonical clearing path. Reassigning the
  // WeakPointer to nullptr inside its own delete callback is safe: the
  // raw pointer was already cleared before this callback was invoked,
  // so RemoveDeleteEventObserver early-outs.
  this->InvalidateAutoCreatedLabel();
}

void QmitknnInteractiveToolGUI::ReEnableLastInteractor()
{
  if (m_LastInteractorButton != nullptr)
    m_LastInteractorButton->setChecked(true);
}
