#include "QmitkTotalSegmentatorToolGUI.h"

#include "mitkProcessExecutor.h"
#include "mitkTotalSegmentatorTool.h"
#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QIcon>
#include <QMessageBox>
#include <QStandardPaths>
#include <QUrl>
#include <QtGlobal>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkTotalSegmentatorToolGUI, "")

QmitkTotalSegmentatorToolGUI::QmitkTotalSegmentatorToolGUI()
  : QmitkMultiLabelSegWithPreviewToolGUIBase(), m_SuperclassEnableConfirmSegBtnFnc(m_EnableConfirmSegBtnFnc)
{
  // Nvidia-smi command returning zero doesn't always imply lack of GPUs.
  // Pytorch uses its own libraries to communicate to the GPUs. Hence, only a warning can be given.
  if (m_GpuLoader.GetGPUCount() == 0)
  {
    std::string warning = "WARNING: No GPUs were detected on your machine. The TotalSegmentator tool can be very slow.";
    this->ShowErrorMessage(warning);
  }
  m_EnableConfirmSegBtnFnc = [this](bool enabled)
  { return !m_FirstPreviewComputation ? m_SuperclassEnableConfirmSegBtnFnc(enabled) : false; };
  const QString storageDir =
    QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/" + qApp->organizationName() + "/";
  MITK_INFO << storageDir.toStdString();
  setUpTotalSegmentator(storageDir);
}

void QmitkTotalSegmentatorToolGUI::ConnectNewTool(mitk::SegWithPreviewTool *newTool)
{
  Superclass::ConnectNewTool(newTool);
  newTool->IsTimePointChangeAwareOff();
  m_FirstPreviewComputation = true;
}

void QmitkTotalSegmentatorToolGUI::InitializeUI(QBoxLayout *mainLayout)
{
  m_Controls.setupUi(this);
  m_Controls.pythonEnvComboBox->addItem("Select");
  m_Controls.previewButton->setDisabled(true);
  m_Controls.statusLabel->setTextFormat(Qt::RichText);
  m_Controls.subtaskComboBox->addItems(m_VALID_TASKS);
  AutoParsePythonPaths();
  SetGPUInfo();
  if (m_GpuLoader.GetGPUCount() != 0)
  {
    WriteStatusMessage(QString("<b>STATUS: </b><i>Welcome to Total Segmentator tool. You're in luck: " + QString::number(m_GpuLoader.GetGPUCount()) +
                               " GPU(s) were detected.</i>"));
  }
  else
  {
    WriteErrorMessage(QString("<b>STATUS: </b><i>Welcome to Total Segmentator tool. Sorry, " + QString::number(m_GpuLoader.GetGPUCount()) +
                              " GPUs were detected.</i>"));
  }
  mainLayout->addLayout(m_Controls.verticalLayout);

  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnPreviewBtnClicked()));
  connect(m_Controls.pythonEnvComboBox,
          SIGNAL(currentTextChanged(const QString &)),
          this,
          SLOT(OnPythonPathChanged(const QString &)));

  Superclass::InitializeUI(mainLayout);

  QString lastSelectedPyEnv = m_Settings.value("TotalSeg/LastPythonPath").toString();
  m_Controls.pythonEnvComboBox->insertItem(0, lastSelectedPyEnv);
}

void QmitkTotalSegmentatorToolGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
}

void QmitkTotalSegmentatorToolGUI::SetGPUInfo()
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

unsigned int QmitkTotalSegmentatorToolGUI::FetchSelectedGPUFromUI()
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

namespace
{
  void onPythonProcessEvent(itk::Object * /*pCaller*/, const itk::EventObject &e, void *)
  {
    std::string testCOUT;
    std::string testCERR;
    const auto *pEvent = dynamic_cast<const mitk::ExternalProcessStdOutEvent *>(&e);

    if (pEvent)
    {
      testCOUT = testCOUT + pEvent->GetOutput();
      MITK_INFO << testCOUT;
    }

    const auto *pErrEvent = dynamic_cast<const mitk::ExternalProcessStdErrEvent *>(&e);

    if (pErrEvent)
    {
      testCERR = testCERR + pErrEvent->GetOutput();
      MITK_ERROR << testCERR;
    }
  }
} // namespace

void QmitkTotalSegmentatorToolGUI::setUpTotalSegmentator(const QString &path)
{
  const QString VENV_NAME = ".totalsegmentator";
  QDir folderPath(path);
  folderPath.mkdir(VENV_NAME);
  if (!folderPath.cd(VENV_NAME))
  {
    return;
  }
  mitk::ProcessExecutor::ArgumentListType args;
  auto spExec = mitk::ProcessExecutor::New();
  itk::CStyleCommand::Pointer spCommand = itk::CStyleCommand::New();
  spCommand->SetCallback(&onPythonProcessEvent);
  spExec->AddObserver(mitk::ExternalProcessOutputEvent(), spCommand);

  args.push_back("-m");
  args.push_back("venv");
  args.push_back(VENV_NAME.toStdString());
  spExec->Execute(path.toStdString(), "python", args);

  if (folderPath.cd("bin"))
  {
    std::string workingDir;
    workingDir = folderPath.absolutePath().toStdString();
    args.clear();
    args.push_back("install");
    args.push_back("Totalsegmentator");
    spExec->Execute(workingDir, "pip3", args);
    if (this->IsTotalSegmentatorInstalled(folderPath.absolutePath()))
    {
      m_PythonPath = folderPath.absolutePath();
    }
    else
    {
      MITK_ERROR << m_WARNING_TOTALSEG_NOT_FOUND;
    }
  }
}

void QmitkTotalSegmentatorToolGUI::OnPreviewBtnClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::TotalSegmentatorTool>();
  if (nullptr == tool)
  {
    return;
  }

  QString pythonPathTextItem = "";
  try
  {
    m_Controls.previewButton->setEnabled(false);
    qApp->processEvents();
    if (!this->IsTotalSegmentatorInstalled(m_PythonPath))
    {
      throw std::runtime_error(m_WARNING_TOTALSEG_NOT_FOUND);
    }
    pythonPathTextItem = m_Controls.pythonEnvComboBox->currentText();
    bool isFast = m_Controls.fastBox->isChecked();
    QString subTask = m_Controls.subtaskComboBox->currentText();
    if (subTask != m_VALID_TASKS[0])
    {
      isFast = true;
    }
    tool->SetPythonPath(m_PythonPath.toStdString());
    tool->SetGpuId(FetchSelectedGPUFromUI());
    tool->SetFast(isFast);
    tool->SetSubTask(subTask.toStdString());
    this->WriteStatusMessage(QString("<b>STATUS: </b><i>Starting Segmentation task... This might take a while.</i>"));
    tool->UpdatePreview();
    m_Controls.previewButton->setEnabled(true);
  }
  catch (const std::exception &e)
  {
    std::stringstream errorMsg;
    errorMsg << "<b>STATUS: </b>Error while processing parameters for TotalSegmentator segmentation. Reason: "
             << e.what();
    this->ShowErrorMessage(errorMsg.str());
    this->WriteErrorMessage(QString::fromStdString(errorMsg.str()));
    m_Controls.previewButton->setEnabled(true);
    return;
  }
  catch (...)
  {
    std::string errorMsg = "Unkown error occured while generation TotalSegmentator segmentation.";
    this->ShowErrorMessage(errorMsg);
    m_Controls.previewButton->setEnabled(true);
    return;
  }
  this->SetLabelSetPreview(tool->GetPreviewSegmentation());
  tool->IsTimePointChangeAwareOn();
  this->ActualizePreviewLabelVisibility();
  this->WriteStatusMessage("<b>STATUS: </b><i>Segmentation task finished successfully.</i>");
  if (!pythonPathTextItem.isEmpty())
  { // only cache if the prediction ended without errors.
    m_Settings.setValue("TotalSeg/LastPythonPath", pythonPathTextItem);
  }
}

void QmitkTotalSegmentatorToolGUI::ShowErrorMessage(const std::string &message, QMessageBox::Icon icon)
{
  this->setCursor(Qt::ArrowCursor);
  QMessageBox *messageBox = new QMessageBox(icon, nullptr, message.c_str());
  messageBox->exec();
  delete messageBox;
  MITK_WARN << message;
}

void QmitkTotalSegmentatorToolGUI::WriteStatusMessage(const QString &message)
{
  m_Controls.statusLabel->setText(message);
  m_Controls.statusLabel->setStyleSheet("font-weight: bold; color: white");
}

void QmitkTotalSegmentatorToolGUI::WriteErrorMessage(const QString &message)
{
  m_Controls.statusLabel->setText(message);
  m_Controls.statusLabel->setStyleSheet("font-weight: bold; color: red");
}

bool QmitkTotalSegmentatorToolGUI::IsTotalSegmentatorInstalled(const QString &pythonPath)
{
  QString fullPath = pythonPath;
#ifdef _WIN32
  if (!(fullPath.endsWith("Scripts", Qt::CaseInsensitive) || fullPath.endsWith("Scripts/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("Scripts");
  }
#else
  if (!(fullPath.endsWith("bin", Qt::CaseInsensitive) || fullPath.endsWith("bin/", Qt::CaseInsensitive)))
  {
    fullPath += QDir::separator() + QString("bin");
  }
#endif
  fullPath = fullPath.mid(fullPath.indexOf(" ") + 1);
  bool isExists = QFile::exists(fullPath + QDir::separator() + QString("TotalSegmentator")) &&
#ifdef _WIN32
                  QFile::exists(fullPath + QDir::separator() + QString("python.exe"));
#else
                  QFile::exists(fullPath + QDir::separator() + QString("python3"));
#endif
  return isExists;
}

void QmitkTotalSegmentatorToolGUI::AutoParsePythonPaths()
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
        m_Controls.pythonEnvComboBox->insertItem(0, "(base): " + searchDir);
        searchDir.append((QDir::separator() + QString("envs")));
      }
    }
    for (QDirIterator subIt(searchDir, QDir::AllDirs, QDirIterator::NoIteratorFlags); subIt.hasNext();)
    {
      subIt.next();
      QString envName = subIt.fileName();
      if (!envName.startsWith('.')) // Filter out irrelevent hidden folders, if any.
      {
        m_Controls.pythonEnvComboBox->insertItem(0, "(" + envName + "): " + subIt.filePath());
      }
    }
  }
  m_Controls.pythonEnvComboBox->setCurrentIndex(-1);
}

void QmitkTotalSegmentatorToolGUI::OnPythonPathChanged(const QString &pyEnv)
{
  if (pyEnv == QString("Select"))
  {
    QString path =
      QFileDialog::getExistingDirectory(m_Controls.pythonEnvComboBox->parentWidget(), "Python Path", "dir");
    if (!path.isEmpty())
    {
      this->OnPythonPathChanged(path);                                  // recall same function for new path validation
      bool oldState = m_Controls.pythonEnvComboBox->blockSignals(true); // block signal firing while inserting item
      m_Controls.pythonEnvComboBox->insertItem(0, path);
      m_Controls.pythonEnvComboBox->setCurrentIndex(0);
      m_Controls.pythonEnvComboBox->blockSignals(
        oldState); // unblock signal firing after inserting item. Remove this after Qt6 migration
    }
  }
  else if (!this->IsTotalSegmentatorInstalled(pyEnv))
  {
    this->ShowErrorMessage(m_WARNING_TOTALSEG_NOT_FOUND);
    m_Controls.previewButton->setDisabled(true);
  }
  else
  {
    // Show positive status meeage
    m_Controls.previewButton->setDisabled(false);
    m_PythonPath = pyEnv.mid(pyEnv.indexOf(" ") + 1);
#ifdef _WIN32
    if (!(m_PythonPath.endsWith("Scripts", Qt::CaseInsensitive) ||
          m_PythonPath.endsWith("Scripts/", Qt::CaseInsensitive)))
    {
      m_PythonPath += QDir::separator() + QString("Scripts");
    }
#else
    if (!(m_PythonPath.endsWith("bin", Qt::CaseInsensitive) || m_PythonPath.endsWith("bin/", Qt::CaseInsensitive)))
    {
      m_PythonPath += QDir::separator() + QString("bin");
    }
#endif
  }
}
