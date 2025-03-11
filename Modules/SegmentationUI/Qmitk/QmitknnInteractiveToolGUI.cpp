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
#include <mitkToolManagerProvider.h>

#include <QmitkStyleManager.h>

#include <QBoxLayout>
#include <QButtonGroup>
#include <QMessageBox>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitknnInteractiveToolGUI, "")

namespace
{
  void SetIcon(QAbstractButton* button, const char* icon)
  {
    button->setIcon(QmitkStyleManager::ThemeIcon(QString(":/nnInteractive/%1").arg(icon)));
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

  bool IsLabelEmpty(mitk::LabelSetImage* segmentation, mitk::Label* label)
  {
    const auto timePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
    const auto timeStep = segmentation->GetTimeGeometry()->TimePointToTimeStep(timePoint);

    if (!segmentation->IsEmpty(label, timeStep))
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

  QCursor LoadToolCursor(mitk::nnInteractiveTool::Tool tool, mitk::nnInteractiveTool::PromptType promptType)
  {
    auto fileName = QString(":/nnInteractive/%1_cursor_%2")
      .arg(QString::fromStdString(mitk::nnInteractiveTool::GetToolString(tool)))
      .arg(QString::fromStdString(mitk::nnInteractiveTool::GetPromptTypeString(promptType)));

    QPixmap pixmap;
    pixmap.load(fileName);

    return QCursor(pixmap, 0, 0);
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
  this->UncheckOtherToolButtons(nullptr); // Ensure override cursor restoration.

  this->GetTool()->nnInterConfirmMessageEvent -= mitk::MessageDelegate1<QmitknnInteractiveToolGUI, bool>(
    this, &QmitknnInteractiveToolGUI::StatusMessageListener);
}

void QmitknnInteractiveToolGUI::InitializeUI(QBoxLayout* mainLayout)
{
  auto wrapperWidget = new QWidget(this);
  mainLayout->addWidget(wrapperWidget);
  m_Ui->setupUi(wrapperWidget);

  this->ThemeIcons();

  connect(m_Ui->initializeButton, &QPushButton::toggled, this, &Self::OnInitializeButtonToggled);
  connect(m_Ui->resetButton, &QPushButton::clicked, this, &Self::OnResetInteractionsButtonClicked);

  this->InitializePromptType();
  this->InitializeToolButtons();

  m_Ui->autoRefineCheckBox->setChecked(this->GetTool()->GetAutoRefine());
  connect(m_Ui->autoRefineCheckBox, &QCheckBox::toggled, this, &Self::OnAutoRefineCheckBoxToggled);

  m_Ui->autoZoomCheckBox->setChecked(this->GetTool()->GetAutoZoom());
  connect(m_Ui->autoZoomCheckBox, &QCheckBox::toggled, this, &Self::OnAutoZoomCheckBoxToggled);

  this->GetTool()->nnInterConfirmMessageEvent += mitk::MessageDelegate1<QmitknnInteractiveToolGUI, bool>(
    this, &QmitknnInteractiveToolGUI::StatusMessageListener);

  Superclass::InitializeUI(mainLayout);
}

void QmitknnInteractiveToolGUI::ThemeIcons()
{
  SetIcon(m_Ui->resetButton, "reset");
  SetIcon(m_Ui->positiveButton, "positive");
  SetIcon(m_Ui->negativeButton, "negative");
  SetIcon(m_Ui->pointButton, "point");
  SetIcon(m_Ui->boxButton, "box");
  SetIcon(m_Ui->scribbleButton, "scribble");
  SetIcon(m_Ui->lassoButton, "lasso");
  SetIcon(m_Ui->maskButton, "mask");
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

  connect(m_PromptTypeButtonGroup, &QButtonGroup::idClicked, [this](int id)
  {
    if (id != static_cast<int>(m_PromptType))
    {
      m_PromptType = static_cast<PromptType>(id);
      this->OnPromptTypeChanged();
    }
  });
}

void QmitknnInteractiveToolGUI::InitializeToolButtons()
{
  m_ToolButtons[Tool::Point] = m_Ui->pointButton;
  m_ToolButtons[Tool::Box] = m_Ui->boxButton;
  m_ToolButtons[Tool::Scribble] = m_Ui->scribbleButton;
  m_ToolButtons[Tool::Lasso] = m_Ui->lassoButton;

  for (const auto& [tool, button] : m_ToolButtons)
    connect(button, &QPushButton::toggled, [this, tool](bool checked) { this->OnToolToggled(tool, checked); });

  connect(m_Ui->maskButton, &QPushButton::clicked, this, &Self::OnMaskButtonClicked);
}

void QmitknnInteractiveToolGUI::OnInitializeButtonToggled(bool checked)
{
  m_Ui->initializeButton->setEnabled(!checked);

  QString title = "nnInteractive";

  auto message = QString(
    "<div style='line-height: 1.25'>"
      "<p>Initializing, please wait a few seconds...</p>"
      "<p><small><em>Note:</em> The first initialization after downloading MITK may take a minute "
      "instead. Please be patient.</small></p>"
    "</div>");
 
  QMessageBox messageBox(QMessageBox::Information, title, message);
  messageBox.setStandardButtons(QMessageBox::NoButton);
  messageBox.show();
  qApp->processEvents();
  try
  {
    this->GetTool()->InitializeBackend();
  }
  catch (const mitk::Exception& e)
  {
    messageBox.accept();
    std::stringstream errorMsg;
    errorMsg << "Error while initializing nnInteractive.\n\nReason: " << e.GetDescription();
    QMessageBox *errorMsgBox = new QMessageBox(QMessageBox::Critical, nullptr, errorMsg.str().c_str());
    errorMsgBox->setTextFormat(Qt::PlainText);
    errorMsgBox->setAttribute(Qt::WA_DeleteOnClose, true);
    errorMsgBox->setModal(true);
    MITK_WARN << errorMsg.str();
    errorMsgBox->exec();
    m_Ui->initializeButton->setEnabled(true);
    return;
  }
  messageBox.accept();

  m_Ui->resetButton->setEnabled(checked);
  m_Ui->promptTypeGroupBox->setEnabled(checked);
  m_Ui->interactionToolsGroupBox->setEnabled(checked);

  auto backend = this->GetTool()->GetBackend();

  if (!backend.has_value())
    return;

  if (backend == mitk::nnInteractiveTool::Backend::CUDA)
    return;

  message = QString(
    "<div style='line-height: 1.25'>"
      "<p><strong>Warning:</strong> The CUDA backend is unavailable. Falling back to the CPU backend, which is "
      "<em>significantly (!)</em> slower.</p>"
      "<p>For a smooth experience and quick response times, a compatible NVIDIA GPU with up-to-date drivers "
      "is highly recommended.</p>"
    "</div>");

  QMessageBox::warning(nullptr, title, message);
}

void QmitknnInteractiveToolGUI::OnResetInteractionsButtonClicked()
{
  for (const auto& [tool, button] : m_ToolButtons)
    button->setChecked(false);

  this->GetTool()->ResetInteractions();
  m_Ui->positiveButton->click();
}

void QmitknnInteractiveToolGUI::OnPromptTypeChanged()
{
  for (auto tool : this->GetTool()->GetTools())
  {
    if (m_ToolButtons[tool]->isChecked())
    {
      this->GetTool()->EnableInteraction(tool, m_PromptType);
      QApplication::changeOverrideCursor(LoadToolCursor(tool, m_PromptType));
      break;
    }
  }
}

void QmitknnInteractiveToolGUI::OnToolToggled(Tool tool, bool checked)
{
  if (checked)
  {
    this->UncheckOtherToolButtons(m_ToolButtons[tool]);
    this->GetTool()->EnableInteraction(tool, m_PromptType);
    QApplication::setOverrideCursor(LoadToolCursor(tool, m_PromptType));
  }
  else
  {
    QApplication::restoreOverrideCursor();
    this->GetTool()->DisableInteraction();
  }
}

void QmitknnInteractiveToolGUI::UncheckOtherToolButtons(QPushButton* toolButton)
{
  for (const auto& [tool, button] : m_ToolButtons)
  {
    if (button != toolButton)
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

  if (toolManager == nullptr)
  {
    MITK_ERROR << "Could not retrieve tool manager from tool manager provider!";
    return;
  }

  const auto* segmentationNode = toolManager->GetWorkingData(0);

  if (segmentationNode == nullptr)
  {
    MITK_ERROR << "Could not retrieve working data from tool manager!";
    return;
  }

  auto segmentation = segmentationNode->GetDataAs<mitk::LabelSetImage>();
  auto activeLabel = segmentation->GetActiveLabel();

  if (activeLabel == nullptr)
  {
    MITK_ERROR << "Could not retrieve active label from working data!";
    return;
  }

  if (IsLabelEmpty(segmentation, activeLabel))
    return;

  if (!ConfirmInitializationWithMask(activeLabel))
    return;

  auto mask = mitk::CreateLabelMask(segmentation, activeLabel->GetValue());

  if (mask.IsNull())
  {
    MITK_ERROR << "Could not create label mask!";
    return;
  }

  this->OnResetInteractionsButtonClicked();
  this->GetTool()->AddInitialSegInteraction(mask);
}

void QmitknnInteractiveToolGUI::StatusMessageListener(bool isConfirmed)
{
  if (isConfirmed)
  {
    this->OnResetInteractionsButtonClicked();
  }
}
