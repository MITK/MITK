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
#include <QRandomGenerator>
#include <QStringListModel>
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
  connect(
    m_Controls.trainerBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnTrainerChanged(const QString &)));
  connect(m_Controls.nopipBox, SIGNAL(stateChanged(int)), this, SLOT(OnCheckBoxChanged(int)));
  connect(m_Controls.multiModalBox, SIGNAL(stateChanged(int)), this, SLOT(OnCheckBoxChanged(int)));
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
  m_Controls.multiModalPath->setVisible(false);
  m_Controls.multiModalLabel->setVisible(false);
  m_Controls.multiModalBox->setVisible(false);
  m_Controls.multiModalPathLabel->setVisible(false);

  mainLayout->addLayout(m_Controls.verticalLayout);
  Superclass::InitializeUI(mainLayout);
}

void QmitknnUNetToolGUI::OnSettingsAccept()
{
  bool doSeg = true;
  std::cout << "clicked" << std::endl;
  quint32 keyFound(0);
  quint32 requestId = QRandomGenerator::global()->generate();
  auto tool = this->GetConnectedToolAs<mitk::nnUNetTool>();
  if (nullptr != tool)
  {
    try
    {
      tool->m_Params.clear();
      // comboboxes
      m_Model = m_Controls.modelBox->currentText();
      m_Task = m_Controls.taskBox->currentText().toUtf8().constData();
      std::string nnUNetDirectory = "";
      if (m_Controls.multiModalBox->isChecked())
      {
        nnUNetDirectory = m_Controls.codedirectoryBox->directory().toUtf8().constData();
      }
      QString pythonPathTextItem = m_Controls.pythonEnvComboBox->currentText();
      QString pythonPath = pythonPathTextItem.mid(pythonPathTextItem.indexOf(" ") + 1);
      if (!(pythonPath.endsWith("bin", Qt::CaseInsensitive) || pythonPath.endsWith("bin/", Qt::CaseInsensitive)))
      {
        pythonPath += QDir::separator() + QString("bin");
      }

      QString trainerPlanner = m_Controls.trainerBox->currentText();
      if (m_Model.startsWith("ensemble", Qt::CaseInsensitive))
      {
        QString ppJsonFile =
          QDir::cleanPath(m_ModelDirectory + QDir::separator() + m_Model + QDir::separator() + m_DatasetName +
                          QDir::separator() + trainerPlanner + QDir::separator() + "postprocessing.json");
        if (QFile(ppJsonFile).exists())
        {
          tool->EnsembleOn();
          QStringList models = trainerPlanner.split("--", QString::SplitBehavior::SkipEmptyParts);
          foreach (QString model, models)
          {
            model.remove("ensemble_", Qt::CaseInsensitive);
            mitk::ModelParams modelObject;
            QStringList splitParts = model.split("__", QString::SplitBehavior::SkipEmptyParts);
            QString modelName = splitParts.first();
            QString trainer = splitParts.at(1);
            QString planId = splitParts.at(2);

            modelObject.m_Model = modelName.toUtf8().constData();
            modelObject.m_Trainer = trainer.toUtf8().constData();
            modelObject.m_PlanId = planId.toUtf8().constData();
            modelObject.m_Task = m_Task;
            modelObject.m_Folds = std::vector<std::string>(1, "1"); // only for testing
            tool->m_Params.push_back(modelObject);
          }
          tool->SetPostProcessingJsonDirectory(ppJsonFile.toStdString());
        }
        else
        {
          std::cout << "post proc file not found" << std::endl;
          // throw exception with message
        }
      }
      else
      {
        mitk::ModelParams modelObject;
        QStringList splitParts = trainerPlanner.split("__", QString::SplitBehavior::SkipEmptyParts);
        QString trainer = splitParts.first();
        QString planId = splitParts.last();

        // tool->SetModel(m_Model);
        modelObject.m_Model = m_Model.toUtf8().constData();
        // tool->SetTask(m_Task);
        modelObject.m_Task = m_Task;
        // tool->m_Folds = folds;
        modelObject.m_Folds = FetchFoldsFromUI();
        // tool->SetTrainer(trainer.toUtf8().constData());
        modelObject.m_Trainer = trainer.toUtf8().constData();
        // tool->SetPlanId(planId.toUtf8().constData());
        modelObject.m_PlanId = planId.toUtf8().constData();

        QList<quint32> keyList = this->cache.keys();
        foreach (quint32 key, keyList)
        {
          nnUNetModel *value = this->cache[key];
          // if (value->request.m_Model == modelObject.m_Model)
          if (value->request == modelObject)
          {
            doSeg = false;
            keyFound = key;
            std::cout << "Key found: " << QString::number(key).toStdString() << std::endl;
            break;
          }
        }
        if (doSeg)
        {
          tool->m_Params.clear();
          tool->m_Params.push_back(modelObject);
        }
      }

      tool->SetnnUNetDirectory(nnUNetDirectory);
      tool->SetPythonPath(pythonPath.toUtf8().constData());
      tool->SetModelDirectory(m_ModelDirectory.toUtf8().constData());
      // checkboxes
      // tool->SetUseGPU(m_Controls.gpuBox->isChecked());
      // tool->SetLowRes(m_Controls.lowresBox->isChecked());
      // tool->SetAllInGPU(m_Controls.allInGpuBox->isChecked());
      // tool->SetExportSegmentation(m_Controls.exportBox->isChecked());
      tool->SetMirror(m_Controls.mirrorBox->isChecked());
      tool->SetMixedPrecision(m_Controls.mixedPrecisionBox->isChecked());
      tool->SetNoPip(m_Controls.nopipBox->isChecked());
      tool->SetMultiModal(m_Controls.multiModalBox->isChecked());

      // Spinboxes
      tool->SetPreprocessingThreads(static_cast<unsigned int>(m_Controls.threadsBox->value()));
      if (doSeg)
      {
        tool->UpdatePreview();
        this->SetLabelSetPreview(tool->GetMLPreview());
        std::cout << "New pointer: " << tool->GetMLPreview() << std::endl;
        std::cout << "New request: " << QString::number(requestId).toStdString() << std::endl;
        // Adding params and output Labelset image to Cache
        nnUNetModel *modelRequest = new nnUNetModel(tool->GetMLPreview());
        modelRequest->request = tool->m_Params[0];
        std::cout << "New model " << modelRequest->request.m_Model << std::endl;
        this->cache.insert(requestId, modelRequest);
      }
      else if (keyFound != 0)
      {
        std::cout << "won't do segmentation. Key found: " << QString::number(keyFound).toStdString() << std::endl;
        if (this->cache.contains(keyFound))
        {
          nnUNetModel *_model = this->cache[keyFound];
          std::cout << "fetched pointer " << _model->outputImage << std::endl;
          std::cout << "fetched model " << _model->request.m_Model << std::endl;

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

    // this->SetLabelSetPreview(tool->GetMLPreview());
    tool->IsTimePointChangeAwareOn();
  }
}

void QmitknnUNetToolGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
  m_Controls.previewButton->setEnabled(enabled);
}

void QmitknnUNetToolGUI::ClearAllComboBoxes()
{
  m_Controls.modelBox->clear();
  m_Controls.taskBox->clear();
  m_Controls.foldBox->clear();
  m_Controls.trainerBox->clear();
}

void QmitknnUNetToolGUI::OnDirectoryChanged(const QString &dir)
{
  this->ClearAllComboBoxes();
  // std::vector<QString> models;
  QDirIterator it(dir, QDir::AllDirs, QDirIterator::NoIteratorFlags);
  while (it.hasNext())
  {
    it.next();
    QString filePath = it.fileName();
    // models.push_back(filePath);
    if (!filePath.startsWith('.')) //&& !filePath.startsWith("ensemble"))
    {                              // Filter out irrelevent hidden folders, if any.
      m_Controls.modelBox->addItem(filePath);
    }
  }
}

void QmitknnUNetToolGUI::OnModelChanged(const QString &text)
{
  m_ModelDirectory = m_Controls.modeldirectoryBox->directory();
  QString updatedPath(QDir::cleanPath(m_ModelDirectory + QDir::separator() + text));
  m_Controls.taskBox->clear();
  for (QDirIterator it(updatedPath, QDir::AllDirs, QDirIterator::NoIteratorFlags); it.hasNext();)
  {
    it.next();
    if (!it.fileName().startsWith('.'))
    {
      m_DatasetName = it.fileName();
      m_Controls.taskBox->addItem(m_DatasetName);
    }
  }
  updatedPath = QDir::cleanPath(updatedPath + QDir::separator() + m_DatasetName);
  // std::vector<QString> trainers;
  m_Controls.trainerBox->clear();
  for (QDirIterator it(updatedPath, QDir::AllDirs, QDirIterator::NoIteratorFlags); it.hasNext();)
  {
    it.next();
    QString trainer = it.fileName();
    if (!trainer.startsWith('.')) // Filter out irrelevent hidden folders, if any.
    {                             // trainers.push_back(trainer);
      m_Controls.trainerBox->addItem(trainer);
    }
  }
}

void QmitknnUNetToolGUI::OnTrainerChanged(const QString &trainerSelected)
{
  if (m_Controls.modelBox->currentText() != "ensembles")
  {
    m_ModelDirectory = m_Controls.modeldirectoryBox->directory(); // check syntax
    QString updatedPath(QDir::cleanPath(m_ModelDirectory + QDir::separator() + m_Controls.modelBox->currentText() +
                                        QDir::separator() + m_DatasetName + QDir::separator() + trainerSelected));
    // std::vector<QString> folds;
    m_Controls.foldBox->clear();
    for (QDirIterator it(updatedPath, QDir::AllDirs, QDirIterator::NoIteratorFlags); it.hasNext();)
    {
      it.next();
      QString fold = it.fileName();
      if (!fold.startsWith('.')) // Filter out irrelevent hidden folders, if any.
      {                          // folds.push_back(fold);
        m_Controls.foldBox->addItem(fold);
      }
    }
  }
  else
  {
    m_Controls.foldBox->clear();
  }
}

void QmitknnUNetToolGUI::OnPythonChanged(const QString &pyEnv)
{
  if (pyEnv == QString("Select"))
  {
    QString path =
      QFileDialog::getExistingDirectory(m_Controls.pythonEnvComboBox->parentWidget(), "Python Path", "dir");
    if (!path.isEmpty())
    {
      m_Controls.pythonEnvComboBox->insertItem(0, path);
      m_Controls.pythonEnvComboBox->setCurrentIndex(0);
    }
  }
}

void QmitknnUNetToolGUI::OnCheckBoxChanged(int state)
{
  bool visibility = false;
  if (state == Qt::Checked)
  {
    visibility = true;
  }
  ctkCheckBox *box = qobject_cast<ctkCheckBox *>(sender());
  if (box != nullptr)
  {
    if (box->objectName() == QString("nopipBox"))
    {
      m_Controls.codedirectoryBox->setVisible(visibility);
      m_Controls.nnUnetdirLabel->setVisible(visibility);
    }
    else if (box->objectName() == QString("multiModalBox"))
    {
      m_Controls.multiModalPath->setVisible(visibility);
      m_Controls.multiModalPathLabel->setVisible(visibility);
    }
  }
}

void QmitknnUNetToolGUI::AutoParsePythonPaths()
{
  QString homeDir = QDir::homePath();
  std::vector<QString> searchDirs;
#if defined(__APPLE__) || defined(MACOSX) || defined(linux) || defined(__linux__)
  // Add search locations for possible standard python paths here
  searchDirs.push_back(homeDir + QDir::separator() + "environments");
  searchDirs.push_back(homeDir + QDir::separator() + "anaconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "miniconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "opt" + QDir::separator() + "miniconda3");
  searchDirs.push_back(homeDir + QDir::separator() + "opt" + QDir::separator() + "anaconda3");
#elif defined(_WIN32) || defined(_WIN64)
  searchDirs.push_back("C:" + QDir::separator() + "anaconda3");
#endif
  for (QString searchDir : searchDirs)
  {
    if (searchDir.endsWith("anaconda3", Qt::CaseInsensitive))
    {
      if (QDir(searchDir).exists()) // Do case insensitive search
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
      {                             // folds.push_back(fold);
        m_Controls.pythonEnvComboBox->insertItem(0, "(" + envName + "): " + subIt.filePath());
      }
    }
  }
  m_Controls.pythonEnvComboBox->setCurrentIndex(-1);
}

std::vector<std::string> QmitknnUNetToolGUI::FetchFoldsFromUI()
{
  std::vector<std::string> folds;
  if (!(m_Controls.foldBox->allChecked() || m_Controls.foldBox->noneChecked()))
  {
    QModelIndexList foldList = m_Controls.foldBox->checkedIndexes();
    foreach (QModelIndex index, foldList)
    {
      QString foldQString =
        m_Controls.foldBox->itemText(index.row()).split("_", QString::SplitBehavior::SkipEmptyParts).last();
      folds.push_back(foldQString.toUtf8().constData());
    }
  }
  return folds;
}