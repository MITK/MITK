/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitknnUNetToolGUI.h"

#include "mitknnUnetTool.h"
#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QtGlobal>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitknnUNetToolGUI, "")

QmitknnUNetToolGUI::QmitknnUNetToolGUI() : QmitkAutoMLSegmentationToolGUIBase()
{
  // Nvidia-smi command returning zero doesn't alway mean lack of GPUs.
  // Pytorch uses its own libraries to communicate to the GPUs. Hence, only a warning can be given.
  if (m_GpuLoader.GetGPUCount() == 0)
  {
    std::string warning = "WARNING: No GPUs were detected on your machine. The nnUNet tool might not work.";
    ShowErrorMessage(warning);
  }

  // define predicates for multi modal data selection combobox
  auto imageType = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto labelSetImageType = mitk::NodePredicateNot::New(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  this->m_MultiModalPredicate = mitk::NodePredicateAnd::New(imageType, labelSetImageType).GetPointer();
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
  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnPreviewRequested()));
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
  connect(m_Controls.posSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnModalPositionChanged(int)));
  connect(m_Controls.pythonEnvComboBox,
#if QT_VERSION >= 0x050F00 // 5.15
          SIGNAL(textActivated(const QString &)),
#elif QT_VERSION >= 0x050C00 // 5.12
          SIGNAL(activated(const QString &)),
#endif
          this,
          SLOT(OnPythonPathChanged(const QString &)));
  connect(m_Controls.refreshdirectoryBox, SIGNAL(clicked()), this, SLOT(OnRefreshPresssed()));

  m_Controls.codedirectoryBox->setVisible(false);
  m_Controls.nnUnetdirLabel->setVisible(false);
  m_Controls.multiModalSpinBox->setVisible(false);
  m_Controls.multiModalSpinLabel->setVisible(false);
  m_Controls.posSpinBoxLabel->setVisible(false);
  m_Controls.posSpinBox->setVisible(false);
  m_Controls.previewButton->setEnabled(false);

  QSize sz(16, 16);
  m_Controls.stopButton->setVisible(false);

  QIcon refreshIcon;
  refreshIcon.addPixmap(style()->standardIcon(QStyle::SP_BrowserReload).pixmap(sz), QIcon::Normal, QIcon::Off);
  m_Controls.refreshdirectoryBox->setIcon(refreshIcon);
  m_Controls.refreshdirectoryBox->setEnabled(true);

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

void QmitknnUNetToolGUI::OnPreviewRequested()
{
  mitk::nnUNetTool::Pointer tool = this->GetConnectedToolAs<mitk::nnUNetTool>();
  if (nullptr != tool)
  {
    try
    {
      m_Controls.previewButton->setEnabled(false); // To prevent misclicked back2back prediction.
      qApp->processEvents();
      tool->PredictOn(); // purposefully placed to make tool->GetMTime different than before.
      QString modelName = m_Controls.modelBox->currentText();
      if (modelName.startsWith("ensemble", Qt::CaseInsensitive))
      {
        ProcessEnsembleModelsParams(tool);
      }
      else
      {
        ProcessModelParams(tool);
      }
      QString pythonPathTextItem = m_Controls.pythonEnvComboBox->currentText();
      QString pythonPath = pythonPathTextItem.mid(pythonPathTextItem.indexOf(" ") + 1);
      bool isNoPip = m_Controls.nopipBox->isChecked();
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

      tool->SetnnUNetDirectory(nnUNetDirectory);
      tool->SetPythonPath(pythonPath.toStdString());
      tool->SetModelDirectory(m_ParentFolder->getResultsFolder().toStdString());
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
        tool->m_OtherModalPaths = FetchMultiModalImagesFromUI();
        tool->MultiModalOn();
      }
      tool->m_InputOutputPair = std::make_pair(nullptr, nullptr);
      m_Controls.statusLabel->setText("<b>STATUS: </b><i>Starting Segmentation task... This might take a while.</i>");
      tool->UpdatePreview();
      if (tool->GetOutputBuffer() == nullptr)
      {
        SegmentationProcessFailed();
      }
      SegmentationResultHandler(tool);
      tool->PredictOff(); // purposefully placed to make tool->GetMTime different than before.
      m_Controls.previewButton->setEnabled(true);
    }
    catch (const std::exception &e)
    {
      std::stringstream errorMsg;
      errorMsg << "Error while processing parameters for nnUNet segmentation. Reason: " << e.what();
      ShowErrorMessage(errorMsg.str());
      m_Controls.previewButton->setEnabled(true);
      tool->PredictOff();
      return;
    }
    catch (...)
    {
      std::string errorMsg = "Unkown error occured while generation nnUNet segmentation.";
      ShowErrorMessage(errorMsg);
      m_Controls.previewButton->setEnabled(true);
      tool->PredictOff();
      return;
    }
  }
}

std::vector<mitk::Image::ConstPointer> QmitknnUNetToolGUI::FetchMultiModalImagesFromUI()
{
  std::vector<mitk::Image::ConstPointer> paths;
  if (m_Controls.multiModalBox->isChecked() && !m_Modalities.empty())
  {
    for (QmitkDataStorageComboBox *modality : m_Modalities)
    {
      if (modality->objectName() != "multiModal_0")
      {
        paths.push_back(dynamic_cast<const mitk::Image *>(modality->GetSelectedNode()->GetData()));
      }
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
  fullPath = fullPath.mid(fullPath.indexOf(" ") + 1);
  return QFile::exists(fullPath + QDir::separator() + QString("nnUNet_predict"));
}

void QmitknnUNetToolGUI::ShowErrorMessage(const std::string &message, QMessageBox::Icon icon)
{
  this->setCursor(Qt::ArrowCursor);
  QMessageBox *messageBox = new QMessageBox(icon, nullptr, message.c_str());
  messageBox->exec();
  delete messageBox;
  MITK_WARN << message;
}

void QmitknnUNetToolGUI::ProcessEnsembleModelsParams(mitk::nnUNetTool::Pointer tool)
{
  if (m_EnsembleParams[0]->modelBox->currentText() == m_EnsembleParams[1]->modelBox->currentText())
  {
    throw std::runtime_error("Both models you have selected for ensembling are the same.");
  }
  QString taskName = m_Controls.taskBox->currentText();
  std::vector<mitk::ModelParams> requestQ;
  QString ppDirFolderNamePart1 = "ensemble_";
  QStringList ppDirFolderNameParts;
  for (std::unique_ptr<QmitknnUNetTaskParamsUITemplate> &layout : m_EnsembleParams)
  {
    QStringList ppDirFolderName;
    QString modelName = layout->modelBox->currentText();
    ppDirFolderName << modelName;
    ppDirFolderName << "__";
    QString trainer = layout->trainerBox->currentText();
    ppDirFolderName << trainer;
    ppDirFolderName << "__";
    QString planId = layout->plannerBox->currentText();
    ppDirFolderName << planId;

    if (!IsModelExists(modelName, taskName, QString(trainer + "__" + planId)))
    {
      std::string errorMsg = "The configuration " + modelName.toStdString() +
                             " you have selected doesn't exist. Check your Results Folder again.";
      throw std::runtime_error(errorMsg);
    }
    std::vector<std::string> testfold; // empty vector to consider all folds for inferencing.
    mitk::ModelParams modelObject = MapToRequest(modelName, taskName, trainer, planId, testfold);
    requestQ.push_back(modelObject);
    ppDirFolderNameParts << ppDirFolderName.join(QString(""));
  }
  tool->EnsembleOn();

  QString ppJsonFilePossibility1 = QDir::cleanPath(
    m_ParentFolder->getResultsFolder() + QDir::separator() + "nnUNet" + QDir::separator() + "ensembles" +
    QDir::separator() + taskName + QDir::separator() + ppDirFolderNamePart1 + ppDirFolderNameParts.first() + "--" +
    ppDirFolderNameParts.last() + QDir::separator() + "postprocessing.json");
  QString ppJsonFilePossibility2 = QDir::cleanPath(
    m_ParentFolder->getResultsFolder() + QDir::separator() + "nnUNet" + QDir::separator() + "ensembles" +
    QDir::separator() + taskName + QDir::separator() + ppDirFolderNamePart1 + ppDirFolderNameParts.last() + "--" +
    ppDirFolderNameParts.first() + QDir::separator() + "postprocessing.json");

  if (QFile(ppJsonFilePossibility1).exists())
  {
    tool->SetPostProcessingJsonDirectory(ppJsonFilePossibility1.toStdString());
  }
  else if (QFile(ppJsonFilePossibility2).exists())
  {
    tool->SetPostProcessingJsonDirectory(ppJsonFilePossibility2.toStdString());
  }
  else
  {
    // warning message
  }
  tool->m_ParamQ.clear();
  tool->m_ParamQ = requestQ;
}

void QmitknnUNetToolGUI::ProcessModelParams(mitk::nnUNetTool::Pointer tool)
{
  tool->EnsembleOff();
  std::vector<mitk::ModelParams> requestQ;
  QString modelName = m_Controls.modelBox->currentText();
  QString taskName = m_Controls.taskBox->currentText();
  QString trainer = m_Controls.trainerBox->currentText();
  QString planId = m_Controls.plannerBox->currentText();
  std::vector<std::string> fetchedFolds = FetchSelectedFoldsFromUI();
  mitk::ModelParams modelObject = MapToRequest(modelName, taskName, trainer, planId, fetchedFolds);
  requestQ.push_back(modelObject);
  tool->m_ParamQ.clear();
  tool->m_ParamQ = requestQ;
}

bool QmitknnUNetToolGUI::IsModelExists(const QString &modelName, const QString &taskName, const QString &trainerPlanner)
{
  QString modelSearchPath =
    QDir::cleanPath(m_ParentFolder->getResultsFolder() + QDir::separator() + "nnUNet" + QDir::separator() + modelName +
                    QDir::separator() + taskName + QDir::separator() + trainerPlanner);
  if (QDir(modelSearchPath).exists())
  {
    return true;
  }
  return false;
}
