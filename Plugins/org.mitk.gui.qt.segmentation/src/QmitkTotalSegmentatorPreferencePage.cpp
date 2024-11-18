/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTotalSegmentatorPreferencePage.h"
#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <QApplication>
#include <QDirIterator>
#include <QFileDialog>
#include "QmitkStyleManager.h"
#include "QmitkToolInstallDialog.h"
#include <QtConcurrent/QtConcurrentRun>


namespace
{
  mitk::IPreferences *GetPreferences()
  {
    auto *preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }
}

void QmitkTotalSegmentatorPreferencePage::Init(berry::IWorkbench::Pointer) {}
const QString QmitkTotalSegmentatorPreferencePage::WARNING_TOTALSEG_NOT_FOUND =
  "TotalSegmentator is not detected in the selected python environment. Please select a valid "
  "python environment or install TotalSegmentator.";

QmitkTotalSegmentatorPreferencePage::QmitkTotalSegmentatorPreferencePage()
  : m_Ui(new Ui::QmitkTotalSegmentatorPreferencePage), m_Control(nullptr){}

void QmitkTotalSegmentatorPreferencePage::CreateQtControl(QWidget *parent)
{
  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);
  QIcon deleteIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/edit-delete.svg"));
  m_Ui->clearTotalbutton->setIcon(deleteIcon);
  m_Preferences = GetPreferences();
  if (m_Preferences == nullptr)
  {
    this->ShowErrorMessage("Error occurred while loading preferences");
    return;
  }
  this->AutoParsePythonPaths();
  this->SetDeviceInfo();
  m_Ui->overrideBox->setChecked(m_Preferences->GetBool("TotalSeg/isCustomInstall", false));
  bool isCustomInstall = m_Ui->overrideBox->isChecked();
  m_Ui->statusLabel->setWordWrap(true);
  m_Ui->sysPythonComboBox->setDuplicatesEnabled(false);
  m_Ui->customEnvComboBox->setDuplicatesEnabled(false);
  m_Ui->customEnvComboBox->setEnabled(isCustomInstall);
  QString m_CustomInstallPath = QString::fromStdString(m_Preferences->Get("TotalSeg/customInstallPath", ""));
  m_SysPythonPath = QString::fromStdString(m_Preferences->Get("TotalSeg/sysPythonPath", ""));
  if (!m_SysPythonPath.isEmpty() && m_Ui->sysPythonComboBox->findText(m_SysPythonPath) == -1)
  {
    m_Ui->sysPythonComboBox->addItem(m_SysPythonPath);
  }
#ifndef _WIN32
  m_Ui->sysPythonComboBox->addItem("/usr/bin");
#endif
  m_Ui->sysPythonComboBox->addItem("Select...");
  m_Ui->sysPythonComboBox->setCurrentIndex(0);

  if (!(m_CustomInstallPath.isEmpty()) && m_CustomInstallPath != "Select...")
  {
    m_Ui->customEnvComboBox->addItem(m_CustomInstallPath);
    m_Ui->customEnvComboBox->setCurrentIndex(0);
    m_Ui->customEnvComboBox->addItem("Select...");
  }
  else
  {
    m_Ui->customEnvComboBox->addItem("Select...");
    m_Ui->customEnvComboBox->setCurrentIndex(0);
  }
  if (this->IsTotalSegmentatorInstalled(m_Installer.GetVirtualEnvPath()))
  {
    m_Ui->installTotalButton->setEnabled(false);
    m_IsInstalled = true;
  }
  else
  {
    m_Ui->installTotalButton->setEnabled(true);
    m_IsInstalled = false;
  }

  connect(m_Ui->installTotalButton, SIGNAL(clicked()), this, SLOT(OnInstallButtonClicked()));
  connect(m_Ui->clearTotalbutton, SIGNAL(clicked()), this, SLOT(OnClearButtonClicked()));
  connect(m_Ui->overrideBox, SIGNAL(stateChanged(int)), this, SLOT(OnOverrideBoxChecked(int)));
  connect(m_Ui->customEnvComboBox,
          QOverload<int>::of(&QComboBox::activated),
          [=](int index) { OnPythonPathChanged(m_Ui->customEnvComboBox->itemText(index)); });
  connect(m_Ui->sysPythonComboBox,
          QOverload<int>::of(&QComboBox::activated),
          [=](int index) { OnSystemPythonChanged(m_Ui->sysPythonComboBox->itemText(index)); });

  this->UpdateStatusLabel();
}

QWidget *QmitkTotalSegmentatorPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkTotalSegmentatorPreferencePage::PerformOk()
{
  if (nullptr == m_Preferences)
  {
    this->ShowErrorMessage("Error occurred while saving preferences");
    return false;
  }
  m_Preferences->Put("TotalSeg/customInstallPath", m_Ui->customEnvComboBox->currentText().toStdString());
  m_Preferences->Put("TotalSeg/sysPythonPath", m_SysPythonPath.toStdString());
  m_Preferences->PutBool("TotalSeg/isCustomInstall", m_Ui->overrideBox->isChecked());
  m_Preferences->PutInt("TotalSeg/deviceId", this->FetchSelectedDeviceFromUI());
  this->UpdateTotalSegPreferencePath();
  return true;
}

void QmitkTotalSegmentatorPreferencePage::PerformCancel()
{
  this->UpdateTotalSegPreferencePath();
}

void QmitkTotalSegmentatorPreferencePage::Update(){}

void QmitkTotalSegmentatorPreferencePage::OnInstallButtonClicked()
{
  m_Ui->installTotalButton->setDisabled(true);
  QString pyEnv = m_SysPythonPath;
  QString uiPyPath = this->GetPythonPathFromUI(pyEnv);
  const auto [path, version] = QmitkSetupVirtualEnvUtil::GetExactPythonPath(uiPyPath);
  if (path.isEmpty())
  {
    this->WriteErrorMessage("<b>ERROR: </b>Couldn't find compatible Python.");
    m_Ui->installTotalButton->setDisabled(false);
    return;
  }
  if (!QmitkSetupVirtualEnvUtil::IsVenvInstalled(path))
  {
    this->WriteErrorMessage("venv module not detected for the selected python to create a new virtual "
                            "environment. Please install venv or select another compatibile python");
    m_Ui->installTotalButton->setDisabled(false);
    return;
  }
  // check if python 3.13 and ask for confirmation
  if (version.startsWith("3.13") &&
      QMessageBox::No ==
        QMessageBox::question(nullptr,
                              "Installing TotalSegmentator",
                              QString("WARNING: This is an unsupported version of Python that may not work. "
                                      "We recommend using a supported Python version between 3.9 and 3.12.\n\n"
                                      "Continue anyway?"),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No))
  {
    m_Ui->installTotalButton->setDisabled(false);
    return;
  }
  m_Installer.SetSystemPythonPath(path);

  //GUI
  this->WriteStatusMessage("<b>STATUS: </b>Installing TotalSegmentator...");
  QmitkToolInstallDialog *installDialog = new QmitkToolInstallDialog(m_Control);
  installDialog->show();

  //Start async process for installation
  QFuture<bool> future = QtConcurrent::run([&] { return m_Installer.SetupVirtualEnv(m_Installer.VENV_NAME); });
  while (future.isRunning())
  {
    qApp->processEvents();
    QThread::msleep(100);
  }
  QString installPath = QmitkSetupVirtualEnvUtil::GetExactPythonPath(m_Installer.GetVirtualEnvPath()).first;
  if (future.result() && this->IsTotalSegmentatorInstalled(installPath))
  {
    this->WriteStatusMessage("<b>STATUS: </b>Successfully installed TotalSegmentator.");
    installDialog->m_Ui->statusLabel->setText("TotalSegmentator was succesfully installed");
    m_IsInstalled = true;
    m_Installer.SetVirtualEnvPath(installPath);
  }
  else
  {
    this->WriteErrorMessage("<b>ERROR: </b>Couldn't install TotalSegmentator.");
    installDialog->m_Ui->statusLabel->setText("Couldn't install TotalSegmentator");
    m_Ui->installTotalButton->setDisabled(false);
    this->WriteStatusMessage("<b>ERROR: </b>Couldn't install TotalSegmentator.");
  }
  installDialog->m_IsInstalling = false;
  installDialog->m_Ui->progressBar->setVisible(false);
  installDialog->m_Ui->closePushButton->setEnabled(true);
  this->UpdateStatusLabel();
}

void QmitkTotalSegmentatorPreferencePage::OnClearButtonClicked()
{
  QDir folderPath(m_Installer.GetVirtualEnvPath());
  QCursor cursor = m_Control->cursor();
  m_Control->setCursor(Qt::BusyCursor);
  if (folderPath.removeRecursively())
  {
    m_Ui->installTotalButton->setEnabled(true);
    m_IsInstalled = false;
    this->WriteStatusMessage("<b>STATUS: </b>Successfully uninstalled TotalSegmentator.");
  }
  else
  {
    m_Ui->installTotalButton->setEnabled(false);
    MITK_ERROR << "The virtual environment couldn't be removed. Please check if you have the required access "
                  "privileges or, some other process is accessing the folders.";
  }
  m_Control->setCursor(cursor);
}

void QmitkTotalSegmentatorPreferencePage::OnOverrideBoxChecked(int state)
{
  if (state)
  {
    m_Ui->customEnvComboBox->setEnabled(true);
  }
  else
  {
    m_Ui->customEnvComboBox->setEnabled(false);
  }
  this->UpdateStatusLabel();
}

void QmitkTotalSegmentatorPreferencePage::OnPythonPathChanged(const QString &pyEnv)
{
  if (pyEnv == QString("Select..."))
  {
    QString path = QFileDialog::getExistingDirectory(m_Ui->customEnvComboBox->parentWidget(), "Python Path", "dir");
    if (!path.isEmpty())
    {
      this->OnPythonPathChanged(path); // recall same function for new path validation
      m_Ui->customEnvComboBox->insertItem(0, path);
      m_Ui->customEnvComboBox->setCurrentIndex(0);
    }
  }
  else if (this->IsTotalSegmentatorInstalled(this->GetPythonPathFromUI(pyEnv)))
  {
    QString uiPyPath = this->GetPythonPathFromUI(pyEnv);
  }
  else
  {
    this->ShowErrorMessage(WARNING_TOTALSEG_NOT_FOUND);
  }
  this->UpdateStatusLabel();
}

std::pair<QString, QString> QmitkTotalSegmentatorPreferencePage::OnSystemPythonChanged(const QString &pyEnv)
{
  std::pair<QString, QString> pyPath;
  if (pyEnv == QString("Select..."))
  {
    QString path = QFileDialog::getExistingDirectory(m_Ui->sysPythonComboBox->parentWidget(), "Python Path", "dir");
    if (!path.isEmpty())
    {
      this->OnSystemPythonChanged(path);// recall same function for new path validation
      m_Ui->sysPythonComboBox->insertItem(0, path);
      m_Ui->sysPythonComboBox->setCurrentIndex(0);
    }
  }
  else
  {
    QString uiPyPath = this->GetPythonPathFromUI(pyEnv);
    pyPath = QmitkSetupVirtualEnvUtil::GetExactPythonPath(uiPyPath);
    m_SysPythonPath = pyPath.first;
  }
  return pyPath;
}

void QmitkTotalSegmentatorPreferencePage::UpdateStatusLabel()
{
  QString text;
  if (m_Ui->overrideBox->isChecked())
  {
    if (this->IsTotalSegmentatorInstalled(this->GetPythonPathFromUI(m_Ui->customEnvComboBox->currentText())))
    {
      text += "Custom installation of TotalSegmentator was detected.\n";
    }
    else
    {
      text += "No TotalSegmentator installation detected at this location.\n";
    }
  }
  else
  {
    if (this->IsTotalSegmentatorInstalled(m_Installer.GetVirtualEnvPath()))
    {
      text += "TotalSegmentator was found installed.\n";
    }
    else
    {
      text += "Compatible version of TotalSegmentator is not detected. Please 'Install TotalSegmentator'"
              " or use your own custom virtual environment.\n";
    }
  }
  if (m_DeviceLoader.GetGPUCount() == 0)
  {
    text += "WARNING: No GPUs were detected on your machine. The TotalSegmentator tool can be very slow.";
  } 
  else
  {
    text += "Your in luck. " + QString::number(m_DeviceLoader.GetGPUCount()) + " GPU(s) were found";
  }
  m_Ui->statusLabel->setText(text);
}

bool QmitkTotalSegmentatorPreferencePage::IsTotalSegmentatorInstalled(const QString &pythonPath)
{
  QString fullPath = pythonPath;
  bool isPythonExists = false;
  bool isExists = false;
#ifdef _WIN32
  isPythonExists = QFile::exists(fullPath + QDir::separator() + QString("python.exe"));
  if (!(fullPath.endsWith("Scripts", Qt::CaseInsensitive) || fullPath.endsWith("Scripts/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("Scripts");
    isPythonExists =
      (!isPythonExists) ? QFile::exists(fullPath + QDir::separator() + QString("python.exe")) : isPythonExists;
  }
  isExists = QFile::exists(fullPath + QDir::separator() + QString("TotalSegmentator.exe")) && isPythonExists;
#else
  isPythonExists = QFile::exists(fullPath + QDir::separator() + QString("python3"));
  if (!(fullPath.endsWith("bin", Qt::CaseInsensitive) || fullPath.endsWith("bin/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("bin");
    isPythonExists =
      (!isPythonExists) ? QFile::exists(fullPath + QDir::separator() + QString("python3")) : isPythonExists;
  }
  isExists = QFile::exists(fullPath + QDir::separator() + QString("TotalSegmentator")) && isPythonExists;
#endif
  if (isExists && m_Installer.TOTALSEGMENTATOR_VERSION !=
                    QmitkSetupVirtualEnvUtil::GetPipPackageVersion(fullPath, "TotalSegmentator"))
  {
    isExists = false;
  }
  return isExists;
}

QString QmitkTotalSegmentatorPreferencePage::GetPythonPathFromUI(const QString &pyUI) const
{
  QString fullPath = pyUI;
  if (-1 != fullPath.indexOf(")"))
  {
    fullPath = fullPath.mid(fullPath.indexOf(")") + 2);
  }
  return fullPath.simplified();
}

void QmitkTotalSegmentatorPreferencePage::AutoParsePythonPaths()
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
      if (!envName.startsWith('.')) // Filter out irrelevant hidden folders, if any.
      {
        m_Ui->sysPythonComboBox->addItem("(" + envName + "): " + subIt.filePath());
      }
    }
  }
}

void QmitkTotalSegmentatorPreferencePage::SetDeviceInfo()
{
  std::vector<QmitkGPUSpec> specs = m_DeviceLoader.GetAllGPUSpecs();
  if (specs.empty())
  {
    m_Ui->deviceComboBox->setEditable(true);
  }
  for (const QmitkGPUSpec &deviceSpec : specs)
  {
    m_Ui->deviceComboBox->addItem(QString::number(deviceSpec.id) + ": " + deviceSpec.name + " (" + deviceSpec.memory + ")");
  }
  m_Ui->deviceComboBox->addItem(QString::fromStdString("cpu"));
  int lastSelectedDeviceId = m_Preferences->GetInt("TotalSeg/deviceId", -1);
  if (lastSelectedDeviceId == -1)
  {
    m_Ui->deviceComboBox->setCurrentText("cpu");
  }
  else
  {
    m_Ui->deviceComboBox->setCurrentIndex(lastSelectedDeviceId);
  }
}

int QmitkTotalSegmentatorPreferencePage::FetchSelectedDeviceFromUI() const
{
  QString deviceInfo = m_Ui->deviceComboBox->currentText();
  if (m_DeviceLoader.GetGPUCount() == 0 || deviceInfo == "cpu")
  {
    return -1;
  }
  else
  {
    QString deviceId = deviceInfo.split(":", Qt::SkipEmptyParts).first();
    return deviceId.toInt();
  }
}

void QmitkTotalSegmentatorPreferencePage::UpdateTotalSegPreferencePath()
{
  if (nullptr == m_Preferences)
  {
    this->ShowErrorMessage("Error occurred while saving preferences");
    return;
  }
  m_Preferences->Put("TotalSeg/totalSegPath", "");
  if (m_Ui->overrideBox->isChecked()) // check && store installation variables
  {
    QString customInstallPath = this->GetPythonPathFromUI(m_Ui->customEnvComboBox->currentText());
    if (this->IsTotalSegmentatorInstalled(customInstallPath))
    {
      customInstallPath = QmitkSetupVirtualEnvUtil::GetExactPythonPath(customInstallPath).first;
      m_Preferences->Put("TotalSeg/totalSegPath", customInstallPath.toStdString());
    }
  }
  else
  {
    QString installPath = m_Installer.GetExactPythonPath(m_Installer.GetVirtualEnvPath()).first;
    if (this->IsTotalSegmentatorInstalled(installPath))
      m_Preferences->Put("TotalSeg/totalSegPath", installPath.toStdString());
  }
}

void QmitkTotalSegmentatorPreferencePage::ShowErrorMessage(const QString &message, QMessageBox::Icon icon)
{
  QMessageBox *messageBox = new QMessageBox(icon, nullptr, message);
  messageBox->setAttribute(Qt::WA_DeleteOnClose, true);
  MITK_WARN << message;
  messageBox->exec();
}

void QmitkTotalSegmentatorPreferencePage::WriteErrorMessage(const QString &message)
{
  m_Ui->statusLabel->setText(message);
  m_Ui->statusLabel->setStyleSheet("font-weight: bold; color: red");
  qApp->processEvents();
}

void QmitkTotalSegmentatorPreferencePage::WriteStatusMessage(const QString &message)
{
  m_Ui->statusLabel->setText(message);
  m_Ui->statusLabel->setStyleSheet("color: white");
}
