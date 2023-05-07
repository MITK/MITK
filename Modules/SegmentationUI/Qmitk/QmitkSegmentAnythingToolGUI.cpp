/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentAnythingToolGUI.h"

#include "mitkSegmentAnythingTool.h"
#include <QApplication>
#include <QDir>
#include <QmitkStyleManager.h>
#include <QDirIterator>
#include <QStandardPaths>


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
  m_Controls.sysPythonComboBox->addItem("Select");
  m_Controls.sysPythonComboBox->setCurrentIndex(0);
  m_Controls.statusLabel->setTextFormat(Qt::RichText);
  m_Controls.modelTypeComboBox->addItems(VALID_MODELS);

  QString welcomeText;
  this->SetGPUInfo();
  if (m_GpuLoader.GetGPUCount() != 0)
  {
    welcomeText = "<b>STATUS: </b><i>Welcome to TotalSegmentator tool. You're in luck: " +
                  QString::number(m_GpuLoader.GetGPUCount()) + " GPU(s) were detected.</i>";
  }
  else
  {
    welcomeText = "<b>STATUS: </b><i>Welcome to TotalSegmentator tool. Sorry, " +
                  QString::number(m_GpuLoader.GetGPUCount()) + " GPUs were detected.</i>";
  }
  connect(m_Controls.activateButton, SIGNAL(clicked()), this, SLOT(OnActivateBtnClicked()));
  connect(m_Controls.resetButton, SIGNAL(clicked()), this, SLOT(OnResetPicksClicked()));
  /* connect(m_Controls.clearButton, SIGNAL(clicked()), this, SLOT(OnClearInstall()));
  connect(m_Controls.installButton, SIGNAL(clicked()), this, SLOT(OnInstallBtnClicked()));
  connect(m_Controls.sysPythonComboBox,
          QOverload<int>::of(&QComboBox::activated),
          [=](int index) { OnSystemPythonChanged(m_Controls.sysPythonComboBox->itemText(index)); }); */

  QIcon deleteIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/edit-delete.svg"));
  QIcon arrowIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/go-next.svg"));
  m_Controls.clearButton->setIcon(deleteIcon);
  m_Controls.activateButton->setIcon(arrowIcon);

  mainLayout->addLayout(m_Controls.verticalLayout);
  Superclass::InitializeUI(mainLayout);
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
    /* if (!this->IsSAMInstalled(m_PythonPath))
    {
      throw std::runtime_error(WARNING_SAM_NOT_FOUND);
    }*/
    tool->SetIsAuto(m_Controls.autoRButton->isChecked());
    tool->SetPythonPath(m_PythonPath.toStdString());
    tool->SetGpuId(FetchSelectedGPUFromUI());
    QString modelType = m_Controls.modelTypeComboBox->currentText();
    std::string path = "c:\\Data";
    tool->SetModelType(modelType.toStdString());
    this->WriteStatusMessage(
      QString("<b>STATUS: </b><i>Checking if model is already downloaded... This might take a while.</i>"));
    if (tool->run_download_model(path))
    {
      this->WriteStatusMessage(QString("<b>STATUS: </b><i>Model is sucessfully found.</i>"));
      tool->IsReadyOn();
    }
    else
    {
      tool->IsReadyOff();
      this->WriteStatusMessage(QString("<b>STATUS: </b><i>Model couldn't be downloaded. Please try again.</i>"));
    }
    m_Controls.activateButton->setEnabled(true);
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
  bool isExists = QFile::exists(fullPath + QDir::separator() + QString("MITK_SAM")) && isPythonExists;
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