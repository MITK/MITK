/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentAnythingToolGUI.h"

#include <mitkSegmentAnythingTool.h>
#include <mitkProcessExecutor.h>
#include <QApplication>
#include <QDir>
#include <QmitkStyleManager.h>
#include <QDirIterator>
#include <QFileDialog>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkSegmentAnythingToolGUI, "")

namespace
{
  mitk::IPreferences *GetPreferences()
  {
    auto *preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }
}

QmitkSegmentAnythingToolGUI::QmitkSegmentAnythingToolGUI() : QmitkSegWithPreviewToolGUIBase(true)
{
  m_EnableConfirmSegBtnFnc = [this](bool enabled)
  {
    bool result = false;
    auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>();
    if (nullptr != tool)
    {
      result = enabled && tool->HasPicks();
    }
    return result;
  };
  m_Preferences = GetPreferences();
  if (nullptr != m_Preferences)
  {
    m_Preferences->OnPropertyChanged +=
      mitk::MessageDelegate1<QmitkSegmentAnythingToolGUI, const mitk::IPreferences::ChangeEvent &>(
        this, &QmitkSegmentAnythingToolGUI::OnPreferenceChangedEvent);
  }
}

QmitkSegmentAnythingToolGUI::~QmitkSegmentAnythingToolGUI() 
{
  auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>();
  if (nullptr != tool)
  {
    tool->SAMStatusMessageEvent -= mitk::MessageDelegate1<QmitkSegmentAnythingToolGUI, const std::string&>(
      this, &QmitkSegmentAnythingToolGUI::StatusMessageListener);
  }
  if (nullptr != m_Preferences)
  {
  m_Preferences->OnPropertyChanged -=
    mitk::MessageDelegate1<QmitkSegmentAnythingToolGUI, const mitk::IPreferences::ChangeEvent &>(
      this, &QmitkSegmentAnythingToolGUI::OnPreferenceChangedEvent);
  }
}

void QmitkSegmentAnythingToolGUI::InitializeUI(QBoxLayout *mainLayout)
{
  auto wrapperWidget = new QWidget(this);
  mainLayout->addWidget(wrapperWidget);
  m_Controls.setupUi(wrapperWidget);

  m_Controls.statusLabel->setTextFormat(Qt::RichText);

  QString welcomeText;
  welcomeText = "<b>STATUS: </b><i>Welcome to Segment Anything tool. " +
                  QString::number(m_GpuLoader.GetGPUCount()) + " GPU(s) were detected.</i>";
  
  connect(m_Controls.activateButton, SIGNAL(clicked()), this, SLOT(OnActivateBtnClicked()));
  connect(m_Controls.resetButton, SIGNAL(clicked()), this, SLOT(OnResetPicksClicked()));

  QIcon arrowIcon = QmitkStyleManager::ThemeIcon(
    QStringLiteral(":/org_mitk_icons/icons/tango/scalable/actions/media-playback-start.svg"));
  m_Controls.activateButton->setIcon(arrowIcon);
  this->UpdateSAMStatusMessage(welcomeText);
  this->ShowProgressBar(false);
  m_Controls.samProgressBar->setMaximum(0);

  Superclass::InitializeUI(mainLayout);
}

void QmitkSegmentAnythingToolGUI::UpdateSAMStatusMessage(QString &initText)
{
  bool isInstalled = this->ValidatePrefences();
  if (isInstalled)
  {
    QString modelType = QString::fromStdString(m_Preferences->Get("sam modeltype", ""));
    initText += " SAM is already installed. Model type '" + modelType + "' selected in Preferences.";
  }
  else
  {
    initText += " SAM tool is not configured correctly. Please go to Preferences (Ctrl+P) > Segment Anything to "
                   "configure and/or install SAM.";
  }
  this->EnableAll(isInstalled);
  this->WriteStatusMessage(initText);
}

bool QmitkSegmentAnythingToolGUI::ValidatePrefences()
{
  if (nullptr == m_Preferences)
  {
    this->WriteErrorMessage("Error while loading preferences.");
    return false;
  }
  const QString storageDir = QString::fromStdString(m_Preferences->Get("sam python path", ""));
  std::string modelType = m_Preferences->Get("sam modeltype", "");
  return (!storageDir.isEmpty() && !modelType.empty());
}

void QmitkSegmentAnythingToolGUI::EnableAll(bool isEnable)
{
  m_Controls.activateButton->setEnabled(isEnable);
}

void QmitkSegmentAnythingToolGUI::WriteStatusMessage(const QString &message)
{
  m_Controls.statusLabel->setText(message);
  m_Controls.statusLabel->setStyleSheet("font-weight: bold; color: white");
  qApp->processEvents();
}

void QmitkSegmentAnythingToolGUI::WriteErrorMessage(const QString &message)
{
  m_Controls.statusLabel->setText(message);
  m_Controls.statusLabel->setStyleSheet("font-weight: bold; color: red");
  qApp->processEvents();
}

void QmitkSegmentAnythingToolGUI::ShowErrorMessage(const std::string &message, QMessageBox::Icon icon)
{
  this->setCursor(Qt::ArrowCursor);
  QMessageBox *messageBox = new QMessageBox(icon, nullptr, message.c_str());
  messageBox->exec();
  delete messageBox;
  MITK_WARN << message;
}

void QmitkSegmentAnythingToolGUI::StatusMessageListener(const std::string &message)
{
  if (message.rfind("Error", 0) == 0)
  {
    this->EnableAll(true);
    this->WriteErrorMessage(QString::fromStdString(message));
  }
  else if (message == "TimeOut")
  { // trying to re init the daemon
    this->WriteErrorMessage(QString("<b>STATUS: </b><i>Sorry, operation timed out. Reactivating SAM tool...</i>"));
    if (this->ActivateSAMDaemon())
    {
      this->WriteStatusMessage(QString("<b>STATUS: </b><i>Segment Anything tool re-initialized.</i>"));
    }
    else
    {
      this->WriteErrorMessage(QString("<b>STATUS: </b><i>Couldn't init tool backend.</i>"));
      this->EnableAll(true);
    }
  }
  else
  {
    this->WriteStatusMessage(QString::fromStdString(message));
  }
}

void QmitkSegmentAnythingToolGUI::OnActivateBtnClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>();
  if (nullptr == tool)
  {
    return;
  }
  try
  {
    this->EnableAll(false);
    qApp->processEvents();
    QString pythonPath = QString::fromStdString(m_Preferences->Get("sam python path", ""));
    if (pythonPath.isEmpty())
    {
      throw std::runtime_error(WARNING_SAM_NOT_FOUND);
    }
    tool->SetPythonPath(pythonPath.toStdString());
    tool->SetGpuId(m_Preferences->GetInt("sam deviceId", -1));
    const QString modelType = QString::fromStdString(m_Preferences->Get("sam modeltype", ""));  
    tool->SetModelType(modelType.toStdString());
    tool->SetTimeOutLimit(m_Preferences->GetInt("sam timeout", 300));
    tool->SetCheckpointPath(m_Preferences->Get("sam parent path", ""));
    tool->SetBackend("SAM");
    this->WriteStatusMessage(
      QString("<b>STATUS: </b><i>Initializing Segment Anything Model...</i>"));
    tool->SAMStatusMessageEvent += mitk::MessageDelegate1<QmitkSegmentAnythingToolGUI,const std::string&>(
      this, &QmitkSegmentAnythingToolGUI::StatusMessageListener);
    if (this->ActivateSAMDaemon())
    {
      this->WriteStatusMessage(QString("<b>STATUS: </b><i>Segment Anything tool initialized.</i>"));
    }
    else
    {
      this->WriteErrorMessage(QString("<b>STATUS: </b><i>Couldn't init tool backend.</i>"));
      this->EnableAll(true);
    }
  }
  catch (const std::exception &e)
  {
    std::stringstream errorMsg;
    errorMsg << "<b>STATUS: </b>Error while processing parameters for Segment Anything segmentation. Reason: " << e.what();
    this->ShowErrorMessage(errorMsg.str());
    this->WriteErrorMessage(QString::fromStdString(errorMsg.str()));
    this->EnableAll(true);
    return;
  }
  catch (...)
  {
    std::string errorMsg = "Unkown error occurred while generation Segment Anything segmentation.";
    this->ShowErrorMessage(errorMsg);
    this->EnableAll(true);
    return;
  }
}

bool QmitkSegmentAnythingToolGUI::ActivateSAMDaemon()
{
  auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>();
  if (nullptr == tool)
  {
    return false;
  }
  this->ShowProgressBar(true);
  qApp->processEvents();
  try
  {
    tool->InitSAMPythonProcess();
    while (!tool->IsPythonReady())
    {
      qApp->processEvents();
    }
    tool->IsReadyOn();
  }
  catch (...)
  {
    tool->IsReadyOff();
  }
  this->ShowProgressBar(false);
  return tool->GetIsReady();
}

void QmitkSegmentAnythingToolGUI::ShowProgressBar(bool enabled)
{
  m_Controls.samProgressBar->setEnabled(enabled);
  m_Controls.samProgressBar->setVisible(enabled);
}

void QmitkSegmentAnythingToolGUI::OnResetPicksClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>();
  if (nullptr != tool)
  {
    tool->ClearPicks();
  }
}

void QmitkSegmentAnythingToolGUI::OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent&)
{
  auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>();
  if (nullptr != tool)
  {
    tool->IsReadyOff();
  }
  QString statusMessage = "A Preference change was detected. Please initialize the tool again.\n";
  this->UpdateSAMStatusMessage(statusMessage);
}
