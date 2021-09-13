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
  if (GetGPUCount() == 0)
  {
    std::stringstream stream;
    stream << "WARNING: No GPUs were detected on your machine. The nnUNet plugin might not work.";
    QMessageBox *messageBox = new QMessageBox(QMessageBox::Critical, nullptr, stream.str().c_str());
    messageBox->exec();
    delete messageBox;
    MITK_WARN << stream.str();
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

#if defined(__APPLE__) || defined(MACOSX) || defined(linux) || defined(__linux__)
  m_Controls.pythonEnvComboBox->addItem("/usr/bin");
#endif

  m_Controls.pythonEnvComboBox->addItem("Select");
  AutoParsePythonPaths();
  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnSettingsAccept()));
  connect(m_Controls.modeldirectoryBox,
          SIGNAL(directoryChanged(const QString &)),
          this,
          SLOT(OnDirectoryChanged(const QString &)));
  connect(
    m_Controls.modelBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnModelChanged(const QString &)));
  connect(m_Controls.taskBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnTaskChanged(const QString &)));
  connect(
    m_Controls.trainerBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnTrainerChanged(const QString &)));
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
          SLOT(OnPythonChanged(const QString &)));

  // qRegisterMetaType<QVector<int>>("QVector");
  connect(this, &QmitknnUNetToolGUI::Operate, m_Worker, &nnUNetSegmentationWorker::DoWork);
  connect(m_Worker, &nnUNetSegmentationWorker::Finished, this, &QmitknnUNetToolGUI::SegmentationResultHandler);
  connect(m_Worker, &nnUNetSegmentationWorker::Failed, this, &QmitknnUNetToolGUI::SegmentationProcessFailed);
  connect(m_SegmentationThread, &QThread::finished, m_Worker, &QObject::deleteLater);

  m_Controls.codedirectoryBox->setVisible(false);
  m_Controls.nnUnetdirLabel->setVisible(false);
  m_Controls.multiModalSpinBox->setVisible(false);
  m_Controls.multiModalSpinLabel->setVisible(false);

  m_Controls.statusLabel->setTextFormat(Qt::RichText);
  m_Controls.statusLabel->setText("<b>STATUS: </b><i>No Tasks Running.</i>");

  if (GetGPUCount() != 0)
  {
    m_Controls.gpuSpinBox->setMaximum(GetGPUCount() - 1);
  }
  mainLayout->addLayout(m_Controls.verticalLayout);
  Superclass::InitializeUI(mainLayout);
  m_UI_ROWS = m_Controls.advancedSettingsLayout->rowCount(); // Must do. Row count is correct only here.
}

void QmitknnUNetToolGUI::OnSettingsAccept()
{
  bool doSeg = true;
  std::cout << "clicked OnSettingsAccept" << std::endl;
  size_t hashKey(0);
  auto tool = this->GetConnectedToolAs<mitk::nnUNetTool>();
  if (nullptr != tool)
  {
    try
    {
      // comboboxes
      QString modelName = m_Controls.modelBox->currentText();
      QString taskName = m_Controls.taskBox->currentText();
      std::string nnUNetDirectory = "";
      if (m_Controls.nopipBox->isChecked())
      {
        nnUNetDirectory = m_Controls.codedirectoryBox->directory().toStdString();
      }
      QString pythonPathTextItem = m_Controls.pythonEnvComboBox->currentText();
      QString pythonPath = pythonPathTextItem.mid(pythonPathTextItem.indexOf(" ") + 1);
      if (!(pythonPath.endsWith("bin", Qt::CaseInsensitive) || pythonPath.endsWith("bin/", Qt::CaseInsensitive)))
      {
        pythonPath += QDir::separator() + QString("bin");
      }

      QString trainerPlanner = m_Controls.trainerBox->currentText();
      std::cout << "trainerPlanner: " << trainerPlanner.toStdString() << std::endl;
      QString splitterString = "__";
      tool->EnsembleOff();

      if (modelName.startsWith("ensemble", Qt::CaseInsensitive))
      {
        std::cout << "model ensemble" << std::endl;
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
      nnUNetModel *modelRequest = new nnUNetModel();
      if (tool->GetEnsemble())
      {
        std::cout << "in ensemble" << std::endl;
        foreach (QString modelSet, trainerSplitParts)
        {
          modelSet.remove("ensemble_", Qt::CaseInsensitive);
          QStringList splitParts = modelSet.split("__", QString::SplitBehavior::SkipEmptyParts);
          QString modelName = splitParts.first();
          QString trainer = splitParts.at(1);
          QString planId = splitParts.at(2);
          auto testfold = std::vector<std::string>(1, "1");
          mitk::ModelParams modelObject = MapToRequest(modelName, taskName, trainer, planId, testfold);
          modelRequest->requestQ.push_back(modelObject);
        }
      }
      else
      {
        QString trainer = trainerSplitParts.first();
        QString planId = trainerSplitParts.last();
        std::vector<std::string> fetchedFolds = FetchSelectedFoldsFromUI();
        mitk::ModelParams modelObject = MapToRequest(modelName, taskName, trainer, planId, fetchedFolds);
        modelRequest->requestQ.push_back(modelObject);
      }

      if (m_DoCache)
      {
        hashKey = modelRequest->GetUniqueHash();
        if (this->cache.contains(hashKey))
        {
          doSeg = false;
          std::cout << "Key found: " << hashKey << std::endl;
        }
      }
      if (doSeg)
      {
        tool->m_ParamQ.clear();
        tool->m_ParamQ = modelRequest->requestQ;
      }

      tool->SetnnUNetDirectory(nnUNetDirectory);
      tool->SetPythonPath(pythonPath.toStdString());
      tool->SetModelDirectory(m_ModelDirectory.left(m_ModelDirectory.lastIndexOf(QDir::separator())).toStdString());
      // checkboxes
      tool->SetMirror(m_Controls.mirrorBox->isChecked());
      tool->SetMixedPrecision(m_Controls.mixedPrecisionBox->isChecked());
      tool->SetNoPip(m_Controls.nopipBox->isChecked());
      tool->SetMultiModal(m_Controls.multiModalBox->isChecked());
      // Spinboxes
      tool->SetPreprocessingThreads(static_cast<unsigned int>(m_Controls.threadsBox->value()));
      tool->SetGpuId(static_cast<unsigned int>(m_Controls.gpuSpinBox->value()));
      // Multi-Modal
      tool->MultiModalOff();
      if (m_Controls.multiModalBox->isChecked())
      {
        tool->otherModalPaths.clear();
        tool->otherModalPaths = FetchMultiModalPathsFromUI();
        tool->MultiModalOn();
      }

      if (doSeg)
      {
        std::cout << "do segmentation" << std::endl;
        if (!m_SegmentationThread->isRunning())
        {
          std::cout << "starting thread..." << std::endl;
          m_SegmentationThread->start();
        }
        m_Controls.statusLabel->setText(
          "<b>STATUS: </b><i>Startting Segmentation task... This might take a while.</i>");
        emit Operate(tool, modelRequest); // start segmentation in worker thread
      }
      else
      {
        delete modelRequest;
        std::cout << "won't do segmentation. Key found: " << QString::number(hashKey).toStdString() << std::endl;
        if (this->cache.contains(hashKey))
        {
          nnUNetModel *_model = this->cache[hashKey];
          // std::cout << "fetched pointer " << _model->outputImage << std::endl;
          this->SetLabelSetPreview(_model->outputImage);
        }
      }
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
    // tool->IsTimePointChangeAwareOn();
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

int QmitknnUNetToolGUI::GetGPUCount()
{
#if defined(__APPLE__) || defined(MACOSX) || defined(linux) || defined(__linux__)
  QProcess process1, process2;
  process1.setStandardOutputProcess(&process2);
  process1.start("nvidia-smi -L");
  process2.start("wc -l");
  process1.waitForFinished(-1);
  process2.waitForFinished(-1);
  QString nGpus = process2.readAll();
#elif defined(_WIN32) || defined(_WIN64)
  QString nGpus = "0";
#endif
  return nGpus.toInt();
}
