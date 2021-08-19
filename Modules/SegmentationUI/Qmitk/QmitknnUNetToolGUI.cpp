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
#include <QProcess>
#include <QtGlobal>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitknnUNetToolGUI, "")

QmitknnUNetToolGUI::QmitknnUNetToolGUI() : QmitkAutoMLSegmentationToolGUIBase() {}

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
  m_Controls.codedirectoryBox->setVisible(false);
  m_Controls.nnUnetdirLabel->setVisible(false);
  m_Controls.multiModalSpinBox->setVisible(false);
  m_Controls.multiModalSpinLabel->setVisible(false);
  mainLayout->addLayout(m_Controls.verticalLayout);
  Superclass::InitializeUI(mainLayout);
}

void QmitknnUNetToolGUI::OnSettingsAccept()
{
  bool doSeg = true;
  std::cout << "clicked" << std::endl;
  size_t hashKey(0);
  auto tool = this->GetConnectedToolAs<mitk::nnUNetTool>();
  if (nullptr != tool)
  {
    try
    {
      // comboboxes
      m_Model = m_Controls.modelBox->currentText();
      m_Task = m_Controls.taskBox->currentText();
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

      if (m_Model.startsWith("ensemble", Qt::CaseInsensitive))
      {
        std::cout << "model ensemble" << std::endl;
        QString ppJsonFile =
          QDir::cleanPath(m_ModelDirectory + QDir::separator() + m_Model + QDir::separator() + m_DatasetName +
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
          mitk::ModelParams modelObject = MapToRequest(modelName, m_Task, trainer, planId, testfold);
          modelRequest->requestQ.push_back(modelObject);
        }
      }
      else
      {
        QString trainer = trainerSplitParts.first();
        QString planId = trainerSplitParts.last();
        std::vector<std::string> fetchedFolds = FetchSelectedFoldsFromUI();
        mitk::ModelParams modelObject = MapToRequest(m_Model, m_Task, trainer, planId, fetchedFolds);
        modelRequest->requestQ.push_back(modelObject);
      }
      hashKey = modelRequest->GetUniqueHash();
      if (this->cache.contains(hashKey))
      {
        doSeg = false;
        std::cout << "Key found: " << hashKey << std::endl;
      }
      if (doSeg)
      {
        tool->m_ParamQ.clear();
        tool->m_ParamQ = modelRequest->requestQ;
      }

      tool->SetnnUNetDirectory(nnUNetDirectory);
      tool->SetPythonPath(pythonPath.toStdString());
      tool->SetModelDirectory(m_ModelDirectory.toStdString());
      // checkboxes
      tool->SetMirror(m_Controls.mirrorBox->isChecked());
      tool->SetMixedPrecision(m_Controls.mixedPrecisionBox->isChecked());
      tool->SetNoPip(m_Controls.nopipBox->isChecked());
      tool->SetMultiModal(m_Controls.multiModalBox->isChecked());

      // Spinboxes
      tool->SetPreprocessingThreads(static_cast<unsigned int>(m_Controls.threadsBox->value()));
      if (doSeg)
      {
        std::cout << "do segmentation" << std::endl;
        tool->UpdatePreview();
        this->SetLabelSetPreview(tool->GetMLPreview());
        std::cout << "New pointer: " << tool->GetMLPreview() << std::endl;
        modelRequest->outputImage = tool->GetMLPreview();
        // Adding params and output Labelset image to Cache
        size_t hashkey = modelRequest->GetUniqueHash();
        std::cout << "New hash: " << hashkey << std::endl;
        this->cache.insert(hashkey, modelRequest);
      }
      else
      {
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
    tool->IsTimePointChangeAwareOn();
  }
}

/*std::vector<QString> QmitknnUNetToolGUI::FetchFoldersFromDir(const QString &path)
{
  std::vector<QString> folders;
  for (QDirIterator it(path, QDir::AllDirs, QDirIterator::NoIteratorFlags); it.hasNext();)
  {
    it.next();
    if (!it.fileName().startsWith('.'))
    {
      folders.push_back(it.fileName());
    }
  }
  return folders;
}*/
