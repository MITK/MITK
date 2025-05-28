/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTotalSegmentatorToolGUI.h"
#include <ui_QmitkTotalSegmentatorGUIControls.h>

#include <mitkProcessExecutor.h>
#include <mitkTotalSegmentatorTool.h>
#include <QApplication>
#include <QIcon>
#include <QmitkStyleManager.h>
#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkTotalSegmentatorToolGUI, "")

namespace
{
  mitk::IPreferences *GetPreferences()
  {
    auto *preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }
  
} // namespace

QmitkTotalSegmentatorToolGUI::QmitkTotalSegmentatorToolGUI()
  : QmitkMultiLabelSegWithPreviewToolGUIBase(),
    m_Controls(new Ui::QmitkTotalSegmentatorToolGUIControls),
    m_SuperclassEnableConfirmSegBtnFnc(m_EnableConfirmSegBtnFnc)
{
  m_EnableConfirmSegBtnFnc = [this](bool enabled)
  { return !m_FirstPreviewComputation ? m_SuperclassEnableConfirmSegBtnFnc(enabled) : false; };
  
  m_Preferences = GetPreferences();
  if (nullptr != m_Preferences)
  {
    m_Preferences->OnPropertyChanged +=
      mitk::MessageDelegate1<QmitkTotalSegmentatorToolGUI, const mitk::IPreferences::ChangeEvent &>(
        this, &QmitkTotalSegmentatorToolGUI::OnPreferenceChangedEvent);
  }
}

QmitkTotalSegmentatorToolGUI::~QmitkTotalSegmentatorToolGUI()
{
  if (nullptr != m_Preferences)
  {
    m_Preferences->OnPropertyChanged -=
      mitk::MessageDelegate1<QmitkTotalSegmentatorToolGUI, const mitk::IPreferences::ChangeEvent &>(
        this, &QmitkTotalSegmentatorToolGUI::OnPreferenceChangedEvent);
  }
}

void QmitkTotalSegmentatorToolGUI::ConnectNewTool(mitk::SegWithPreviewTool *newTool)
{
  Superclass::ConnectNewTool(newTool);
  m_FirstPreviewComputation = true;
}

void QmitkTotalSegmentatorToolGUI::InitializeUI(QBoxLayout *mainLayout)
{
  auto wrapperWidget = new QWidget(this);
  mainLayout->addWidget(wrapperWidget);
  m_Controls->setupUi(wrapperWidget);

  this->EnableAll(false);
  m_Controls->statusLabel->setTextFormat(Qt::RichText);
  m_Controls->subtaskComboBox->addItems(VALID_TASKS);
  bool hasLicense = m_Preferences->GetBool("TotalSeg/hasLicense", false);
  this->ToggleLicensedTasks(hasLicense);
  QString welcomeText = "<b>STATUS: </b><i>Welcome to the TotalSegmentator tool.</i>";
  connect(m_Controls->previewButton, SIGNAL(clicked()), this, SLOT(OnPreviewBtnClicked()));
  m_Controls->fastBox->setChecked(true);
  bool isAvailable = !(m_Preferences->Get("TotalSeg/totalSegPath", "").empty());
  if (isAvailable)
  {
    welcomeText += " TotalSegmentator is already installed.";
    this->EnableAll(isAvailable);
  }
  else
  {
    welcomeText += " TotalSegmentator is not installed. Please go to preferences.";
    this->EnableAll(isAvailable);
  }
  this->WriteStatusMessage(welcomeText);
  QIcon arrowIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/tango/scalable/actions/media-playback-start.svg"));
  m_Controls->previewButton->setIcon(arrowIcon);

  Superclass::InitializeUI(mainLayout);
}

void QmitkTotalSegmentatorToolGUI::EnableAll(bool isEnable)
{
  m_Controls->previewButton->setEnabled(isEnable);
  m_Controls->subtaskComboBox->setEnabled(isEnable);
}

void QmitkTotalSegmentatorToolGUI::OnPreviewBtnClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::TotalSegmentatorTool>();
  if (nullptr == tool)
  {
    return;
  }
  try
  {
    m_Controls->previewButton->setEnabled(false);
    qApp->processEvents();
    bool isFast = m_Controls->fastBox->isChecked();
    QString subTask = m_Controls->subtaskComboBox->currentText();
    if (subTask != VALID_TASKS[0] && subTask != VALID_TASKS[1])
    {
      isFast = true;
    }
    QString totalPath = QString::fromStdString(m_Preferences->Get("TotalSeg/totalSegPath", ""));
    tool->SetPythonPath(totalPath.toStdString());
    tool->SetGpuId(m_Preferences->GetInt("TotalSeg/deviceId", 0));
    tool->SetFast(isFast);
    tool->SetSubTask(subTask.toStdString());
    this->WriteStatusMessage(QString("<b>STATUS: </b><i>Starting Segmentation task... This might take a while.</i>"));
    m_FirstPreviewComputation = false;
    tool->UpdatePreview();
    m_Controls->previewButton->setEnabled(true);
  }
  catch (const std::exception &e)
  {
    std::stringstream errorMsg;
    errorMsg << "<b>STATUS: </b>Error while processing parameters for TotalSegmentator segmentation. Reason: "
             << e.what();
    this->ShowErrorMessage(errorMsg.str());
    this->WriteErrorMessage(QString::fromStdString(errorMsg.str()));
    m_Controls->previewButton->setEnabled(true);
    m_FirstPreviewComputation = true;
    return;
  }
  catch (...)
  {
    std::string errorMsg = "Unknown error occurred while generation TotalSegmentator segmentation.";
    this->ShowErrorMessage(errorMsg);
    m_Controls->previewButton->setEnabled(true);
    m_FirstPreviewComputation = true;
    return;
  }
  this->SetLabelSetPreview(tool->GetPreviewSegmentation());
  this->ActualizePreviewLabelVisibility();
  this->WriteStatusMessage("<b>STATUS: </b><i>Segmentation task finished successfully.</i>");
}

void QmitkTotalSegmentatorToolGUI::ShowErrorMessage(const std::string &message, QMessageBox::Icon icon)
{
  this->setCursor(Qt::ArrowCursor);
  QMessageBox *messageBox = new QMessageBox(icon, nullptr, message.c_str());
  messageBox->setAttribute(Qt::WA_DeleteOnClose, true);
  MITK_WARN << message;
  messageBox->exec();
}

void QmitkTotalSegmentatorToolGUI::WriteStatusMessage(const QString &message)
{
  m_Controls->statusLabel->setText(message);
  m_Controls->statusLabel->setStyleSheet("font-weight: bold; color: white");
  qApp->processEvents();
}

void QmitkTotalSegmentatorToolGUI::WriteErrorMessage(const QString &message)
{
  m_Controls->statusLabel->setText(message);
  m_Controls->statusLabel->setStyleSheet("font-weight: bold; color: red");
  qApp->processEvents();
}

void QmitkTotalSegmentatorToolGUI::OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent &)
{
  bool isAvailable = !(m_Preferences->Get("TotalSeg/totalSegPath", "").empty());
  this->EnableAll(isAvailable);
  QString text = "<b>STATUS: </b><i>Welcome to TotalSegmentator tool.";
  if (isAvailable)
  {
    text += " TotalSegmentator is already installed.";
    this->EnableAll(isAvailable);
  }
  else
  {
    text += " TotalSegmentator is not installed, or configured correctly. Please go to preferences.";
    this->EnableAll(isAvailable);
  }
  this->WriteStatusMessage(text);
  bool hasLicense = m_Preferences->GetBool("TotalSeg/hasLicense", false);
  this->ToggleLicensedTasks(hasLicense);
}

void QmitkTotalSegmentatorToolGUI::ToggleLicensedTasks(bool activate)
{
  if (activate)
  {
    m_Controls->subtaskComboBox->addItems(LICENSED_TASKS);
  }
  else
  {
    m_Controls->subtaskComboBox->clear();
    m_Controls->subtaskComboBox->addItems(VALID_TASKS);
  }
}

