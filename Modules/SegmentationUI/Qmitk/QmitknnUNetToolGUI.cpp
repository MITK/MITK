/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitknnUNetToolGUI.h"

#include "mitknnUnetTool.h"
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QOpenGLWidget>
#include <QProcess>
#include <QtGlobal>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitknnUNetToolGUI, "")

QmitknnUNetToolGUI::QmitknnUNetToolGUI() : QmitkAutoMLSegmentationToolGUIBase()
{
  // Nvidia-smi command returning zero doesn't alway mean lack of GPUs.
  // Pytorch uses its own libraries to communicate to the GPUs. Hence, only a warning can be given.
  if (m_GpuLoader.GetGPUCount() == 0)
  {
    ShowErrorMessage(std::string("WARNING: No GPUs were detected on your machine. The nnUNet tool might not work."));
  }
  m_SegmentationThread = new QThread(this);
  m_Worker = new nnUNetSegmentationWorker;
  m_Worker->moveToThread(m_SegmentationThread);
}

QmitknnUNetToolGUI::~QmitknnUNetToolGUI()
{
  this->m_SegmentationThread->quit();
  this->m_SegmentationThread->wait();
}

void QmitknnUNetToolGUI::ConnectNewTool(mitk::AutoSegmentationWithPreviewTool *newTool)
{
  Superclass::ConnectNewTool(newTool);
  newTool->IsTimePointChangeAwareOff();
}

void QmitknnUNetToolGUI::InitializeUI(QBoxLayout *mainLayout)
{
  m_Controls.setupUi(this);
#ifndef _WIN32
  m_Controls.pythonEnvComboBox->addItem("/usr/bin");
#endif
  m_Controls.pythonEnvComboBox->addItem("Select");
  AutoParsePythonPaths();
  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnSettingsAccepted()));
  connect(m_Controls.modeldirectoryBox,
          SIGNAL(directoryChanged(const QString &)),
          this,
          SLOT(OnDirectoryChanged(const QString &)));
  connect(
    m_Controls.modelBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnModelChanged(const QString &)));
  connect(m_Controls.taskBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnTaskChanged(const QString &)));
  connect(
    m_Controls.plannerBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnTrainerChanged(const QString &)));
  connect(m_Controls.nopipBox, SIGNAL(stateChanged(int)), this, SLOT(OnCheckBoxChanged(int)));
  connect(m_Controls.multiModalBox, SIGNAL(stateChanged(int)), this, SLOT(OnCheckBoxChanged(int)));
  connect(m_Controls.multiModalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnModalitiesNumberChanged(int)));
  connect(m_Controls.pythonEnvComboBox,
#if QT_VERSION >= 0x050F00 // 5.15
          SIGNAL(textActivated(const QString &)),
#elif QT_VERSION >= 0x050C00 // 5.12
          SIGNAL(activated(const QString &)),
#endif
          this,
          SLOT(OnPythonPathChanged(const QString &)));

  connect(this, &QmitknnUNetToolGUI::Operate, m_Worker, &nnUNetSegmentationWorker::DoWork);
  connect(m_Worker, &nnUNetSegmentationWorker::Finished, this, &QmitknnUNetToolGUI::SegmentationResultHandler);
  connect(m_Worker, &nnUNetSegmentationWorker::Failed, this, &QmitknnUNetToolGUI::SegmentationProcessFailed);
  connect(m_SegmentationThread, &QThread::finished, m_Worker, &QObject::deleteLater);

  m_Controls.codedirectoryBox->setVisible(false);
  m_Controls.nnUnetdirLabel->setVisible(false);
  m_Controls.multiModalSpinBox->setVisible(false);
  m_Controls.multiModalSpinLabel->setVisible(false);

  m_Controls.statusLabel->setTextFormat(Qt::RichText);
  m_Controls.statusLabel->setText("<b>STATUS: </b><i>Welcome to nnUNet. " + QString::number(m_GpuLoader.GetGPUCount()) +
                                  " GPUs were detected.</i>");

  if (m_GpuLoader.GetGPUCount() != 0)
  {
    m_Controls.gpuSpinBox->setMaximum(m_GpuLoader.GetGPUCount() - 1);
  }
  mainLayout->addLayout(m_Controls.verticalLayout);
  Superclass::InitializeUI(mainLayout);
  m_UI_ROWS = m_Controls.advancedSettingsLayout->rowCount(); // Must do. Row count is correct only here.
}

void QmitknnUNetToolGUI::OnSettingsAccepted()
{
  auto tool = this->GetConnectedToolAs<mitk::nnUNetTool>();
  if (nullptr != tool)
  {
    try
    {
      QString modelName = m_Controls.modelBox->currentText();
      QString taskName = m_Controls.taskBox->currentText();
      bool isNoPip = m_Controls.nopipBox->isChecked();

      QString pythonPathTextItem = m_Controls.pythonEnvComboBox->currentText();
      QString pythonPath = pythonPathTextItem.mid(pythonPathTextItem.indexOf(" ") + 1);
#ifdef _WIN32
      if (!isNoPip && !(pythonPath.endsWith("Scripts", Qt::CaseInsensitive) ||
                        pythonPath.endsWith("Scripts/", Qt::CaseInsensitive)))
      {
        pythonPath += QDir::separator() + QString("Scripts");
      }
#else
      if (!(pythonPath.endsWith("bin", Qt::CaseInsensitive) || pythonPath.endsWith("bin/", Qt::CaseInsensitive)))
      {
        pythonPath += QDir::separator() + QString("bin");
      }
#endif
      std::string nnUNetDirectory;
      if (isNoPip)
      {
        nnUNetDirectory = m_Controls.codedirectoryBox->directory().toStdString();
      }
      else if (!IsNNUNetInstalled(pythonPath))
      {
        throw std::runtime_error("nnUNet is not detected in the selected python environment. Please select a valid "
                                 "python environment or install nnUNet.");
      }
      QString trainerPlanner = m_Controls.trainerBox->currentText();
      QString splitterString = "__";
      tool->EnsembleOff();

      if (modelName.startsWith("ensemble", Qt::CaseInsensitive))
      {
        QString ppJsonFile =
          QDir::cleanPath(m_ModelDirectory + QDir::separator() + modelName + QDir::separator() + taskName +
                          QDir::separator() + trainerPlanner + QDir::separator() + "postprocessing.json");
        if (QFile(ppJsonFile).exists())
        {
          tool->EnsembleOn();
          tool->SetPostProcessingJsonDirectory(ppJsonFile.toStdString());
          splitterString = "--";
        }
      }
      QStringList trainerSplitParts = trainerPlanner.split(splitterString, QString::SplitBehavior::SkipEmptyParts);
      std::vector<mitk::ModelParams> requestQ;
      if (tool->GetEnsemble())
      {
        foreach (QString modelSet, trainerSplitParts)
        {
          modelSet.remove("ensemble_", Qt::CaseInsensitive);
          QStringList splitParts = modelSet.split("__", QString::SplitBehavior::SkipEmptyParts);
          QString modelName = splitParts.first();
          QString trainer = splitParts.at(1);
          QString planId = splitParts.at(2);
          auto testfold = std::vector<std::string>(1, "1");
          mitk::ModelParams modelObject = MapToRequest(modelName, taskName, trainer, planId, testfold);
          requestQ.push_back(modelObject);
        }
      }
      else
      {
        QString trainer = trainerSplitParts.first();
        QString planId = trainerSplitParts.last();
        std::vector<std::string> fetchedFolds = FetchSelectedFoldsFromUI();
        mitk::ModelParams modelObject = MapToRequest(modelName, taskName, trainer, planId, fetchedFolds);
        requestQ.push_back(modelObject);
      }

      tool->m_ParamQ.clear();
      tool->m_ParamQ = requestQ;

      tool->SetnnUNetDirectory(nnUNetDirectory);
      tool->SetPythonPath(pythonPath.toStdString());
      tool->SetModelDirectory(m_ModelDirectory.left(m_ModelDirectory.lastIndexOf(QDir::separator())).toStdString());
      // checkboxes
      tool->SetMirror(m_Controls.mirrorBox->isChecked());
      tool->SetMixedPrecision(m_Controls.mixedPrecisionBox->isChecked());
      tool->SetNoPip(isNoPip);
      tool->SetMultiModal(m_Controls.multiModalBox->isChecked());
      // Spinboxes
      tool->SetGpuId(static_cast<unsigned int>(m_Controls.gpuSpinBox->value()));
      // Multi-Modal
      tool->MultiModalOff();
      if (m_Controls.multiModalBox->isChecked())
      {
        tool->m_OtherModalPaths.clear();
        tool->m_OtherModalPaths = FetchMultiModalPathsFromUI();
        tool->MultiModalOn();
      }

      if (!m_SegmentationThread->isRunning())
      {
        MITK_DEBUG << "Starting thread...";
        m_SegmentationThread->start();
      }
      m_Controls.statusLabel->setText("<b>STATUS: </b><i>Starting Segmentation task... This might take a while.</i>");
      emit Operate(tool);
    }
    catch (const std::exception &e)
    {
      this->setCursor(Qt::ArrowCursor);
      std::stringstream stream;
      stream << "Error while processing parameters for nnUNet segmentation. Reason: " << e.what();
      QMessageBox *messageBox = new QMessageBox(QMessageBox::Critical, nullptr, stream.str().c_str());
      messageBox->exec();
      delete messageBox;
      MITK_ERROR << stream.str();
      return;
    }
    catch (...)
    {
      this->setCursor(Qt::ArrowCursor);
      std::stringstream stream;
      stream << "Unkown error occured while generation nnUNet segmentation.";
      QMessageBox *messageBox = new QMessageBox(QMessageBox::Critical, nullptr, stream.str().c_str());
      messageBox->exec();
      delete messageBox;
      MITK_ERROR << stream.str();
      return;
    }
  }
}

std::vector<std::string> QmitknnUNetToolGUI::FetchMultiModalPathsFromUI()
{
  std::vector<std::string> paths;
  if (m_Controls.multiModalBox->isChecked() && !m_ModalPaths.empty())
  {
    for (auto modality : m_ModalPaths)
    {
      paths.push_back(modality->currentPath().toStdString());
    }
  }
  return paths;
}

bool QmitknnUNetToolGUI::IsNNUNetInstalled(const QString &pythonPath)
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
  return QFile::exists(fullPath + QDir::separator() + QString("nnUNet_predict"));
}

void QmitknnUNetToolGUI::ShowErrorMessage(std::string &message)
{
  this->setCursor(Qt::ArrowCursor);
  QMessageBox *messageBox = new QMessageBox(QMessageBox::Critical, nullptr, message.c_str());
  messageBox->exec();
  delete messageBox;
  MITK_WARN << message;
}
