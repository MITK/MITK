/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMedSAMToolGUI.h"

#include <QApplication>
#include <QmitkStyleManager.h>
#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkMedSAMTool.h>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkMedSAMToolGUI, "")

namespace
{
  mitk::IPreferences *GetPreferences()
  {
    auto *preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }
}

QmitkMedSAMToolGUI::QmitkMedSAMToolGUI() : QmitkSegWithPreviewToolGUIBase(true)
{
  m_EnableConfirmSegBtnFnc = [this](bool enabled)
  {
    bool result = false;
    auto tool = this->GetConnectedToolAs<mitk::MedSAMTool>();
    if (nullptr != tool)
    {
      result = enabled && tool->HasPicks();
    }
    return result;
  };
  m_Preferences = GetPreferences();
  m_Preferences->OnPropertyChanged +=
    mitk::MessageDelegate1<QmitkMedSAMToolGUI, const mitk::IPreferences::ChangeEvent &>(
      this, &QmitkMedSAMToolGUI::OnPreferenceChangedEvent);
}

QmitkMedSAMToolGUI::~QmitkMedSAMToolGUI()
{
  auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>(); // check -ashis
  if (nullptr != tool)
  {
    tool->SAMStatusMessageEvent -=
      mitk::MessageDelegate1<QmitkMedSAMToolGUI, const std::string &>(this, &QmitkMedSAMToolGUI::StatusMessageListener);
  }
}

void QmitkMedSAMToolGUI::EnableAll(bool isEnable)
{
  m_Controls.activateButton->setEnabled(isEnable);
}

void QmitkMedSAMToolGUI::WriteStatusMessage(const QString &message)
{
  m_Controls.statusLabel->setText(message);
  m_Controls.statusLabel->setStyleSheet("font-weight: bold; color: white");
  qApp->processEvents();
}

void QmitkMedSAMToolGUI::WriteErrorMessage(const QString &message)
{
  m_Controls.statusLabel->setText(message);
  m_Controls.statusLabel->setStyleSheet("font-weight: bold; color: red");
  qApp->processEvents();
}

void QmitkMedSAMToolGUI::ShowProgressBar(bool enabled)
{
  m_Controls.samProgressBar->setEnabled(enabled);
  m_Controls.samProgressBar->setVisible(enabled);
}

void QmitkMedSAMToolGUI::ShowErrorMessage(const std::string &message, QMessageBox::Icon icon)
{
  this->setCursor(Qt::ArrowCursor);
  QMessageBox *messageBox = new QMessageBox(icon, nullptr, message.c_str());
  messageBox->exec();
  delete messageBox;
  MITK_WARN << message;
}

void QmitkMedSAMToolGUI::InitializeUI(QBoxLayout *mainLayout)
{
  m_Controls.setupUi(this);
  m_Controls.statusLabel->setTextFormat(Qt::RichText);

  QString welcomeText;
  if (m_GpuLoader.GetGPUCount() != 0)
  {
    welcomeText = "<b>STATUS: </b><i>Welcome to MedSAM Anything tool. You're in luck: " +
                  QString::number(m_GpuLoader.GetGPUCount()) + " GPU(s) were detected.</i>";
  }
  else
  {
    welcomeText = "<b>STATUS: </b><i>Welcome to MedSAM Anything tool. Sorry, " +
                  QString::number(m_GpuLoader.GetGPUCount()) + " GPUs were detected.</i>";
  }

  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnPreviewBtnClicked()));
  connect(m_Controls.activateButton, SIGNAL(clicked()), this, SLOT(OnActivateBtnClicked()));
  connect(m_Controls.resetButton, SIGNAL(clicked()), this, SLOT(OnResetPicksClicked()));

  QIcon arrowIcon = QmitkStyleManager::ThemeIcon(
    QStringLiteral(":/org_mitk_icons/icons/tango/scalable/actions/media-playback-start.svg"));
  m_Controls.activateButton->setIcon(arrowIcon);

  bool isInstalled = this->ValidatePrefences();
  if (isInstalled)
  {
    QString modelType = QString::fromStdString(m_Preferences->Get("sam modeltype", ""));
    welcomeText += " MedSAM is already found installed.";
  }
  else
  {
    welcomeText += " MedSAM tool is not configured correctly. Please go to Preferences (Cntl+P) > Segment Anything to "
                   "configure and/or install SAM & MedSAM.";
  }
  this->EnableAll(isInstalled);
  this->WriteStatusMessage(welcomeText);
  this->ShowProgressBar(false);
  m_Controls.samProgressBar->setMaximum(0);
  mainLayout->addLayout(m_Controls.verticalLayout);
  Superclass::InitializeUI(mainLayout);
}

bool QmitkMedSAMToolGUI::ValidatePrefences()
{
  const QString storageDir = QString::fromStdString(m_Preferences->Get("sam python path", ""));
  bool isInstalled = QmitkSegmentAnythingToolGUI::IsSAMInstalled(storageDir);
  std::string modelType = m_Preferences->Get("sam modeltype", "");
  std::string path = m_Preferences->Get("sam parent path", "");
  return (isInstalled && !modelType.empty() && !path.empty());
}

void QmitkMedSAMToolGUI::StatusMessageListener(const std::string &message)
{
  if (message.rfind("Error", 0) == 0)
  {
    this->EnableAll(true);
    this->WriteErrorMessage(QString::fromStdString(message));
  }
  else if (message == "TimeOut")
  { // trying to re init the daemon
    this->WriteErrorMessage(QString("<b>STATUS: </b><i>Sorry, operation timed out. Reactivating MedSAM tool...</i>"));
    if (this->ActivateSAMDaemon())
    {
      this->WriteStatusMessage(QString("<b>STATUS: </b><i>MedSAM tool re-initialized.</i>"));
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

bool QmitkMedSAMToolGUI::ActivateSAMDaemon()
{
  auto tool = this->GetConnectedToolAs<mitk::MedSAMTool>();
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

void QmitkMedSAMToolGUI::OnActivateBtnClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::MedSAMTool>();
  if (nullptr == tool)
  {
    return;
  }
  try
  {
    this->EnableAll(false);
    qApp->processEvents();
    QString pythonPath = QString::fromStdString(m_Preferences->Get("sam python path", ""));
    if (!QmitkSegmentAnythingToolGUI::IsSAMInstalled(pythonPath))
    {
      throw std::runtime_error(WARNING_SAM_NOT_FOUND);
    }
    tool->SetPythonPath(pythonPath.toStdString());
    tool->SetGpuId(m_Preferences->GetInt("sam gpuid", -1));
    tool->SetModelType("vit_b"); // MedSAM only works with vit_b
    tool->SetTimeOutLimit(m_Preferences->GetInt("sam timeout", 300));
    tool->SetCheckpointPath(m_Preferences->Get("sam parent path", ""));
    tool->SetBackend("MedSAM");
    this->WriteStatusMessage(QString("<b>STATUS: </b><i>Initializing MedSAM...</i>"));
    tool->SAMStatusMessageEvent +=
      mitk::MessageDelegate1<QmitkMedSAMToolGUI, const std::string &>(this, &QmitkMedSAMToolGUI::StatusMessageListener);
    if (this->ActivateSAMDaemon())
    {
      this->WriteStatusMessage(QString("<b>STATUS: </b><i>MedSAM tool initialized.</i>"));
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
    errorMsg << "<b>STATUS: </b>Error while processing parameters for MedSAM segmentation. Reason: " << e.what();
    this->ShowErrorMessage(errorMsg.str());
    this->WriteErrorMessage(QString::fromStdString(errorMsg.str()));
    this->EnableAll(true);
    return;
  }
  catch (...)
  {
    std::string errorMsg = "Unkown error occured while generation MedSAM segmentation.";
    this->ShowErrorMessage(errorMsg);
    this->EnableAll(true);
    return;
  }
}

void QmitkMedSAMToolGUI::OnPreviewBtnClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::MedSAMTool>();
  if (nullptr != tool)
  {
    tool->UpdatePreview();
  }
}

void QmitkMedSAMToolGUI::OnResetPicksClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::MedSAMTool>();
  if (nullptr != tool)
  {
    tool->ClearPicks();
  }
}

void QmitkMedSAMToolGUI::OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent &event)
{
  const std::string property = event.GetProperty();
  const std::string modelType = "modeltype";
  if (property.compare(property.size() - modelType.size(), modelType.size(), modelType) == 0)
    return; // Model type change ignored.

  this->EnableAll(true);
  this->WriteStatusMessage("A Preference change was detected. Please initialize the tool again.");
  auto tool = this->GetConnectedToolAs<mitk::MedSAMTool>();
  if (nullptr != tool)
  {
    tool->IsReadyOff();
  }
}
