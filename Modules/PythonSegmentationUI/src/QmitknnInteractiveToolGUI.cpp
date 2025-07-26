/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitknnInteractiveToolGUI.h"
#include <ui_QmitknnInteractiveToolGUI.h>

#include <mitkLabelSetImageConverter.h>
#include <mitknnInteractiveInteractor.h>
#include <mitkPythonContext.h>
#include <mitkToolManagerProvider.h>

#include <QmitknnInteractiveInstallDialog.h>
#include <QmitkStyleManager.h>

#include <QBoxLayout>
#include <QButtonGroup>
#include <QMessageBox>
#include <QShortcut>
#include <QTimer>

MITK_TOOL_GUI_MACRO(MITKPYTHONSEGMENTATIONUI_EXPORT, QmitknnInteractiveToolGUI, "")

namespace
{
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

    QString title = "nnInteractive - Initialize with Mask";

    auto message = QString(
      "<div style='line-height: 1.25'>"
        "<p>The selected label cannot be used as a mask to start a new "
        "session because it is empty.</p>"
        "<p>Selected label: %1</p>"
      "</div>")
      .arg(GetLabelAsString(label));

    QMessageBox::information(nullptr, title, message, QMessageBox::Ok);

    return true;
  }

  bool ConfirmInitializationWithMask(const mitk::Label* label)
  {
    QString title = "nnInteractive - Initialize with Mask";

    auto message = QString(
      "<div style='line-height: 1.25'>"
        "<p>Do you want to <b>reset all interactions</b> and start a new "
        "session based on the existing content of the selected label?</p>"
        "<p>Selected label: %1</p>"
      "</div>")
      .arg(GetLabelAsString(label));

    auto button = QMessageBox::question(nullptr, title, message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    return button == QMessageBox::Yes;
  }
}

QmitknnInteractiveToolGUI::QmitknnInteractiveToolGUI()
  : QmitkSegWithPreviewToolGUIBase(false, false),
    m_Ui(new Ui::QmitknnInteractiveToolGUI),
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

  this->GetTool()->ConfirmCleanUpEvent -= mitk::MessageDelegate1<QmitknnInteractiveToolGUI, bool>(
    this, &QmitknnInteractiveToolGUI::OnConfirmCleanUp);
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
  connect(m_Ui->resetButton, &QPushButton::clicked, this, &Self::OnResetInteractionsButtonClicked);

  this->InitializePromptType();
  this->InitializeInteractorButtons();

  m_Ui->autoRefineCheckBox->setChecked(this->GetTool()->GetAutoRefine());
  connect(m_Ui->autoRefineCheckBox, &QCheckBox::toggled, this, &Self::OnAutoRefineCheckBoxToggled);

  m_Ui->autoZoomCheckBox->setChecked(this->GetTool()->GetAutoZoom());
  connect(m_Ui->autoZoomCheckBox, &QCheckBox::toggled, this, &Self::OnAutoZoomCheckBoxToggled);

  this->GetTool()->ConfirmCleanUpEvent += mitk::MessageDelegate1<QmitknnInteractiveToolGUI, bool>(
    this, &QmitknnInteractiveToolGUI::OnConfirmCleanUp);

  Superclass::InitializeUI(mainLayout);

  // Set shortcut to reset all interactions.

  m_Ui->resetButton->setToolTip("Press R to reset all interactions");
  auto reset = new QShortcut(QKeySequence(Qt::Key_R), this);
  connect(reset, &QShortcut::activated, m_Ui->resetButton, &QPushButton::click);

  // Set shortcut to confirm a segmentation.
  // TODO: Once we agree on a common shortcut concept, this should be moved to the base class.

  auto confirmButton = this->GetConfirmSegmentationButton();
  confirmButton->setToolTip("Press C to confirm a segmentation");
  auto confirmSegmentation = new QShortcut(QKeySequence(Qt::Key_C), this);
  connect(confirmSegmentation, &QShortcut::activated, confirmButton, &QPushButton::click);
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

  const QString toolTip("Press T to switch the prompt types");
  m_Ui->positiveButton->setToolTip(toolTip);
  m_Ui->negativeButton->setToolTip(toolTip);

  auto togglePromptType = new QShortcut(QKeySequence(Qt::Key_T), this);

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

  m_Ui->pointButton->setToolTip("Press P to toggle the point interaction");
  auto togglePointInteractor = new QShortcut(QKeySequence(Qt::Key_P), this);
  connect(togglePointInteractor, &QShortcut::activated, m_Ui->pointButton, &QPushButton::click);

  m_Ui->boxButton->setToolTip("Press B to toggle the box interaction");
  auto toggleBoxInteractor = new QShortcut(QKeySequence(Qt::Key_B), this);
  connect(toggleBoxInteractor, &QShortcut::activated, m_Ui->boxButton, &QPushButton::click);

  m_Ui->scribbleButton->setToolTip("Press S to toggle the scribble interaction");
  auto toggleScribbleInteractor = new QShortcut(QKeySequence(Qt::Key_S), this);
  connect(toggleScribbleInteractor, &QShortcut::activated, m_Ui->scribbleButton, &QPushButton::click);

  m_Ui->lassoButton->setToolTip("Press L to toggle the lasso interaction");
  auto toggleLassoInteractor = new QShortcut(QKeySequence(Qt::Key_L), this);
  connect(toggleLassoInteractor, &QShortcut::activated, m_Ui->lassoButton, &QPushButton::click);

  m_InteractorButtons[InteractionType::Point] = m_Ui->pointButton;
  m_InteractorButtons[InteractionType::Box] = m_Ui->boxButton;
  m_InteractorButtons[InteractionType::Scribble] = m_Ui->scribbleButton;
  m_InteractorButtons[InteractionType::Lasso] = m_Ui->lassoButton;

  for (const auto& interactorButton : m_InteractorButtons)
  {
    // We cannot use a structured binding in this for loop because
    // interactionType is captured by the lambda below. Capturing structured
    // bindings is a C++20 feature and currently we are bound to C++17.
    auto interactionType = interactorButton.first;
    auto button = interactorButton.second;

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
  if (this->GetTool()->IsInstalled())
    return true;

  QmitknnInteractiveInstallDialog installDialog;
  return installDialog.exec() == QDialog::Accepted;
}

void QmitknnInteractiveToolGUI::OnInitializeButtonToggled(bool /*checked*/)
{
  m_Ui->initializeButton->setEnabled(false);

  this->GetTool()->CreatePythonContext();

  if (!this->Install())
  {
    m_Ui->initializeButton->setEnabled(true);
    return;
  }

  const QString title = "nnInteractive";

  const QString initMessage(
    "<div style='line-height: 1.25'>"
      "<p>Initializing, please wait a few seconds...</p>"
      "<p><small><em>Note:</em> The first initialization after downloading MITK may take a minute "
      "instead. Please be patient.</small></p>"
    "</div>");
 
  auto messageBox = new QMessageBox(QMessageBox::Information, title, initMessage);
  messageBox->setStandardButtons(QMessageBox::NoButton);
  messageBox->setAttribute(Qt::WA_DeleteOnClose);
  messageBox->show();
  qApp->processEvents();

  QTimer::singleShot(100, this, [=]() {
    try
    {
      this->GetTool()->StartSession();
    }
    catch (const mitk::Exception& e)
    {
      messageBox->accept();

      const auto errorMessage = QString(
        "<div style='line-height: 1.25'>"
        "<p>Error while initializing nnInteractive.</p>"
        "<p>Reason: %1</p>"
        "</div>")
        .arg(QString::fromLocal8Bit(e.GetDescription()));

      MITK_ERROR << errorMessage.toStdString();
      auto errorMsgBox = new QMessageBox(QMessageBox::Critical, nullptr, errorMessage);
      errorMsgBox->setTextInteractionFlags(Qt::TextSelectableByMouse);
      errorMsgBox->setAttribute(Qt::WA_DeleteOnClose, true);
      errorMsgBox->setModal(true);
      errorMsgBox->exec();

      m_Ui->initializeButton->setEnabled(true);
      return;
    }

    messageBox->accept();

    m_Ui->resetButton->setEnabled(true);
    m_Ui->promptTypeGroupBox->setEnabled(true);
    m_Ui->interactionToolsGroupBox->setEnabled(true);

    auto backend = this->GetTool()->GetBackend();

    if (!backend.has_value())
      return;

    if (backend == mitk::nnInteractive::Backend::CUDA)
      return;

    m_Ui->autoZoomCheckBox->setEnabled(false);
    m_Ui->autoZoomCheckBox->setChecked(false);
    m_Ui->autoZoomCheckBox->setToolTip("Auto-zoom is not available with CPU backend.");

    const QString cpuBackendMessage = QString(
      "<div style='line-height: 1.25'>"
      "<p><strong>Warning:</strong> The CUDA backend is unavailable. Falling back to the CPU backend, which is "
      "<em>significantly (!)</em> slower.</p>"
      "<p>For a smooth experience and quick response times, a compatible NVIDIA GPU with up-to-date drivers "
      "is highly recommended.</p>"
      "</div>");

    QMessageBox::warning(nullptr, title, cpuBackendMessage);
  });
}

void QmitknnInteractiveToolGUI::OnResetInteractionsButtonClicked()
{
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
  if (isConfirmed)
    this->OnResetInteractionsButtonClicked();
}
