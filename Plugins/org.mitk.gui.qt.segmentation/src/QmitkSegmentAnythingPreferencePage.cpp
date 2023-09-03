/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentAnythingPreferencePage.h"

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>
#include <mitkProcessExecutor.h>
#include <itkCommand.h>
#include <QmitkSegmentAnythingToolGUI.h>

#include <QFileDialog>
#include <QmitkStyleManager.h>
#include <QDir>
#include <QDirIterator>


namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }
}

QmitkSegmentAnythingPreferencePage::QmitkSegmentAnythingPreferencePage()
  : m_Ui(new Ui::QmitkSegmentAnythingPreferencePage),
    m_Control(nullptr){}

QmitkSegmentAnythingPreferencePage::~QmitkSegmentAnythingPreferencePage(){}

void QmitkSegmentAnythingPreferencePage::Init(berry::IWorkbench::Pointer){}

void QmitkSegmentAnythingPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);
#ifndef _WIN32
  m_Ui->sysPythonComboBox->addItem("/usr/bin");
#endif
  this->AutoParsePythonPaths();
  m_Ui->timeoutEdit->setValidator(new QIntValidator(0, 1000, this));
  m_Ui->sysPythonComboBox->addItem("Select...");
  m_Ui->sysPythonComboBox->setCurrentIndex(0);
  connect(m_Ui->installSAMButton, SIGNAL(clicked()), this, SLOT(OnInstallBtnClicked()));
  connect(m_Ui->clearSAMButton, SIGNAL(clicked()), this, SLOT(OnClearInstall()));
  connect(m_Ui->sysPythonComboBox,
          QOverload<int>::of(&QComboBox::activated),
          [=](int index) { OnSystemPythonChanged(m_Ui->sysPythonComboBox->itemText(index)); });
  QIcon deleteIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/edit-delete.svg"));
  m_Ui->clearSAMButton->setIcon(deleteIcon);
  const QString storageDir = m_Installer.GetVirtualEnvPath();
  bool isInstalled = QmitkSegmentAnythingToolGUI::IsSAMInstalled(storageDir);
  QString welcomeText;
  if (isInstalled)
  {
    m_PythonPath = GetExactPythonPath(storageDir);
    m_Installer.SetVirtualEnvPath(m_PythonPath);
    welcomeText += " Segment Anything tool is already found installed.";
    m_Ui->installSAMButton->setEnabled(false);
  }
  else
  {
    welcomeText += " Segment Anything tool not installed. Please click on \"Install SAM\" above. \
      The installation will create a new virtual environment using the System Python selected above.";
    m_Ui->installSAMButton->setEnabled(true);
  }
  this->WriteStatusMessage(welcomeText);
  m_Ui->samModelTypeComboBox->addItems(VALID_MODELS);
  m_Ui->gpuComboBox->addItem(CPU_ID);
  this->SetGPUInfo();
  this->Update();
}

QWidget* QmitkSegmentAnythingPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkSegmentAnythingPreferencePage::PerformOk()
{
  auto* prefs = GetPreferences();
  prefs->Put("sam parent path", m_Installer.STORAGE_DIR.toStdString());
  prefs->Put("sam python path", m_PythonPath.toStdString());
  prefs->Put("sam modeltype", m_Ui->samModelTypeComboBox->currentText().toStdString());
  prefs->PutInt("sam gpuid", FetchSelectedGPUFromUI());
  prefs->PutInt("sam timeout", std::stoi(m_Ui->timeoutEdit->text().toStdString()));
  return true;
}

void QmitkSegmentAnythingPreferencePage::PerformCancel(){}

void QmitkSegmentAnythingPreferencePage::Update()
{
  auto* prefs = GetPreferences();
  m_Ui->samModelTypeComboBox->setCurrentText(QString::fromStdString(prefs->Get("sam modeltype", "vit_b")));
  m_Ui->timeoutEdit->setText(QString::number(prefs->GetInt("sam timeout", 300)));
  int gpuId = prefs->GetInt("sam gpuid", -1);
  if (gpuId == -1)
  {
    m_Ui->gpuComboBox->setCurrentText(CPU_ID);
  }
  else if (m_GpuLoader.GetGPUCount() == 0)
  {
    m_Ui->gpuComboBox->setCurrentText(QString::number(gpuId));
  }
  else
  {
    std::vector<QmitkGPUSpec> specs = m_GpuLoader.GetAllGPUSpecs();
    QmitkGPUSpec gpuSpec = specs[gpuId];
    m_Ui->gpuComboBox->setCurrentText(QString::number(gpuSpec.id) + ": " + gpuSpec.name + " (" + gpuSpec.memory + ")");
  }
}

QString QmitkSegmentAnythingPreferencePage::OnSystemPythonChanged(const QString &pyEnv)
{
  QString pyPath;
  if (pyEnv == QString("Select..."))
  {
    QString path = QFileDialog::getExistingDirectory(m_Ui->sysPythonComboBox->parentWidget(), "Python Path", "dir");
    if (!path.isEmpty())
    {
      this->OnSystemPythonChanged(path);                           // recall same function for new path validation
      bool oldState = m_Ui->sysPythonComboBox->blockSignals(true); // block signal firing while inserting item
      m_Ui->sysPythonComboBox->insertItem(0, path);
      m_Ui->sysPythonComboBox->setCurrentIndex(0);
      m_Ui->sysPythonComboBox->blockSignals(oldState); // unblock signal firing after inserting item. Remove this after Qt6 migration
    }
  }
  else
  {
    QString uiPyPath = this->GetPythonPathFromUI(pyEnv);
    pyPath = this->GetExactPythonPath(uiPyPath);
  }
  return pyPath;
}

QString QmitkSegmentAnythingPreferencePage::GetPythonPathFromUI(const QString &pyUI) const
{
  QString fullPath = pyUI;
  if (-1 != fullPath.indexOf(")"))
  {
    fullPath = fullPath.mid(fullPath.indexOf(")") + 2);
  }
  return fullPath.simplified();
}

QString QmitkSegmentAnythingPreferencePage::GetExactPythonPath(const QString &pyEnv) const
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

void QmitkSegmentAnythingPreferencePage::AutoParsePythonPaths()
{
  QString homeDir = QDir::homePath();
  std::vector<QString> searchDirs;
#ifdef _WIN32
  searchDirs.push_back(QString("C:") + QDir::separator() + QString("ProgramData") + QDir::separator() +
                       QString("anaconda3"));
#else
  // Add search locations for possible standard python paths here
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
        m_Ui->sysPythonComboBox->addItem("(base): " + searchDir);
        searchDir.append((QDir::separator() + QString("envs")));
      }
    }
    for (QDirIterator subIt(searchDir, QDir::AllDirs, QDirIterator::NoIteratorFlags); subIt.hasNext();)
    {
      subIt.next();
      QString envName = subIt.fileName();
      if (!envName.startsWith('.')) // Filter out irrelevent hidden folders, if any.
      {
        m_Ui->sysPythonComboBox->addItem("(" + envName + "): " + subIt.filePath());
      }
    }
  }
}

void QmitkSegmentAnythingPreferencePage::SetGPUInfo()
{
  std::vector<QmitkGPUSpec> specs = m_GpuLoader.GetAllGPUSpecs();
  for (const QmitkGPUSpec &gpuSpec : specs)
  {
    m_Ui->gpuComboBox->addItem(QString::number(gpuSpec.id) + ": " + gpuSpec.name + " (" + gpuSpec.memory + ")");
  }
  if (specs.empty())
  {
    m_Ui->gpuComboBox->setCurrentIndex(m_Ui->gpuComboBox->findText("cpu"));
  }
  else
  {
    m_Ui->gpuComboBox->setCurrentIndex(m_Ui->gpuComboBox->count()-1);
  }
}

int QmitkSegmentAnythingPreferencePage::FetchSelectedGPUFromUI() const
{
  QString gpuInfo = m_Ui->gpuComboBox->currentText();
  if ("cpu" == gpuInfo)
  {
    return -1;
  }
  else if(m_GpuLoader.GetGPUCount() == 0)
  {
    return static_cast<int>(gpuInfo.toInt());
  }
  else
  {
    QString gpuId = gpuInfo.split(":", QString::SplitBehavior::SkipEmptyParts).first();
    return static_cast<int>(gpuId.toInt());
  }
}

void QmitkSegmentAnythingPreferencePage::OnInstallBtnClicked()
{
  QString systemPython = OnSystemPythonChanged(m_Ui->sysPythonComboBox->currentText());
  if (!systemPython.isEmpty())
  {
    this->WriteStatusMessage("<b>STATUS: </b>Installing SAM...");
    m_Ui->installSAMButton->setEnabled(false);
    m_Installer.SetSystemPythonPath(systemPython);
    bool isInstalled = false;
    bool isFinished = m_Installer.SetupVirtualEnv(m_Installer.VENV_NAME);
    if (isFinished)
    {
      isInstalled = QmitkSegmentAnythingToolGUI::IsSAMInstalled(m_Installer.GetVirtualEnvPath());
    }
    if (isInstalled)
    {
      m_PythonPath = this->GetExactPythonPath(m_Installer.GetVirtualEnvPath());
      this->WriteStatusMessage("<b>STATUS: </b>Successfully installed SAM.");
    }
    else
    {
      this->WriteErrorMessage("<b>ERROR: </b>Couldn't install SAM.");
      m_Ui->installSAMButton->setEnabled(true);
    }
  }
}

void QmitkSegmentAnythingPreferencePage::OnClearInstall()
{
  QDir folderPath(m_Installer.GetVirtualEnvPath());
  bool isDeleted = folderPath.removeRecursively();
  if (isDeleted)
  {
    this->WriteStatusMessage("Deleted SAM installation.");
    m_Ui->installSAMButton->setEnabled(true);
    m_PythonPath.clear();
  }
  else
  {
    MITK_ERROR << "The virtual environment couldn't be removed. Please check if you have the required access "
                  "privileges or, some other process is accessing the folders.";
  }
}

void QmitkSegmentAnythingPreferencePage::WriteStatusMessage(const QString &message)
{
  m_Ui->samInstallStatusLabel->setText(message);
  m_Ui->samInstallStatusLabel->setStyleSheet("font-weight: bold; color: white");
  qApp->processEvents();
}

void QmitkSegmentAnythingPreferencePage::WriteErrorMessage(const QString &message)
{
  m_Ui->samInstallStatusLabel->setText(message);
  m_Ui->samInstallStatusLabel->setStyleSheet("font-weight: bold; color: red");
  qApp->processEvents();
}

QString QmitkSAMInstaller::GetVirtualEnvPath()
{
  return STORAGE_DIR + VENV_NAME;
}

bool QmitkSAMInstaller::SetupVirtualEnv(const QString &venvName)
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
