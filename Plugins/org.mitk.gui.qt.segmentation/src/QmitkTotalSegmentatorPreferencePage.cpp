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
#include <QProcess>
#include <QStringDecoder>


namespace
{
  mitk::IPreferences *GetPreferences()
  {
    auto *preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }

  // Returns license key if already set
  QString GetLicenseKey(const QString &pythonPath)
  {
    QProcess pyProcess;
    QString licenseText;
    pyProcess.start(pythonPath + QDir::separator() +
#ifdef _WIN32
                      "python.exe",
#else
                      "python3",
#endif
                    QStringList() << "-c"
                                  << "import totalsegmentator.config as tc;"
                                      "print(tc.get_license_number())",
                    QProcess::ReadOnly);
    if (pyProcess.waitForFinished())
    {
      licenseText = QString(QStringDecoder(QStringDecoder::Utf8)(pyProcess.readAllStandardOutput()));
    }
    return licenseText;
  }

  // Resets license key to empty string, i.e. no more license
  QString ResetLicenseKey(const QString &pythonPath)
  {
    QProcess pyProcess;
    QString statusText;
    pyProcess.start(pythonPath + QDir::separator() + 
#ifdef _WIN32
                      "python.exe",
#else
                      "python3",
#endif
                    QStringList() << "-c"
                                  << "import totalsegmentator.config as tc;tc.set_license_number('', True);"
                                     "print('Removed License.')",
                    QProcess::ReadOnly);
    if (pyProcess.waitForFinished())
    {
      statusText = QString(QStringDecoder(QStringDecoder::Utf8)(pyProcess.readAllStandardOutput()));
    }
    return statusText;
  }

  // Sets Totalseg license key 
  QString SetLicenseKey(const QString &pythonPath, const QString &licenseText)
  {
    QProcess pyProcess;
    QString statusText;
    pyProcess.start(pythonPath + QDir::separator() + "totalseg_set_license"
#ifdef _WIN32
                      ".exe"
#endif
                    ,QStringList() << "-l" << licenseText,
                     QProcess::ReadOnly);
    if (pyProcess.waitForFinished())
    {
      statusText = QString(QStringDecoder(QStringDecoder::Utf8)(pyProcess.readAllStandardOutput()));
    }
    return statusText;
  }
}

void QmitkTotalSegmentatorPreferencePage::Init(berry::IWorkbench::Pointer) {}
const QString QmitkTotalSegmentatorPreferencePage::WARNING_TOTALSEG_NOT_FOUND =
  "TotalSegmentator is not detected in the selected python environment. Please select a valid "
  "python environment or install TotalSegmentator.";
const QString QmitkTotalSegmentatorPreferencePage::WARNING_PYTHON_NOT_FOUND =
  "Python is not detected in the selected path. "
  "Please select a path with a valid python install.";

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
  this->SetDeviceInfo();
  m_Ui->overrideBox->setChecked(m_Preferences->GetBool("TotalSeg/isCustomInstall", false));
  bool isCustomInstall = m_Ui->overrideBox->isChecked();
  m_Ui->statusLabel->setWordWrap(true);
  m_Ui->sysPythonComboBox->setDuplicatesEnabled(false);
  m_Ui->customEnvComboBox->setDuplicatesEnabled(false);
  m_Ui->customEnvComboBox->setEnabled(isCustomInstall);
  QString m_CustomInstallPath = QString::fromStdString(m_Preferences->Get("TotalSeg/customInstallPath", ""));
  m_SysPythonPath = QString::fromStdString(m_Preferences->Get("TotalSeg/sysPythonPath", ""));
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
  m_Ui->licenseBox->setMaxLength(LICENSE_KEY_LENGTH);

  connect(m_Ui->installTotalButton, SIGNAL(clicked()), this, SLOT(OnInstallButtonClicked()));
  connect(m_Ui->clearTotalbutton, SIGNAL(clicked()), this, SLOT(OnClearButtonClicked()));
  connect(m_Ui->applyButton, SIGNAL(clicked()), this, SLOT(OnApplyButtonClicked()));
  connect(m_Ui->overrideBox, SIGNAL(stateChanged(int)), this, SLOT(OnOverrideBoxChecked(int)));
  connect(m_Ui->customEnvComboBox,
          QOverload<int>::of(&QComboBox::activated),
          [=](int index) { OnPythonPathChanged(m_Ui->customEnvComboBox->itemText(index)); });
  connect(m_Ui->sysPythonComboBox,
          QOverload<int>::of(&QComboBox::activated),
          [=](int index) { OnSystemPythonChanged(m_Ui->sysPythonComboBox->itemText(index)); });

  this->UpdateStatusLabel(); 
  this->UpdateLicenseBox();
}

QWidget *QmitkTotalSegmentatorPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkTotalSegmentatorPreferencePage::PerformOk()
{
  m_Preferences->Put("TotalSeg/customInstallPath", m_Ui->customEnvComboBox->currentText().toStdString());
  m_Preferences->Put("TotalSeg/sysPythonPath", m_SysPythonPath.toStdString());
  m_Preferences->PutBool("TotalSeg/isCustomInstall", m_Ui->overrideBox->isChecked());
  m_Preferences->PutInt("TotalSeg/deviceId", this->FetchSelectedDeviceFromUI());
  m_Preferences->Put("TotalSeg/totalSegPath", this->GetExactPythonPath().toStdString());
  return true;
}

void QmitkTotalSegmentatorPreferencePage::PerformCancel()
{
  m_Preferences->Put("TotalSeg/totalSegPath", this->GetExactPythonPath().toStdString());
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
  QmitkToolInstallDialog *installDialog = new QmitkToolInstallDialog(m_Control, "TotalSegmentator");
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
          pythonCode.append("print('TotalSegmentator was installed with CUDA') if torch.cuda.is_available() else "
                            "print('TotalSegmentator was installed WITHOUT CUDA');");
          m_Installer.ExecutePython(pythonCode, &QmitkTotalSegmentatorToolInstaller::PrintProcessEvent);
          return true;
        },
        &QmitkTotalSegmentatorToolInstaller::PrintProcessEvent);
      });
  while (future.isRunning())
  {
    qApp->processEvents();
    QThread::msleep(100);
  }

  QString installPath = QmitkSetupVirtualEnvUtil::GetExactPythonPath(m_Installer.GetVirtualEnvPath()).first;

  if (future.result() && this->IsTotalSegmentatorInstalled(installPath))
  {
    this->WriteStatusMessage("<b>STATUS: </b>Successfully installed TotalSegmentator.");
    installDialog->FinishInstallation("TotalSegmentator was succesfully installed");
    m_IsInstalled = true;
    m_Installer.SetVirtualEnvPath(installPath);
  }
  else
  {
    this->WriteErrorMessage("<b>ERROR: </b>Couldn't install TotalSegmentator.");
    installDialog->FinishInstallation("Couldn't install TotalSegmentator");
    m_Ui->installTotalButton->setDisabled(false);
    this->WriteStatusMessage("<b>ERROR: </b>Couldn't install TotalSegmentator.");
  }
  this->UpdateStatusLabel();
  this->UpdateLicenseBox();
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
    if (this->IsTotalSegmentatorInstalled(this->GetPythonPathFromUI(path)))
    {
      m_Ui->customEnvComboBox->insertItem(0, path);
      m_Ui->customEnvComboBox->setCurrentIndex(0);
    }
    else
    {
      this->ShowErrorMessage(WARNING_TOTALSEG_NOT_FOUND);
    }
  }
  this->UpdateStatusLabel();
}

void QmitkTotalSegmentatorPreferencePage::OnSystemPythonChanged(const QString &pyEnv)
{
  if (pyEnv == QString("Select..."))
  {
    QString path = QFileDialog::getExistingDirectory(m_Ui->sysPythonComboBox->parentWidget(), "Python Path", "dir");
    std::pair<QString, QString> pyPath;
    pyPath = QmitkSetupVirtualEnvUtil::GetExactPythonPath(this->GetPythonPathFromUI(path));
    if (!pyPath.first.isEmpty())
    {
      m_SysPythonPath = pyPath.first;
      m_Ui->sysPythonComboBox->insertItem(0, path);
      m_Ui->sysPythonComboBox->setCurrentIndex(0);
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
    text += QString::number(m_DeviceLoader.GetGPUCount()) + " GPU(s) were found";
  }
  m_Ui->statusLabel->setText(text);
}

bool QmitkTotalSegmentatorPreferencePage::IsTotalSegmentatorInstalled(const QString &pythonPath)
{
  QString fullPath = pythonPath;
  bool hasPython = false;
  bool hasTotalSeg = false;
#ifdef _WIN32
  hasPython = QFile::exists(fullPath + QDir::separator() + QString("python.exe"));
  if (!(fullPath.endsWith("Scripts", Qt::CaseInsensitive) || fullPath.endsWith("Scripts/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("Scripts");
    hasPython =
      (!hasPython) ? QFile::exists(fullPath + QDir::separator() + QString("python.exe")) : hasPython;
  }
  hasTotalSeg = QFile::exists(fullPath + QDir::separator() + QString("TotalSegmentator.exe")) && hasPython;
#else
  hasPython = QFile::exists(fullPath + QDir::separator() + QString("python3"));
  if (!(fullPath.endsWith("bin", Qt::CaseInsensitive) || fullPath.endsWith("bin/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("bin");
    hasPython =
      (!hasPython) ? QFile::exists(fullPath + QDir::separator() + QString("python3")) : hasPython;
  }
  hasTotalSeg = QFile::exists(fullPath + QDir::separator() + QString("TotalSegmentator")) && hasPython;
#endif
  if (hasTotalSeg && m_Installer.TOTALSEGMENTATOR_VERSION !=
                    QmitkSetupVirtualEnvUtil::GetPipPackageVersion(fullPath, "TotalSegmentator"))
  {
    hasTotalSeg = false;
  }
  return hasTotalSeg;
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

QString QmitkTotalSegmentatorPreferencePage::GetExactPythonPath()
{
  QString pythonPath;
  if (m_Ui->overrideBox->isChecked())
  {
    QString customInstallPath = this->GetPythonPathFromUI(m_Ui->customEnvComboBox->currentText());
    if (this->IsTotalSegmentatorInstalled(customInstallPath))
    {
      pythonPath = QmitkSetupVirtualEnvUtil::GetExactPythonPath(customInstallPath).first;
    }
  }
  else
  {
    QString installPath = m_Installer.GetExactPythonPath(m_Installer.GetVirtualEnvPath()).first;
    if (this->IsTotalSegmentatorInstalled(installPath))
      pythonPath = installPath;
  }
  return pythonPath;
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

void QmitkTotalSegmentatorPreferencePage::OnApplyButtonClicked() 
{
  QString licenseText = m_Ui->licenseBox->text().trimmed();
  this->AddOrRemoveLicense(licenseText);
}

void QmitkTotalSegmentatorPreferencePage::AddOrRemoveLicense(const QString &licenseText)
{
  QString pythonPath = this->GetExactPythonPath();
  if (pythonPath.isEmpty())
  {
    this->ShowErrorMessage("TotalSegmentator not found to apply the license key.");
    return;
  }
  QString statusResponse;
  if (licenseText.isEmpty()) // remove license key
  {
    statusResponse = ::ResetLicenseKey(pythonPath);
  }
  else if (licenseText.length() == LICENSE_KEY_LENGTH) // apply valid license
  {
    statusResponse = ::SetLicenseKey(pythonPath, licenseText);
  }
  else
  {
    const QString errorText = "Invalid license key entered. 18 characters expected.";
    this->ShowErrorMessage(errorText);
    MITK_INFO << errorText;
    return;
  }
  this->WriteStatusMessage(statusResponse);
  if (statusResponse.startsWith("ERROR") || statusResponse.startsWith("Removed"))
    m_Preferences->PutBool("TotalSeg/hasLicense", false);
  else
    m_Preferences->PutBool("TotalSeg/hasLicense", true);
}

void QmitkTotalSegmentatorPreferencePage::UpdateLicenseBox()
{
  QString pythonPath = this->GetExactPythonPath();
  m_Ui->licenseBox->clear();
  if (!pythonPath.isEmpty())// populate license text box
    m_Ui->licenseBox->setText(::GetLicenseKey(pythonPath));
}
