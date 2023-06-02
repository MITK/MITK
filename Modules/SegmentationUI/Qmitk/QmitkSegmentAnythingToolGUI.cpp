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


MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkSegmentAnythingToolGUI, "")

QmitkSegmentAnythingToolGUI::QmitkSegmentAnythingToolGUI() : QmitkSegWithPreviewToolGUIBase(true)
{
  // Nvidia-smi command returning zero doesn't always imply lack of GPUs.
  // Pytorch uses its own libraries to communicate to the GPUs. Hence, only a warning can be given.
  if (m_GpuLoader.GetGPUCount() == 0)
  {
    std::string warning = "WARNING: No GPUs were detected on your machine. The TotalSegmentator tool can be very slow.";
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
}

void QmitkSegmentAnythingToolGUI::InitializeUI(QBoxLayout *mainLayout)
{
  m_Controls.setupUi(this);
#ifndef _WIN32
  m_Controls.sysPythonComboBox->addItem("/usr/bin");
#endif
  this->AutoParsePythonPaths();
  m_Controls.sysPythonComboBox->addItem("Select...");
  m_Controls.sysPythonComboBox->setCurrentIndex(0);
  m_Controls.statusLabel->setTextFormat(Qt::RichText);
  m_Controls.modelTypeComboBox->addItems(VALID_MODELS_URL_MAP.keys());

  QString welcomeText;
  this->SetGPUInfo();
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
  connect(m_Controls.installButton, SIGNAL(clicked()), this, SLOT(OnInstallBtnClicked()));
  connect(m_Controls.clearButton, SIGNAL(clicked()), this, SLOT(OnClearInstall()));
  connect(m_Controls.sysPythonComboBox,
          QOverload<int>::of(&QComboBox::activated),
          [=](int index) { OnSystemPythonChanged(m_Controls.sysPythonComboBox->itemText(index)); });
  connect(m_Controls.gpuComboBox, SIGNAL(currentTextChanged(const QString &)), this,
          SLOT(OnParametersChanged(const QString &)));
  connect(m_Controls.modelTypeComboBox, SIGNAL(currentTextChanged(const QString &)), this,
          SLOT(OnParametersChanged(const QString &)));

  QIcon deleteIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/edit-delete.svg"));
  QIcon arrowIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/go-next.svg"));
  m_Controls.clearButton->setIcon(deleteIcon);
  m_Controls.activateButton->setIcon(arrowIcon);

  const QString storageDir = m_Installer.GetVirtualEnvPath();
  m_IsInstalled = this->IsSAMInstalled(storageDir);
  if (m_IsInstalled)
  {
    m_PythonPath = GetExactPythonPath(storageDir);
    m_Installer.SetVirtualEnvPath(m_PythonPath);
    welcomeText += " SAM is already found installed.";
  }
  else
  {
    welcomeText += " SAM is not installed. Please click on \"Install SAM\" above.";
  }
  this->EnableAll(m_IsInstalled);
  this->WriteStatusMessage(welcomeText);
  this->ShowProgressBar(false);
  m_Controls.samProgressBar->setMaximum(0);

  mainLayout->addLayout(m_Controls.verticalLayout);
  Superclass::InitializeUI(mainLayout);
}

QString QmitkSegmentAnythingToolGUI::GetExactPythonPath(const QString &pyEnv) const
{
  QString fullPath = pyEnv;
  bool isPythonExists = false;
#ifdef _WIN32
  isPythonExists = QFile::exists(fullPath + QDir::separator() + QString("python.exe"));
  if (!isPythonExists &&
      !(fullPath.endsWith("Scripts", Qt::CaseInsensitive) || fullPath.endsWith("Scripts/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("Scripts");
    isPythonExists = QFile::exists(fullPath + QDir::separator() + QString("python.exe"));
  }
#else
  isPythonExists = QFile::exists(fullPath + QDir::separator() + QString("python3"));
  if (!isPythonExists &&
      !(fullPath.endsWith("bin", Qt::CaseInsensitive) || fullPath.endsWith("bin/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("bin");
    isPythonExists = QFile::exists(fullPath + QDir::separator() + QString("python3"));
  }
#endif
  if (!isPythonExists)
  {
    fullPath.clear();
  }
  return fullPath;
}

void QmitkSegmentAnythingToolGUI::EnableAll(bool isEnable)
{
  m_Controls.activateButton->setEnabled(isEnable);
  m_Controls.installButton->setEnabled((!isEnable));
}

void QmitkSegmentAnythingToolGUI::SetGPUInfo()
{
  std::vector<QmitkGPUSpec> specs = m_GpuLoader.GetAllGPUSpecs();
  for (const QmitkGPUSpec &gpuSpec : specs)
  {
    m_Controls.gpuComboBox->addItem(QString::number(gpuSpec.id) + ": " + gpuSpec.name + " (" + gpuSpec.memory + ")");
  }
  if (specs.empty())
  {
    m_Controls.gpuComboBox->setEditable(true);
    m_Controls.gpuComboBox->addItem(QString::number(0));
    m_Controls.gpuComboBox->setValidator(new QIntValidator(0, 999, this));
  }
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

void QmitkSegmentAnythingToolGUI::AutoParsePythonPaths()
{
  QString homeDir = QDir::homePath();
  std::vector<QString> searchDirs;
#ifdef _WIN32
  searchDirs.push_back(QString("C:") + QDir::separator() + QString("ProgramData") + QDir::separator() +
                       QString("anaconda3"));
#else
  // Add search locations for possible standard python paths here
  searchDirs.push_back(homeDir + QDir::separator() + "environments");
  searchDirs.push_back(homeDir + QDir::separator() + "anaconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "miniconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "opt" + QDir::separator() + "miniconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "opt" + QDir::separator() + "anaconda3");
#endif
  for (QString searchDir : searchDirs)
  {
    if (searchDir.endsWith("anaconda3", Qt::CaseInsensitive))
    {
      if (QDir(searchDir).exists())
      {
        m_Controls.sysPythonComboBox->addItem("(base): " + searchDir);
        searchDir.append((QDir::separator() + QString("envs")));
      }
    }
    for (QDirIterator subIt(searchDir, QDir::AllDirs, QDirIterator::NoIteratorFlags); subIt.hasNext();)
    {
      subIt.next();
      QString envName = subIt.fileName();
      if (!envName.startsWith('.')) // Filter out irrelevent hidden folders, if any.
      {
        m_Controls.sysPythonComboBox->addItem("(" + envName + "): " + subIt.filePath());
      }
    }
  }
}

unsigned int QmitkSegmentAnythingToolGUI::FetchSelectedGPUFromUI() const
{
  QString gpuInfo = m_Controls.gpuComboBox->currentText();
  if (m_GpuLoader.GetGPUCount() == 0)
  {
    return static_cast<unsigned int>(gpuInfo.toInt());
  }
  else
  {
    QString gpuId = gpuInfo.split(":", QString::SplitBehavior::SkipEmptyParts).first();
    return static_cast<unsigned int>(gpuId.toInt());
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
    m_Controls.activateButton->setEnabled(false);
    qApp->processEvents();
    if (!this->IsSAMInstalled(m_PythonPath))
    {
      throw std::runtime_error(WARNING_SAM_NOT_FOUND);
    }
    tool->SetIsAuto(false);
    tool->SetPythonPath(m_PythonPath.toStdString());
    tool->SetGpuId(FetchSelectedGPUFromUI());
    QString modelType = m_Controls.modelTypeComboBox->currentText();    
    tool->SetModelType(modelType.toStdString());
    this->WriteStatusMessage(
      QString("<b>STATUS: </b><i>Checking if model is already downloaded... This might take a while.</i>"));
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
  m_Controls.activateButton->setEnabled(true);
  this->ShowProgressBar(false);
}

QString QmitkSegmentAnythingToolGUI::GetPythonPathFromUI(const QString &pyUI) const
{
  QString fullPath = pyUI;
  if (-1 != fullPath.indexOf(")"))
  {
    fullPath = fullPath.mid(fullPath.indexOf(")") + 2);
  }
  return fullPath.simplified();
}

bool QmitkSegmentAnythingToolGUI::DownloadModel(const QString &modelType)
{
  QUrl url = VALID_MODELS_URL_MAP[modelType];
  QString modelFileName = url.fileName();
  const QString& storageDir = m_Installer.STORAGE_DIR;
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
  const QString &storageDir = m_Installer.STORAGE_DIR;
  const QString &modelFileName = reply->url().fileName();
  QFile file(storageDir + QDir::separator() + modelFileName);
  if (file.open(QIODevice::WriteOnly))
  {
    file.write(reply->readAll());
    file.close();
    disconnect(&m_Manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(FileDownloaded(QNetworkReply *)));
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

QString QmitkSegmentAnythingToolGUI::OnSystemPythonChanged(const QString &pyEnv)
{
  QString pyPath;
  if (pyEnv == QString("Select..."))
  {
    m_Controls.activateButton->setDisabled(true);
    QString path =
      QFileDialog::getExistingDirectory(m_Controls.sysPythonComboBox->parentWidget(), "Python Path", "dir");
    if (!path.isEmpty())
    {
      this->OnSystemPythonChanged(path);                                // recall same function for new path validation
      bool oldState = m_Controls.sysPythonComboBox->blockSignals(true); // block signal firing while inserting item
      m_Controls.sysPythonComboBox->insertItem(0, path);
      m_Controls.sysPythonComboBox->setCurrentIndex(0);
      m_Controls.sysPythonComboBox->blockSignals(
        oldState); // unblock signal firing after inserting item. Remove this after Qt6 migration
    }
  }
  else
  {
    QString uiPyPath = this->GetPythonPathFromUI(pyEnv);
    pyPath = this->GetExactPythonPath(uiPyPath);
  }
  return pyPath;
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
  bool isExists = /*QFile::exists(fullPath + QDir::separator() + QString("MITK_SAM"))*/ true && isPythonExists;
  return isExists;
}

void QmitkSegmentAnythingToolGUI::OnParametersChanged(const QString &)
{
  if (m_Controls.activateButton->isEnabled())
  {
    this->WriteStatusMessage("<b>STATUS: </b><i>Please Reactivate SAM.</i>");
  }
}

void QmitkSegmentAnythingToolGUI::OnResetPicksClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::SegmentAnythingTool>();
  if (nullptr != tool)
  {
    tool->ClearPicks();
  }
}

void QmitkSegmentAnythingToolGUI::OnInstallBtnClicked()
{
  bool isInstalled = false;
  QString systemPython = OnSystemPythonChanged(m_Controls.sysPythonComboBox->currentText());
  if (systemPython.isEmpty())
  {
    this->WriteErrorMessage("<b>ERROR: </b>Couldn't find Python.");
  }
  else
  {
    this->WriteStatusMessage("<b>STATUS: </b>Installing SAM...");
    m_Installer.SetSystemPythonPath(systemPython);
    isInstalled = m_Installer.SetupVirtualEnv(m_Installer.VENV_NAME);
    if (isInstalled)
    {
      m_PythonPath = this->GetExactPythonPath(m_Installer.GetVirtualEnvPath());
      this->WriteStatusMessage("<b>STATUS: </b>Successfully installed SAM.");
    }
    else
    {
      this->WriteErrorMessage("<b>ERROR: </b>Couldn't install SAM.");
    }
  }
  this->EnableAll(isInstalled);
}

void QmitkSegmentAnythingToolGUI::OnClearInstall()
{
  QDir folderPath(m_Installer.GetVirtualEnvPath());
  if (folderPath.removeRecursively())
  {
    m_Controls.installButton->setEnabled(true);
    m_IsInstalled = false;
  }
  else
  {
    MITK_ERROR << "The virtual environment couldn't be removed. Please check if you have the required access "
                  "privileges or, some other process is accessing the folders.";
  }
  this->EnableAll(m_IsInstalled);
}

bool QmitkSegmentAnythingToolInstaller::SetupVirtualEnv(const QString &venvName)
{
  if (GetSystemPythonPath().isEmpty())
  {
    return false;
  }
  QDir folderPath(GetBaseDir());
  folderPath.mkdir(venvName);
  if (!folderPath.cd(venvName))
  {
    return false; // Check if directory creation was successful.
  }
  mitk::ProcessExecutor::ArgumentListType args;
  auto spExec = mitk::ProcessExecutor::New();
  auto spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&PrintProcessEvent);
  spExec->AddObserver(mitk::ExternalProcessOutputEvent(), spCommand);

  args.push_back("-m");
  args.push_back("venv");
  args.push_back(venvName.toStdString());
#ifdef _WIN32
  QString pythonFile = GetSystemPythonPath() + QDir::separator() + "python.exe";
  QString pythonExeFolder = "Scripts";
#else
  QString pythonFile = GetSystemPythonPath() + QDir::separator() + "python3";
  QString pythonExeFolder = "bin";
#endif
  spExec->Execute(GetBaseDir().toStdString(), pythonFile.toStdString(), args); // Setup local virtual environment
  if (folderPath.cd(pythonExeFolder))
  {
    this->SetPythonPath(folderPath.absolutePath());
    this->SetPipPath(folderPath.absolutePath());
    this->InstallPytorch();
    for (auto &package : PACKAGES)
    {
      this->PipInstall(package.toStdString(), &PrintProcessEvent);
    }
    std::string pythonCode; // python syntax to check if torch is installed with CUDA.
    pythonCode.append("import torch;");
    pythonCode.append("print('Pytorch was installed with CUDA') if torch.cuda.is_available() else print('PyTorch was "
                      "installed WITHOUT CUDA');");
    this->ExecutePython(pythonCode, &PrintProcessEvent);
    return true;
  }
  return false;
}

QString QmitkSegmentAnythingToolInstaller::GetVirtualEnvPath()
{
  return STORAGE_DIR + VENV_NAME;
}
