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
  m_Controls.multiModalPathLabel->setVisible(false);

  mainLayout->addLayout(m_Controls.verticalLayout);
  Superclass::InitializeUI(mainLayout);
}

void QmitknnUNetToolGUI::OnSettingsAccept()
{
  auto tool = this->GetConnectedToolAs<mitk::nnUNetTool>();
  if (nullptr != tool)
  {
    try
    {  mitk::ModelParams modelObject;
      // comboboxes
      m_Model = m_Controls.modelBox->itemText(m_Controls.modelBox->currentIndex()).toUtf8().constData();
      m_Task = m_Controls.taskBox->itemText(m_Controls.taskBox->currentIndex()).toUtf8().constData();
      std::string nnUNetDirectory = "";
      if (m_Controls.multiModalBox->isChecked())
      {
        nnUNetDirectory = m_Controls.codedirectoryBox->directory().toUtf8().constData();
      }
      QString pythonPathTextItem = m_Controls.pythonEnvComboBox->itemText(m_Controls.pythonEnvComboBox->currentIndex());
      QString pythonPath = pythonPathTextItem.mid(pythonPathTextItem.indexOf(" ") + 1);
      if (!(pythonPath.endsWith("bin", Qt::CaseInsensitive) || pythonPath.endsWith("bin/", Qt::CaseInsensitive)))
      {
        pythonPath += QDir::separator() + QString("bin");
      }
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
      QString trainerPlanner = m_Controls.trainerBox->itemText(m_Controls.trainerBox->currentIndex());
      QStringList splitParts = trainerPlanner.split("__", QString::SplitBehavior::SkipEmptyParts);
      QString trainer = splitParts.first();
      QString planId = splitParts.last();

      tool->SetModel(m_Model);
      modelObject.m_Model = m_Model;
      tool->SetTask(m_Task);
      modelObject.m_Task = m_Task;
      tool->SetnnUNetDirectory(nnUNetDirectory);
      tool->SetPythonPath(pythonPath.toUtf8().constData());
      tool->SetModelDirectory(m_ModelDirectory.toUtf8().constData());
      tool->m_Folds = folds;
      modelObject.m_Folds = folds;
      tool->SetTrainer(trainer.toUtf8().constData());
      modelObject.m_Trainer = trainer.toUtf8().constData();
      tool->SetPlanId(planId.toUtf8().constData());
      modelObject.m_PlanId = planId.toUtf8().constData();

      tool->params.push_back(modelObject);

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

      tool->UpdatePreview();
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

    this->SetLabelSetPreview(tool->GetMLPreview());
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
    if (!filePath.startsWith('.') && !filePath.startsWith("ensemble"))
    { // Filter out irrelevent hidden folders, if any.
      m_Controls.modelBox->addItem(filePath);
    }
  }
}

void QmitknnUNetToolGUI::OnModelChanged(const QString &text)
{
  m_ModelDirectory = m_Controls.modeldirectoryBox->directory();
  QString updatedPath(QDir::cleanPath(m_ModelDirectory + QDir::separator() + text));
  // QString dataset_name;
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