/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentAnythingToolGUI.h"

#include "mitkSegmentAnythingTool.h"
#include "mitkProcessExecutor.h"
#include <QApplication>
#include <QDir>
#include <QmitkStyleManager.h>
#include <QDirIterator>
#include <QFileDialog>
#include <QNetworkRequest>
#include <QtConcurrentRun>

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
  // Nvidia-smi command returning zero doesn't always imply lack of GPUs.
  // Pytorch uses its own libraries to communicate to the GPUs. Hence, only a warning can be given.
  if (m_GpuLoader.GetGPUCount() == 0)
  {
    std::string warning = "WARNING: No GPUs were detected on your machine. The SegmentAnything tool can be very slow.";
    this->ShowErrorMessage(warning);
  }
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
  m_Prefences = GetPreferences();
}

QmitkSegmentAnythingToolGUI::~QmitkSegmentAnythingToolGUI() 
{
  auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>();
  if (nullptr != tool)
  {
    tool->SAMStatusMessageEvent -= mitk::MessageDelegate1<QmitkSegmentAnythingToolGUI, const std::string&>(
      this, &QmitkSegmentAnythingToolGUI::StatusMessageListener);
  }
}

void QmitkSegmentAnythingToolGUI::InitializeUI(QBoxLayout *mainLayout)
{
  m_Controls.setupUi(this);
  m_Controls.statusLabel->setTextFormat(Qt::RichText);

  QString welcomeText;
  if (m_GpuLoader.GetGPUCount() != 0)
  {
    welcomeText = "<b>STATUS: </b><i>Welcome to Segment Anything tool. You're in luck: " +
                  QString::number(m_GpuLoader.GetGPUCount()) + " GPU(s) were detected.</i>";
  }
  else
  {
    welcomeText = "<b>STATUS: </b><i>Welcome to Segment Anything tool. Sorry, " +
                  QString::number(m_GpuLoader.GetGPUCount()) + " GPUs were detected.</i>";
  }
  connect(m_Controls.activateButton, SIGNAL(clicked()), this, SLOT(OnActivateBtnClicked()));
  connect(m_Controls.resetButton, SIGNAL(clicked()), this, SLOT(OnResetPicksClicked()));

  QIcon arrowIcon = QmitkStyleManager::ThemeIcon(
    QStringLiteral(":/org_mitk_icons/icons/tango/scalable/actions/media-playback-start.svg"));
  m_Controls.activateButton->setIcon(arrowIcon);

  bool isInstalled = this->ValidatePrefences();
  if (isInstalled)
  {
    m_PythonPath = QString::fromStdString(m_Prefences->Get("sam python path", ""));
    QString modelType = QString::fromStdString(m_Prefences->Get("sam modeltype", ""));
    welcomeText += " SAM is already found installed. Model type '" + modelType + "' selected in Preferences.";
  }
  else
  {
    welcomeText += " SAM tool is not configured correctly. Please go to Preferences (Cntl+P) > Segment Anything to configure and/or install SAM.";
  }
  this->EnableAll(isInstalled);
  this->WriteStatusMessage(welcomeText);
  this->ShowProgressBar(false);
  m_Controls.samProgressBar->setMaximum(0);

  mainLayout->addLayout(m_Controls.verticalLayout);
  Superclass::InitializeUI(mainLayout);
}

bool QmitkSegmentAnythingToolGUI::ValidatePrefences()
{
  const QString storageDir = QString::fromStdString(m_Prefences->Get("sam python path", ""));
  bool isInstalled = QmitkSegmentAnythingToolGUI::IsSAMInstalled(storageDir);
  std::string modelType = m_Prefences->Get("sam modeltype", "");
  std::string path = m_Prefences->Get("sam parent path", "");
  return (isInstalled && !modelType.empty() && !path.empty());
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
  this->WriteStatusMessage(QString::fromStdString(message));
  if (message.rfind("Error", 0) == 0)
  {
    this->EnableAll(true);
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
    if (!QmitkSegmentAnythingToolGUI::IsSAMInstalled(m_PythonPath))
    {
      throw std::runtime_error(WARNING_SAM_NOT_FOUND);
    }
    tool->SetIsAuto(false);
    tool->SetPythonPath(m_PythonPath.toStdString());
    tool->SetGpuId(m_Prefences->GetInt("sam gpuid", 0));
    const QString modelType = QString::fromStdString(m_Prefences->Get("sam modeltype", ""));  
    tool->SetModelType(modelType.toStdString());
    this->WriteStatusMessage(
      QString("<b>STATUS: </b><i>Checking if model is already downloaded... This might take a while.</i>"));
    tool->SAMStatusMessageEvent += mitk::MessageDelegate1<QmitkSegmentAnythingToolGUI,const std::string&>(
      this, &QmitkSegmentAnythingToolGUI::StatusMessageListener);
    if (this->DownloadModel(modelType))
    {
      this->ActivateSAMDaemon();
      this->WriteStatusMessage(QString("<b>STATUS: </b><i>Model found. SAM Activated.</i>"));
    }
    else
    {
      tool->IsReadyOff();
      this->WriteStatusMessage(QString("<b>STATUS: </b><i>Model not found. Starting download...</i>"));
    }
  }
  catch (const std::exception &e)
  {
    std::stringstream errorMsg;
    errorMsg << "<b>STATUS: </b>Error while processing parameters for SAM segmentation. Reason: " << e.what();
    this->ShowErrorMessage(errorMsg.str());
    this->WriteErrorMessage(QString::fromStdString(errorMsg.str()));
    m_Controls.activateButton->setEnabled(true);
    return;
  }
  catch (...)
  {
    std::string errorMsg = "Unkown error occured while generation SAM segmentation.";
    this->ShowErrorMessage(errorMsg);
    m_Controls.activateButton->setEnabled(true);
    return;
  }
}

void QmitkSegmentAnythingToolGUI::ActivateSAMDaemon()
{
  auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>();
  if (nullptr == tool)
  {
    return;
  }
  this->ShowProgressBar(true);
  tool->InitSAMPythonProcess();
  while (!tool->IsPythonReady())
  {
    qApp->processEvents();
  }
  tool->IsReadyOn();
  this->ShowProgressBar(false);
}

bool QmitkSegmentAnythingToolGUI::DownloadModel(const QString &modelType)
{
  QUrl url = QmitkSegmentAnythingToolGUI::VALID_MODELS_URL_MAP[modelType];
  QString modelFileName = url.fileName();
  const QString storageDir = QString::fromStdString(m_Prefences->Get("sam parent path", ""));
  QString checkPointPath = storageDir + QDir::separator() + modelFileName;
  if (QFile::exists(checkPointPath))
  {
    auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>();
    if (nullptr != tool)
    {
      tool->SetCheckpointPath(checkPointPath.toStdString());
    }
    return true;
  }
  connect(&m_Manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(FileDownloaded(QNetworkReply*)));
  QNetworkRequest request(url);
  m_Manager.get(request);
  this->ShowProgressBar(true);
  return false;
}

void QmitkSegmentAnythingToolGUI::FileDownloaded(QNetworkReply *reply)
{
  const QString storageDir = QString::fromStdString(m_Prefences->Get("sam parent path", ""));
  const QString &modelFileName = reply->url().fileName();
  QFile file(storageDir + QDir::separator() + modelFileName);
  if (file.open(QIODevice::WriteOnly))
  {
    file.write(reply->readAll());
    file.close();
    disconnect(&m_Manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(FileDownloaded(QNetworkReply *)));
    this->WriteStatusMessage(QString("<b>STATUS: </b><i>Model successfully downloaded. Activating SAM...."));
    auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>();
    if (nullptr != tool)
    {
      tool->SetCheckpointPath(file.fileName().toStdString());
      this->ActivateSAMDaemon();
      this->WriteStatusMessage(QString("<b>STATUS: </b><i>Model successfully downloaded. SAM Activated.</i>"));
    }
  }
  else
  {
    this->WriteErrorMessage("<b>STATUS: </b><i>Model couldn't be downloaded. SAM not activated.</i>");
  }
  this->EnableAll(true);
  this->ShowProgressBar(false);
}

void QmitkSegmentAnythingToolGUI::ShowProgressBar(bool enabled)
{
  m_Controls.samProgressBar->setEnabled(enabled);
  m_Controls.samProgressBar->setVisible(enabled);
}

bool QmitkSegmentAnythingToolGUI::IsSAMInstalled(const QString &pythonPath)
{
  QString fullPath = pythonPath;
  bool isPythonExists = false;
  bool isSamExists = false;
#ifdef _WIN32
  isPythonExists = QFile::exists(fullPath + QDir::separator() + QString("python.exe"));
  if (!(fullPath.endsWith("Scripts", Qt::CaseInsensitive) || fullPath.endsWith("Scripts/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("Scripts");
    isPythonExists =
      (!isPythonExists) ? QFile::exists(fullPath + QDir::separator() + QString("python.exe")) : isPythonExists;
  }
#else
  isPythonExists = QFile::exists(fullPath + QDir::separator() + QString("python3"));
  if (!(fullPath.endsWith("bin", Qt::CaseInsensitive) || fullPath.endsWith("bin/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("bin");
    isPythonExists =
      (!isPythonExists) ? QFile::exists(fullPath + QDir::separator() + QString("python3")) : isPythonExists;
  }
#endif
  isSamExists = QFile::exists(fullPath + QDir::separator() + QString("run_inference_daemon.py"));
  bool isExists = isSamExists && isPythonExists;
  return isExists;
}

void QmitkSegmentAnythingToolGUI::OnResetPicksClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>();
  if (nullptr != tool)
  {
    tool->ClearPicks();
  }
}
