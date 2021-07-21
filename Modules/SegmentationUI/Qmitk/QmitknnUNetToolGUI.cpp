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
  /*m_Controls.modelBox->addItem("3D Full Res");
  m_Controls.modelBox->addItem("3D Low Res");
  m_Controls.modelBox->addItem("3D Cascade Full Res");
  m_Controls.modelBox->addItem("2D");

  m_Controls.taskBox->addItem("Heart");
  m_Controls.taskBox->addItem("Brain Tumour");
  m_Controls.taskBox->addItem("Prostate");*/

  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnSettingsAccept()));
  connect(m_Controls.modeldirectoryBox,
          SIGNAL(directoryChanged(const QString &)),
          this,
          SLOT(OnDirectoryChanged(const QString &)));
  connect(
    m_Controls.modelBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnModelChanged(const QString &)));
  connect(
    m_Controls.trainerBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnTrainerChanged(const QString &)));
  mainLayout->addLayout(m_Controls.verticalLayout);

  Superclass::InitializeUI(mainLayout);
}

void QmitknnUNetToolGUI::OnSettingsAccept()
{
  auto tool = this->GetConnectedToolAs<mitk::nnUNetTool>();
  if (nullptr != tool)
  {
    try
    {
      // comboboxes
      m_Model = m_Controls.modelBox->itemText(m_Controls.modelBox->currentIndex()).toUtf8().constData();
      m_Task = m_Controls.taskBox->itemText(m_Controls.taskBox->currentIndex()).toUtf8().constData();
      m_nnUNetDirectory = m_Controls.codedirectoryBox->directory().toUtf8().constData();
      std::string modelDirectory = m_ModelDirectory.toUtf8().constData();
      m_OutputDirectory = m_Controls.outdirBox->directory().toUtf8().constData();
      std::string fold = m_Controls.foldBox->itemText(m_Controls.foldBox->currentIndex()).toUtf8().constData();

      tool->SetModel(m_Model);
      tool->SetTask(m_Task);
      tool->SetnnUNetDirectory(m_nnUNetDirectory);
      tool->SetOutputDirectory(m_OutputDirectory);
      tool->SetModelDirectory(modelDirectory);
      tool->SetFold(fold);

      // checkboxes
      tool->SetUseGPU(m_Controls.gpuBox->isChecked());
      // tool->SetLowRes(m_Controls.lowresBox->isChecked());
      tool->SetAllInGPU(m_Controls.allInGpuBox->isChecked());
      tool->SetExportSegmentation(m_Controls.exportBox->isChecked());
      tool->SetMirror(m_Controls.mirrorBox->isChecked());
      tool->SetMixedPrecision(m_Controls.mixedPrecisionBox->isChecked());

      // Spinboxes
      tool->SetPreprocessingThreads(static_cast<unsigned int>(m_Controls.threadsBox->value()));

      tool->UpdatePreview();
    }
    catch (const std::exception &e)
    {
      this->setCursor(Qt::ArrowCursor);
      std::stringstream stream;
      stream << "Error while generation nnUNet segmentation. Reason: " << e.what();
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

void QmitknnUNetToolGUI::OnDirectoryChanged(const QString &dir)
{
  QStringList splitPath = dir.split(
    QDir::separator(), QString::SplitBehavior::SkipEmptyParts); // Should work for all OS but not tested on Windows
  QString task = splitPath.last();
  m_Controls.taskBox->addItem(task);
  // std::vector<QString> models;
  QDirIterator it(dir, QDir::AllDirs, QDirIterator::NoIteratorFlags);
  while (it.hasNext())
  {
    it.next();
    QString filePath = it.fileName();
    // models.push_back(filePath);
    if (!filePath.startsWith('.'))
    { // Filter out irrelevent hidden folders, if any.
      m_Controls.modelBox->addItem(filePath);
    }
  }
}

void QmitknnUNetToolGUI::OnModelChanged(const QString &text)
{
  std::cout << "IN OnModelChanged" << std::endl;
  m_ModelDirectory = m_Controls.modeldirectoryBox->directory(); // check syntax
  QString updatedPath(QDir::cleanPath(m_ModelDirectory + QDir::separator() + text));
  std::cout << "updatedPath " << updatedPath.toUtf8().constData() << std::endl;
  // QString dataset_name;
  for (QDirIterator it(updatedPath, QDir::AllDirs, QDirIterator::NoIteratorFlags); it.hasNext();)
  {
    it.next();
    if (!it.fileName().startsWith('.'))
    {
      m_DatasetName = it.fileName();
      std::cout << "dataset_name " << m_DatasetName.toUtf8().constData() << std::endl;
    }
  }
  updatedPath = QDir::cleanPath(updatedPath + QDir::separator() + m_DatasetName);
  std::cout << "new updatedPath " << updatedPath.toUtf8().constData() << std::endl;
  // std::vector<QString> trainers;
  m_Controls.trainerBox->clear();
  for (QDirIterator it(updatedPath, QDir::AllDirs, QDirIterator::NoIteratorFlags); it.hasNext();)
  {
    it.next();
    QString trainer = it.fileName();
    if (!trainer.startsWith('.'))
    { // Filter out irrelevent hidden folders, if any.
      // trainers.push_back(trainer);
      std::cout << "trainer " << trainer.toUtf8().constData() << std::endl;
      m_Controls.trainerBox->addItem(trainer);
    }
  }
}

void QmitknnUNetToolGUI::OnTrainerChanged(const QString &trainerSelected)
{
  std::cout << "IN trainer Changed" << std::endl;
  m_ModelDirectory = m_Controls.modeldirectoryBox->directory(); // check syntax
  QString updatedPath(QDir::cleanPath(m_ModelDirectory + QDir::separator() + m_Controls.modelBox->currentText() +
                                      QDir::separator() + m_DatasetName + QDir::separator() + trainerSelected));
  std::cout << "updatedPath " << updatedPath.toUtf8().constData() << std::endl;
  // std::vector<QString> folds;
  m_Controls.foldBox->clear();
  for (QDirIterator it(updatedPath, QDir::AllDirs, QDirIterator::NoIteratorFlags); it.hasNext();)
  {
    it.next();
    QString fold = it.fileName();
    if (!fold.startsWith('.'))
    { // Filter out irrelevent hidden folders, if any.
      // folds.push_back(fold);
      std::cout << "fold " << fold.toUtf8().constData() << std::endl;
      m_Controls.foldBox->addItem(fold);
    }
  }
}