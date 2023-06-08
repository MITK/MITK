/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentationPreferencePage.h"

#include <mitkBaseApplication.h>
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

#include <ui_QmitkSegmentationPreferencePageControls.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }
}

QmitkSegmentationPreferencePage::QmitkSegmentationPreferencePage()
  : m_Ui(new Ui::QmitkSegmentationPreferencePageControls),
    m_Control(nullptr),
    m_Initializing(false)
{
}

QmitkSegmentationPreferencePage::~QmitkSegmentationPreferencePage()
{
}

void QmitkSegmentationPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkSegmentationPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Initializing = true;

  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);
#ifndef _WIN32
  m_Ui->sysPythonComboBox->addItem("/usr/bin");
#endif
  this->AutoParsePythonPaths();
  m_Ui->sysPythonComboBox->addItem("Select...");
  m_Ui->sysPythonComboBox->setCurrentIndex(0);
  connect(m_Ui->labelSetPresetToolButton, SIGNAL(clicked()), this, SLOT(OnLabelSetPresetButtonClicked()));
  connect(m_Ui->suggestionsToolButton, SIGNAL(clicked()), this, SLOT(OnSuggestionsButtonClicked()));
  connect(m_Ui->installSAMButton, SIGNAL(clicked()), this, SLOT(OnInstallBtnClicked()));
  connect(m_Ui->clearSAMButton, SIGNAL(clicked()), this, SLOT(OnClearInstall()));
  connect(m_Ui->sysPythonComboBox, QOverload<int>::of(&QComboBox::activated),
          [=](int index) { OnSystemPythonChanged(m_Ui->sysPythonComboBox->itemText(index)); });
  QIcon deleteIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/edit-delete.svg"));
  m_Ui->clearSAMButton->setIcon(deleteIcon);

  const QString storageDir = m_Installer.GetVirtualEnvPath();
  m_IsInstalled = QmitkSegmentAnythingToolGUI::IsSAMInstalled(storageDir);
  QString welcomeText;
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
  this->WriteStatusMessage(welcomeText);

  m_Ui->samModelTypeComboBox->addItems(QmitkSegmentAnythingToolGUI::VALID_MODELS_URL_MAP.keys());

  this->Update();
  m_Initializing = false;
}

QWidget* QmitkSegmentationPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkSegmentationPreferencePage::PerformOk()
{
  auto* prefs = GetPreferences();

  prefs->PutBool("compact view", m_Ui->compactViewCheckBox->isChecked());
  prefs->PutBool("draw outline", m_Ui->outlineRadioButton->isChecked());
  prefs->PutBool("selection mode", m_Ui->selectionModeCheckBox->isChecked());
  prefs->Put("label set preset", m_Ui->labelSetPresetLineEdit->text().toStdString());
  prefs->PutBool("default label naming", m_Ui->defaultNameRadioButton->isChecked());
  prefs->Put("label suggestions", m_Ui->suggestionsLineEdit->text().toStdString());
  prefs->PutBool("replace standard suggestions", m_Ui->replaceStandardSuggestionsCheckBox->isChecked());
  prefs->PutBool("suggest once", m_Ui->suggestOnceCheckBox->isChecked());

  prefs->Put("sam parent path", m_Installer.STORAGE_DIR.toStdString());
  prefs->Put("sam python path", m_PythonPath.toStdString());
  prefs->Put("sam modeltype", m_Ui->samModelTypeComboBox->currentText().toStdString());
  return true;
}

void QmitkSegmentationPreferencePage::PerformCancel()
{
}

void QmitkSegmentationPreferencePage::Update()
{
  auto* prefs = GetPreferences();

  m_Ui->compactViewCheckBox->setChecked(prefs->GetBool("compact view", false));

  if (prefs->GetBool("draw outline", true))
  {
    m_Ui->outlineRadioButton->setChecked(true);
  }
  else
  {
    m_Ui->overlayRadioButton->setChecked(true);
  }

  m_Ui->selectionModeCheckBox->setChecked(prefs->GetBool("selection mode", false));

  auto labelSetPreset = mitk::BaseApplication::instance().config().getString(mitk::BaseApplication::ARG_SEGMENTATION_LABELSET_PRESET.toStdString(), "");
  bool isOverriddenByCmdLineArg = !labelSetPreset.empty();

  if (!isOverriddenByCmdLineArg)
    labelSetPreset = prefs->Get("label set preset", "");

  m_Ui->labelSetPresetLineEdit->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->labelSetPresetToolButton->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->labelSetPresetCmdLineArgLabel->setVisible(isOverriddenByCmdLineArg);

  m_Ui->labelSetPresetLineEdit->setText(QString::fromStdString(labelSetPreset));

  if (prefs->GetBool("default label naming", true))
  {
    m_Ui->defaultNameRadioButton->setChecked(true);
  }
  else
  {
    m_Ui->askForNameRadioButton->setChecked(true);
  }

  auto labelSuggestions = mitk::BaseApplication::instance().config().getString(mitk::BaseApplication::ARG_SEGMENTATION_LABEL_SUGGESTIONS.toStdString(), "");
  isOverriddenByCmdLineArg = !labelSuggestions.empty();

  if (!isOverriddenByCmdLineArg)
    labelSuggestions = prefs->Get("label suggestions", "");

  m_Ui->defaultNameRadioButton->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->askForNameRadioButton->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->suggestionsLineEdit->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->suggestionsToolButton->setDisabled(isOverriddenByCmdLineArg);
  m_Ui->suggestionsCmdLineArgLabel->setVisible(isOverriddenByCmdLineArg);

  m_Ui->suggestionsLineEdit->setText(QString::fromStdString(labelSuggestions));

  m_Ui->replaceStandardSuggestionsCheckBox->setChecked(prefs->GetBool("replace standard suggestions", true));
  m_Ui->suggestOnceCheckBox->setChecked(prefs->GetBool("suggest once", true));
}

void QmitkSegmentationPreferencePage::OnLabelSetPresetButtonClicked()
{
  const auto filename = QFileDialog::getOpenFileName(m_Control, QStringLiteral("Load Label Set Preset"), QString(), QStringLiteral("Label set preset (*.lsetp)"));

  if (!filename.isEmpty())
    m_Ui->labelSetPresetLineEdit->setText(filename);
}

void QmitkSegmentationPreferencePage::OnSuggestionsButtonClicked()
{
  const auto filename = QFileDialog::getOpenFileName(m_Control, QStringLiteral("Load Label Suggestions"), QString(), QStringLiteral("Label suggestions (*.json)"));

  if (!filename.isEmpty())
    m_Ui->suggestionsLineEdit->setText(filename);
}

QString QmitkSegmentationPreferencePage::OnSystemPythonChanged(const QString &pyEnv)
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

QString QmitkSegmentationPreferencePage::GetPythonPathFromUI(const QString &pyUI) const
{
  QString fullPath = pyUI;
  if (-1 != fullPath.indexOf(")"))
  {
    fullPath = fullPath.mid(fullPath.indexOf(")") + 2);
  }
  return fullPath.simplified();
}

QString QmitkSegmentationPreferencePage::GetExactPythonPath(const QString &pyEnv) const
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

void QmitkSegmentationPreferencePage::AutoParsePythonPaths()
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

/* bool QmitkSegmentationPreferencePage::IsSAMInstalled(const QString &pythonPath)
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
#endif*/
//  bool isExists = /*QFile::exists(fullPath + QDir::separator() + QString("MITK_SAM"))*/ true && isPythonExists;
//  return isExists;
//}

void QmitkSegmentationPreferencePage::OnInstallBtnClicked()
{
  MITK_INFO << "Install clicked";
  QString systemPython = OnSystemPythonChanged(m_Ui->sysPythonComboBox->currentText());
  if (!systemPython.isEmpty())
  {
    this->WriteStatusMessage("<b>STATUS: </b>Installing SAM...");
    m_Installer.SetSystemPythonPath(systemPython);
    m_IsInstalled = m_Installer.SetupVirtualEnv(m_Installer.VENV_NAME);
    if (m_IsInstalled)
    {
      m_PythonPath = this->GetExactPythonPath(m_Installer.GetVirtualEnvPath());
      this->WriteStatusMessage("<b>STATUS: </b>Successfully installed SAM.");
      auto *prefs = GetPreferences();
      prefs->Put("sam python path", m_PythonPath.toStdString());
    }
    else
    {
      this->WriteErrorMessage("<b>ERROR: </b>Couldn't install SAM.");
    }
  }
}

void QmitkSegmentationPreferencePage::OnClearInstall()
{
  MITK_INFO << "OnClearInstall clicked";
  QDir folderPath(m_Installer.GetVirtualEnvPath());
  m_IsInstalled = folderPath.removeRecursively();
  if (!m_IsInstalled)
  {
    MITK_ERROR << "The virtual environment couldn't be removed. Please check if you have the required access "
                  "privileges or, some other process is accessing the folders.";
  }
}

void QmitkSegmentationPreferencePage::WriteStatusMessage(const QString &message)
{
  m_Ui->samInstallStatusLabel->setText(message);
  m_Ui->samInstallStatusLabel->setStyleSheet("font-weight: bold; color: white");
  qApp->processEvents();
}

void QmitkSegmentationPreferencePage::WriteErrorMessage(const QString &message)
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
