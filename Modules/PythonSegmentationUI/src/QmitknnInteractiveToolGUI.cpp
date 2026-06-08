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
#include <mitknnInteractiveInteractor.h>
#include <mitkPythonContext.h>
#include <mitkPythonHelper.h>
#include <mitkToolManagerProvider.h>

#include <QmitkMultiLabelInspector.h>
#include <QmitkPipInstallDialog.h>
#include <mitkPipPackageInfo.h>
#include <QmitkStyleManager.h>

#include <QApplication>
#include <QBoxLayout>
#include <QButtonGroup>
#include <QMessageBox>
#include <QShortcut>
#include <QTimer>
#include <QWidget>

MITK_TOOL_GUI_MACRO(MITKPYTHONSEGMENTATIONUI_EXPORT, QmitknnInteractiveToolGUI, "")

namespace
{
  constexpr auto LINE_HEIGHT_STYLE = "style='line-height: 1.25'";

  // Qt::Key_A..Qt::Key_Z are 0x41..0x5a and coincide with the ASCII codes
  // of the uppercase letters, so the same constant drives the QShortcut,
  // the tooltip hint, and the label suffix.
  constexpr Qt::Key RESET_KEY       = Qt::Key_R;
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

  QString GetLabelAsString(const mitk::Label* label)
  {
    QColor color(
      static_cast<int>(label->GetColor().GetRed() * 255),
      static_cast<int>(label->GetColor().GetGreen() * 255),
      static_cast<int>(label->GetColor().GetBlue() * 255));

    auto name = QString::fromStdString(label->GetName());

    return QString("<span style='color: %1'>&#9609;</span> %2")
      .arg(color.name())
      .arg(name);
  }

  mitk::TimeStepType GetCurrentTimeStep(const mitk::BaseData* data)
  {
    const auto* renderingManager = mitk::RenderingManager::GetInstance();
    const auto* timeNavigationController = renderingManager->GetTimeNavigationController();
    const auto timePoint = timeNavigationController->GetSelectedTimePoint();
    const auto geometry = data->GetTimeGeometry();

    return geometry->TimePointToTimeStep(timePoint);
  }

  bool IsLabelEmpty(const mitk::MultiLabelSegmentation* segmentation, const mitk::Label* label)
  {
    if (!segmentation->IsEmpty(label, GetCurrentTimeStep(segmentation)))
      return false;

    auto message = QString(
      "<h3 %1>Initialize with Mask</h3>"
      "<p %1>The selected label cannot be used as a mask to start a new "
      "session because it is empty.</p>"
      "<p %1>Selected label: %2</p>")
      .arg(LINE_HEIGHT_STYLE)
      .arg(GetLabelAsString(label));

    QMessageBox::information(nullptr, "nnInteractive", message, QMessageBox::Ok);

    return true;
  }

  bool ConfirmInitializationWithMask(const mitk::Label* label)
  {
    auto message = QString(
      "<h3 %1>Initialize with Mask</h3>"
      "<p %1>Do you want to <strong>reset all interactions</strong> and start a "
      "new session based on the existing content of the selected label?</p>"
      "<p %1>Selected label: %2</p>")
      .arg(LINE_HEIGHT_STYLE)
      .arg(GetLabelAsString(label));

    auto button = QMessageBox::question(nullptr, "nnInteractive", message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    return button == QMessageBox::Yes;
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

  SetIcon(m_Ui->resetButton, "Reset");
  SetIcon(m_Ui->positiveButton, "Positive");
  SetIcon(m_Ui->negativeButton, "Negative");
  SetIcon(m_Ui->maskButton, "Mask");

  connect(m_Ui->initializeButton, &QPushButton::toggled, this, &Self::OnInitializeButtonToggled);
  connect(m_Ui->settingsButton, &QPushButton::clicked, this, &Self::OnSettingsButtonClicked);
  connect(m_Ui->resetButton, &QPushButton::clicked, this, &Self::OnResetInteractionsButtonClicked);

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
  // or expiry. Parented to this widget, so it is destroyed with the GUI.
  m_HeartbeatTimer = new QTimer(this);
  connect(m_HeartbeatTimer, &QTimer::timeout, this, &Self::OnHeartbeatTimeout);

  Superclass::InitializeUI(mainLayout);

  // TODO: Once we agree on a common shortcut concept, the confirm binding
  // should be moved to the base class.

  auto confirmButton = this->GetConfirmSegmentationButton();

  BindShortcut(this, RESET_KEY, m_Ui->resetButton, "Press %1 to reset all interactions");
  BindShortcut(this, CONFIRM_KEY, confirmButton, "Press %1 to confirm a segmentation");

  // Cache the base label of each shortcut-bound widget as seen from the
  // .ui file (and, for the confirm button, from the base class). The cache
  // is the single source of truth for ApplyShortcutLabels, so repeated
  // invocations never accumulate suffixes.

  m_ShortcutLabels = {
    { m_Ui->resetButton,    RESET_KEY,    m_Ui->resetButton->text() },
    { m_Ui->pointButton,    POINT_KEY,    m_Ui->pointButton->text() },
    { m_Ui->boxButton,      BOX_KEY,      m_Ui->boxButton->text() },
    { m_Ui->scribbleButton, SCRIBBLE_KEY, m_Ui->scribbleButton->text() },
    { m_Ui->lassoButton,    LASSO_KEY,    m_Ui->lassoButton->text() },
    { confirmButton,        CONFIRM_KEY,  confirmButton->text() },
  };
  m_PromptTypeBaseTitle = m_Ui->promptTypeGroupBox->title();

  auto prefService = mitk::CoreServices::GetPreferencesService();
  m_Preferences = prefService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  m_Preferences->OnPropertyChanged +=
    mitk::MessageDelegate1<QmitknnInteractiveToolGUI, const mitk::IPreferences::ChangeEvent&>(
      this, &QmitknnInteractiveToolGUI::OnPreferenceChangedEvent);

  this->ApplyShortcutLabels();
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

  connect(m_Ui->maskButton, &QPushButton::clicked, this, &Self::OnMaskButtonClicked);
}

bool QmitknnInteractiveToolGUI::Install()
{
  const auto venvName = this->GetTool()->GetVirtualEnvName();

  // If the venv already exists, check if packages are installed.
  // This avoids showing the install dialog when everything is up to date.
  if (mitk::PythonHelper::VirtualEnvExists(venvName))
  {
    if (!this->GetTool()->CreatePythonContext())
      return false;

    if (this->GetTool()->IsInstalled())
      return true;
  }

  // PyTorch needs a CUDA-specific index URL on Windows. On other platforms
  // pip uses the default PyPI index.
#if defined(_WIN32)
  // Starting with CUDA v12.9 we get the following error on our lowest
  // supported GPU architecture (e.g. GeForce 10 Series):
  //   torch.AcceleratorError: CUDA error: no kernel image is available
  //   for exec
  const std::string cudaIndexUrl = "https://download.pytorch.org/whl/cu128";
#else
  const std::string cudaIndexUrl;
#endif

  // Pre-fetch the model weights so the first StartSession() doesn't surprise
  // the user with a silent multi-minute download. The checkpoint name mirrors
  // the preference mitknnInteractiveTool::StartSession() reads. Guard each
  // link in the preferences chain so a missing preferences service doesn't
  // crash the installer before it even starts. In local mode the user points
  // at a checkpoint folder on disk, so no Hugging Face download is queued.
  std::string modelSource = "huggingface";
  std::string checkpoint = "nnInteractive_v1.0";
  if (auto* prefsService = mitk::CoreServices::GetPreferencesService())
  {
    if (auto* system = prefsService->GetSystemPreferences())
    {
      if (auto* prefs = system->Node("org.mitk.views.segmentation"))
      {
        modelSource = prefs->Get("nnInteractive/modelSource", modelSource);
        checkpoint = prefs->Get("nnInteractive/modelCheckpoint", checkpoint);
      }
    }
  }

  mitk::PipInstallSpec spec;
  spec.name = "nnInteractive";
  spec.venvName = venvName;
  spec.upgradePipFirst = true;

  mitk::PipInstallGroup torchGroup;
  torchGroup.requirements = { "torch>=2.8.0,<2.9.0", "torchvision>=0.23.0,<1.0.0" };
  torchGroup.indexUrl = cudaIndexUrl;
  spec.groups.push_back(std::move(torchGroup));

  mitk::PipInstallGroup nnInteractiveGroup;
  nnInteractiveGroup.requirements = { "nninteractive>=2.3.2,<3.0.0" };
  spec.groups.push_back(std::move(nnInteractiveGroup));

  if (modelSource != "local")
  {
    mitk::HuggingFaceDownload modelDownload;
    modelDownload.repoId = "nnInteractive/nnInteractive";
    modelDownload.allowPatterns = { checkpoint + "/*" };
    modelDownload.displayName = "model checkpoint " + checkpoint;
    modelDownload.optional = true;
    spec.huggingFaceDownloads.push_back(std::move(modelDownload));
  }

  QmitkPipInstallDialog dialog(spec, this);

  if (dialog.exec() != QDialog::Accepted)
    return false;

  // The dialog populated the venv (and possibly created it). Create a fresh
  // context so the embedded interpreter picks up the newly installed packages.
  // PythonContext checks Py_IsInitialized internally, so calling this a second
  // time after the early-return path above is safe.
  return this->GetTool()->CreatePythonContext();
}

void QmitknnInteractiveToolGUI::OnInitializeButtonToggled(bool /*checked*/)
{
#if defined(__APPLE__) && !defined(__aarch64__)
  QMessageBox::information(
    nullptr,
    "nnInteractive",
    QString(
      "<h3 %1>Unsupported Platform</h3>"
      "<p %1>nnInteractive requires an Apple Silicon Mac.</p>"
      "<p %1>It is not compatible with Intel-based Macs.</p>")
      .arg(LINE_HEIGHT_STYLE),
    QMessageBox::Ok);
#else
  this->EnableInitializeButtons(false);

  if (!Install())
  {
    this->EnableInitializeButtons(true);
    return;
  }

  const auto initMessage = QString(
    "<h3 %1>Initializing nnInteractive</h3>"
    "<p %1>Please wait a few seconds until nnInteractive is fully initialized...</p>").arg(LINE_HEIGHT_STYLE);
 
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

      auto errorMsgBox = new QMessageBox(QMessageBox::Critical, nullptr,
        QString("<p %1>%2</p>").arg(LINE_HEIGHT_STYLE).arg(headline));

      if (isPythonError)
        errorMsgBox->setDetailedText(description);

      errorMsgBox->setTextInteractionFlags(Qt::TextSelectableByMouse);
      errorMsgBox->setAttribute(Qt::WA_DeleteOnClose, true);
      errorMsgBox->setModal(true);
      errorMsgBox->exec();

      this->EnableInitializeButtons(true);
      return;
    }

    messageBox->accept();

    // Re-enable the settings button so the user can adjust preferences that
    // apply mid-session (e.g., interaction mode). The initialize button
    // stays disabled because re-initialization within the same session is
    // not supported.
    m_Ui->settingsButton->setEnabled(true);

    m_Ui->resetButton->setEnabled(true);
    m_Ui->promptTypeGroupBox->setEnabled(true);
    m_Ui->interactionToolsGroupBox->setEnabled(true);

    // Disable interaction buttons the loaded checkpoint does not support.
    this->ApplyCapabilityGating();

    // Keep a remote session alive: the client's own background heartbeat cannot
    // run while MITK is idle (the embedded interpreter holds the GIL on this
    // thread), so beat from the Qt event loop instead. Zero means a local
    // session or a server with the liveness timeout disabled (no heartbeat).
    const int heartbeatIntervalMs = this->GetTool()->GetHeartbeatIntervalMs();
    if (heartbeatIntervalMs > 0)
      m_HeartbeatTimer->start(heartbeatIntervalMs);

    // Show the model checkpoint license (CC BY-NC-SA 4.0 for the official model)
    // so users are aware of its non-commercial terms. Works for local and remote
    // sessions alike (the remote license comes from the server's capabilities).
    this->UpdateModelLicenseDisplay(this->GetTool()->GetModelLicense());

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
#endif
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

  // Switch to positive prompt type.
  m_Ui->positiveButton->click();
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

void QmitknnInteractiveToolGUI::OnMaskButtonClicked()
{
  auto toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  const auto* segmentation = toolManager->GetWorkingData(0)->GetDataAs<mitk::MultiLabelSegmentation>();
  auto activeLabel = segmentation->GetActiveLabel();

  if (activeLabel == nullptr)
  {
    MITK_ERROR << "Could not retrieve active label from segmentation!";
    return;
  }

  // Check if the label is empty and notify the user if so.
  if (IsLabelEmpty(segmentation, activeLabel))
    return;

  // Ask the user for confirmation, as this will reset all previous interactions
  // (if any) and display the selected label to ensure the user is aware of the
  // source label for the mask.
  if (!ConfirmInitializationWithMask(activeLabel))
    return;

  auto mask = mitk::CreateLabelMask(segmentation, activeLabel->GetValue());

  // Reset interactions and initialize a new session with a mask/label.
  this->OnResetInteractionsButtonClicked();
  this->GetTool()->InitializeSessionWithMask(mask);
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

  // Clear the model license now that no session is bound.
  this->UpdateModelLicenseDisplay(std::nullopt);

  // Restore cursor and uncheck any active interactor button. The tool has
  // already disabled its interactor; this just keeps the GUI's check state in
  // sync.
  this->UncheckOtherInteractorButtons(nullptr);

  m_Ui->resetButton->setEnabled(false);
  m_Ui->promptTypeGroupBox->setEnabled(false);
  m_Ui->interactionToolsGroupBox->setEnabled(false);

  // Re-enable Initialize and uncheck it without re-triggering OnInitializeButtonToggled,
  // which would immediately start a new session against the user's intent.
  {
    QSignalBlocker blocker(m_Ui->initializeButton);
    m_Ui->initializeButton->setChecked(false);
  }
  m_Ui->initializeButton->setEnabled(true);
  m_Ui->settingsButton->setEnabled(true);
}

void QmitknnInteractiveToolGUI::OnHeartbeatTimeout()
{
  if (auto* tool = this->GetTool())
    tool->Heartbeat();
}

void QmitknnInteractiveToolGUI::UpdateModelLicenseDisplay(const std::optional<std::string>& license)
{
  auto* label = m_Ui->modelLicenseLabel;

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
  m_Ui->maskButton->setEnabled(caps.Mask);
}

void QmitknnInteractiveToolGUI::OnPreviewUpdated()
{
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
  if (event.GetProperty() == "nnInteractive/showShortcutsInLabels")
    this->ApplyShortcutLabels();
  else if (event.GetProperty() == "nnInteractive/inferenceMode")
    this->UpdateInitializeButtonText();
}

void QmitknnInteractiveToolGUI::UpdateInitializeButtonText()
{
  const bool remote = m_Preferences->Get("nnInteractive/inferenceMode", "local") == "remote";
  m_Ui->initializeButton->setText(remote ? "Initialize (remote server)" : "Initialize (local)");
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
