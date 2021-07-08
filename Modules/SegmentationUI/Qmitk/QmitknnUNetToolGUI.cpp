/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitknnUNetToolGUI.h"

#include "mitknnUnetTool.h"
#include <QMessageBox>
#include <QProcess>
#include <QDir>
#include <QDirIterator>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitknnUNetToolGUI, "")

QmitknnUNetToolGUI::QmitknnUNetToolGUI() : QmitkAutoMLSegmentationToolGUIBase()                                         
{}

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
  connect(m_Controls.modeldirectoryBox, SIGNAL(directoryChanged(const QString &)), this, SLOT(OnDirectoryChanged(const QString &)));

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
      m_ModelDirectory = m_Controls.modeldirectoryBox->directory().toUtf8().constData();
      m_OutputDirectory = m_Controls.outdirBox->directory().toUtf8().constData();
      tool->SetModel(m_Model);
      tool->SetTask(m_Task);
      tool->SetnnUNetDirectory(m_nnUNetDirectory);
      tool->SetOutputDirectory(m_OutputDirectory);
      tool->SetModelDirectory(m_ModelDirectory);

      // checkboxes
      tool->SetUseGPU(m_Controls.gpuBox->isChecked());
      //tool->SetLowRes(m_Controls.lowresBox->isChecked());
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

void QmitknnUNetToolGUI::OnDirectoryChanged(const QString &dir )
{
  QStringList splitPath = dir.split(QDir::separator(), Qt::SkipEmptyParts); // Should work for all OS but not tested on Windows
  QString task = splitPath.last();
  m_Controls.taskBox->addItem(task);
  //std::vector<QString> models;
  QDirIterator it(dir , QDir::AllDirs , QDirIterator::NoIteratorFlags);
  while (it.hasNext()) {
    it.next();
    QString filePath = it.fileName();
    //models.push_back(filePath);
    if (!filePath.startsWith('.')){ //Filter out irrelevent hidden folders, if any.
      m_Controls.modelBox->addItem(filePath);
    } 
  }
}