/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentAnythingPreferencePage.h"

#include <mitkProcessExecutor.h>
#include <itkCommand.h>
#include <QmitkSegmentAnythingToolGUI.h>
#include "QmitkToolInstallDialog.h"

#include <QFileDialog>
#include <QmitkStyleManager.h>
#include <QDirIterator>
#include <QCursor>
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>
#include <QApplication>


namespace
{
  mitk::IPreferences *GetPreferences()
  {
    auto *preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }
}

QmitkSegmentAnythingPreferencePage::QmitkSegmentAnythingPreferencePage()
  : m_Ui(new Ui::QmitkSegmentAnythingPreferencePage), m_Control(nullptr){}

QmitkSegmentAnythingPreferencePage::~QmitkSegmentAnythingPreferencePage(){}

void QmitkSegmentAnythingPreferencePage::Init(berry::IWorkbench::Pointer){}

const QString QmitkSegmentAnythingPreferencePage::WARNING_PYTHON_NOT_FOUND =
  "Python is not detected in the selected path.\n"
  "Please select a path with a valid python install.";

void QmitkSegmentAnythingPreferencePage::CreateQtControl(QWidget *parent)
{
  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);

  m_Ui->samModelTipLabel->hide();
  m_Ui->timeoutEdit->setValidator(new QIntValidator(0, 1000, this));
  QIcon deleteIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/edit-delete.svg"));
  m_Ui->clearSAMButton->setIcon(deleteIcon);
  
  m_Ui->samModelTypeComboBox->addItems(VALID_MODELS);
  m_Ui->deviceComboBox->addItem(CPU_ID);
  m_Preferences = GetPreferences();
  if (nullptr == m_Preferences)
  {
    this->ShowErrorMessage("Error occurred while loading preferences.");
    return;
  }

  m_SysPythonPath = QString::fromStdString(m_Preferences->Get("sam systemPython path", ""));
  m_Ui->sysPythonComboBox->setDuplicatesEnabled(false);
  QStringList pythonDirs = QmitkSetupVirtualEnvUtil::AutoParsePythonPaths();
  m_Ui->sysPythonComboBox->addItems(pythonDirs);
  m_Ui->sysPythonComboBox->addItem("Select...");
  int sysPythonId = m_Ui->sysPythonComboBox->findText(m_SysPythonPath);
  if (!m_SysPythonPath.isEmpty() && sysPythonId == -1)
  {
    m_Ui->sysPythonComboBox->addItem(m_SysPythonPath);
    m_Ui->sysPythonComboBox->setCurrentIndex(m_Ui->sysPythonComboBox->count() - 1);
  }
  else
  {
    m_Ui->sysPythonComboBox->setCurrentIndex(sysPythonId);
  }
#ifndef _WIN32
  m_Ui->sysPythonComboBox->addItem("/usr/bin");
#endif
  QString installPath = QString::fromStdString(m_Preferences->Get("sam python path", ""));
  bool isInstalled = !installPath.isEmpty();
  QString welcomeText;
  if (isInstalled)
  {
    m_PythonPath = QmitkSetupVirtualEnvUtil::GetExactPythonPath(installPath).first;
    m_Installer.SetVirtualEnvPath(m_PythonPath);
    welcomeText += " Segment Anything is already found installed.";
    m_Ui->installSAMButton->setEnabled(false);
  }
  else
  {
    welcomeText += " Segment Anything not installed. Please click on \"Install SAM with MedSAM\" above. \
      The installation will create a new virtual environment using the System Python selected above.";
    m_Ui->installSAMButton->setEnabled(true);
  }

  connect(m_Ui->installSAMButton, SIGNAL(clicked()), this, SLOT(OnInstallBtnClicked()));
  connect(m_Ui->clearSAMButton, SIGNAL(clicked()), this, SLOT(OnClearInstall()));
  connect(m_Ui->sysPythonComboBox,
          QOverload<int>::of(&QComboBox::activated),
          [=](int index) { OnSystemPythonChanged(m_Ui->sysPythonComboBox->itemText(index)); });

  this->WriteStatusMessage(welcomeText);
  this->SetDeviceInfo();
  this->Update();
}

QWidget *QmitkSegmentAnythingPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkSegmentAnythingPreferencePage::PerformOk()
{
  m_Preferences->Put("sam systemPython path", m_SysPythonPath.toStdString());
  m_Preferences->Put("sam python path", m_PythonPath.toStdString());
  m_Preferences->Put("sam modeltype", m_Ui->samModelTypeComboBox->currentText().toStdString());
  m_Preferences->PutInt("sam deviceId", FetchSelectedDeviceFromUI());
  m_Preferences->PutInt("sam timeout", std::stoi(m_Ui->timeoutEdit->text().toStdString()));
  return true;
}

void QmitkSegmentAnythingPreferencePage::PerformCancel()
{
  m_Preferences->Put("sam python path", m_PythonPath.toStdString());
}

void QmitkSegmentAnythingPreferencePage::Update()
{
  m_Ui->samModelTypeComboBox->setCurrentText(QString::fromStdString(m_Preferences->Get("sam modeltype", "vit_b")));
  m_Ui->timeoutEdit->setText(QString::number(m_Preferences->GetInt("sam timeout", 300)));
  int gpuId = m_Preferences->GetInt("sam deviceId", -1);
  if (gpuId == -1)
  {
    m_Ui->deviceComboBox->setCurrentText(CPU_ID);
  }
  else if (m_GpuLoader.GetGPUCount() == 0)
  {
    m_Ui->deviceComboBox->setCurrentText(QString::number(gpuId));
  }
  else
  {
    std::vector<QmitkGPUSpec> specs = m_GpuLoader.GetAllGPUSpecs();
    QmitkGPUSpec gpuSpec = specs[gpuId];
    m_Ui->deviceComboBox->setCurrentText(QString::number(gpuSpec.id) + ": " + gpuSpec.name + " (" + gpuSpec.memory + ")");
  }
}

void QmitkSegmentAnythingPreferencePage::OnSystemPythonChanged(const QString &pyEnv)
{
  if (pyEnv == QString("Select..."))
  {
    QString path = QFileDialog::getExistingDirectory(m_Ui->sysPythonComboBox->parentWidget(), "Python Path", "dir");
    std::pair<QString, QString> pyPath;
    pyPath = QmitkSetupVirtualEnvUtil::GetExactPythonPath(this->GetPythonPathFromUI(path));
    if (!pyPath.first.isEmpty())
    {
      m_Ui->sysPythonComboBox->insertItem(0, path);
      m_Ui->sysPythonComboBox->setCurrentIndex(0);
      m_SysPythonPath = pyPath.first;
    }
    else
    {
      this->ShowErrorMessage(WARNING_PYTHON_NOT_FOUND);
    }
  }
  else
  {
    m_SysPythonPath = pyEnv;
  }
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

void QmitkSegmentAnythingPreferencePage::SetDeviceInfo()
{
  std::vector<QmitkGPUSpec> specs = m_GpuLoader.GetAllGPUSpecs();
  for (const QmitkGPUSpec &gpuSpec : specs)
  {
    m_Ui->deviceComboBox->addItem(QString::number(gpuSpec.id) + ": " + gpuSpec.name + " (" + gpuSpec.memory + ")");
  }
  if (specs.empty())
  {
    m_Ui->deviceComboBox->setCurrentIndex(m_Ui->deviceComboBox->findText("cpu"));
  }
  else
  {
    m_Ui->deviceComboBox->setCurrentIndex(m_Ui->deviceComboBox->count()-1);
  }
}

int QmitkSegmentAnythingPreferencePage::FetchSelectedDeviceFromUI() const
{
  QString gpuInfo = m_Ui->deviceComboBox->currentText();
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
    QString gpuId = gpuInfo.split(":", Qt::SkipEmptyParts).first();
    return static_cast<int>(gpuId.toInt());
  }
}

void QmitkSegmentAnythingPreferencePage::OnInstallBtnClicked()
{
  this->OnClearInstall(); // Clear any installation before
  QString uiPyPath = this->GetPythonPathFromUI(m_Ui->sysPythonComboBox->currentText());
  const auto [path, version] = QmitkSetupVirtualEnvUtil::GetExactPythonPath(uiPyPath);
  if (path.isEmpty())
  {
    this->WriteErrorMessage("<b>ERROR: </b>Couldn't find compatible Python.");
    return;
  }
  if (!QmitkSetupVirtualEnvUtil::IsVenvInstalled(path))
  {
    this->WriteErrorMessage("venv module not found for the selected python to create a new virtual " 
                            "environment. Please install venv or select another compatible python");
    return;
  }
  //check if python 3.12 and ask for confirmation
  if (version.startsWith("3.13") &&
       QMessageBox::No == QMessageBox::question(nullptr,
                            "Installing Segment Anything",
                            QString("WARNING: This is an unsupported version of Python that may not work. "
                                    "We recommend using a supported Python version between 3.9 and 3.12.\n\n"
                                    "Continue anyway?"),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No))
  {
    return;
  }

  //GUI
  this->WriteStatusMessage("<b>STATUS: </b>Installing Segment Anything...");
  m_Ui->installSAMButton->setEnabled(false);
  m_Installer.SetSystemPythonPath(path);
  QmitkToolInstallDialog *installDialog = new QmitkToolInstallDialog(m_Control, "Segment Anything");
  installDialog->show();

  //Start async process for installation
  QFuture<bool> future = QtConcurrent::run([&] {
      return m_Installer.SetupVirtualEnv(
        m_Installer.VENV_NAME,
        m_Installer.PACKAGES,
        [&]
        {
          std::string pythonCode; // python syntax to check if torch is installed with CUDA.
          pythonCode.append("import torch;");
          pythonCode.append("print('Segment Anything was installed with CUDA') if "
                            "torch.cuda.is_available() else print('Segment Anything was "
                            "installed WITHOUT CUDA');");
          m_Installer.ExecutePython(pythonCode, &QmitkSAMToolInstaller::PrintProcessEvent);
          return true;
        },
        &QmitkSAMToolInstaller::PrintProcessEvent);
      });
  while (future.isRunning())
  {
    qApp->processEvents();
    QThread::msleep(100);
  }
  if (future.result() && this->IsSAMInstalled(m_Installer.GetVirtualEnvPath()))
  {
    m_PythonPath = QmitkSetupVirtualEnvUtil::GetExactPythonPath(m_Installer.GetVirtualEnvPath()).first;
    this->WriteStatusMessage("<b>STATUS: </b>Successfully installed Segment Anything.");
    installDialog->FinishInstallation("Successfully installed Segment Anything.");
  }
  else
  {
    this->WriteErrorMessage("<b>ERROR: </b>Couldn't install Segment Anything.");
    m_Ui->installSAMButton->setEnabled(true);
    installDialog->FinishInstallation("Something went wrong while installing Segment Anything. Please try again.");
  }
}

void QmitkSegmentAnythingPreferencePage::OnClearInstall()
{
  QDir folderPath(m_Installer.GetVirtualEnvPath());
  QCursor cursor = m_Control->cursor();
  m_Control->setCursor(Qt::BusyCursor);
  if (folderPath.removeRecursively())
  {
    this->WriteStatusMessage("Deleted SAM installation.");
    m_Ui->installSAMButton->setEnabled(true);
    m_PythonPath = "";
  }
  else
  {
    MITK_ERROR << "The virtual environment couldn't be removed. Please check if you have the required access "
                  "privileges or, some other process is accessing the folders.";
  }
  m_Control->setCursor(cursor);
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

void QmitkSegmentAnythingPreferencePage::ShowErrorMessage(const QString &message, QMessageBox::Icon icon)
{
  QMessageBox *messageBox = new QMessageBox(icon, nullptr, message);
  messageBox->setAttribute(Qt::WA_DeleteOnClose, true);
  MITK_WARN << message;
  messageBox->exec();
}

bool QmitkSegmentAnythingPreferencePage::IsSAMInstalled(const QString &pythonPath)
{
  QString fullPath = pythonPath;
  bool hasPython = false;
  bool hasSam = false;
#ifdef _WIN32
  hasPython = QFile::exists(fullPath + QDir::separator() + QString("python.exe"));
  if (!(fullPath.endsWith("Scripts", Qt::CaseInsensitive) || fullPath.endsWith("Scripts/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("Scripts");
    hasPython = (!hasPython) ? QFile::exists(fullPath + QDir::separator() + QString("python.exe")) : hasPython;
  }
#else
  hasPython = QFile::exists(fullPath + QDir::separator() + QString("python3"));
  if (!(fullPath.endsWith("bin", Qt::CaseInsensitive) || fullPath.endsWith("bin/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("bin");
    hasPython = (!hasPython) ? QFile::exists(fullPath + QDir::separator() + QString("python3")) : hasPython;
  }
#endif
  hasSam = QFile::exists(fullPath + QDir::separator() + QString("run_inference_daemon.py")) &&
           QFile::exists(fullPath + QDir::separator() + QString("sam_runner.py")) &&
           QFile::exists(fullPath + QDir::separator() + QString("medsam_runner.py"));
  bool isInstalled = hasSam && hasPython;
  return isInstalled;
}
