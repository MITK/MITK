/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitknnUNetToolGUI.h"

#include "mitkProcessExecutor.h"
#include "mitknnUnetTool.h"
#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QIcon>
#include <QmitkStyleManager.h>
#include <QmitknnUNetEnsembleLayout.h>
#include <QtGlobal>
#include <algorithm>
#include <ctkCollapsibleGroupBox.h>
#include <itksys/SystemTools.hxx>
#include <nlohmann/json.hpp>
#include <mitkIOUtil.h>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitknnUNetToolGUI, "")

QmitknnUNetToolGUI::QmitknnUNetToolGUI() : QmitkMultiLabelSegWithPreviewToolGUIBase(), m_SuperclassEnableConfirmSegBtnFnc(m_EnableConfirmSegBtnFnc)
{
  // Nvidia-smi command returning zero doesn't always imply lack of GPUs.
  // Pytorch uses its own libraries to communicate to the GPUs. Hence, only a warning can be given.
  if (m_GpuLoader.GetGPUCount() == 0)
  {
    std::string warning = "WARNING: No GPUs were detected on your machine. The nnUNet tool might not work.";
    this->ShowErrorMessage(warning);
  }

  // define predicates for multi modal data selection combobox
  auto imageType = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto labelSetImageType = mitk::NodePredicateNot::New(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  m_MultiModalPredicate = mitk::NodePredicateAnd::New(imageType, labelSetImageType).GetPointer();

  m_nnUNetThread = new QThread(this);
  m_Worker = new nnUNetDownloadWorker;
  m_Worker->moveToThread(m_nnUNetThread);
  
  m_EnableConfirmSegBtnFnc = [this](bool enabled)
  {
    return !m_FirstPreviewComputation ? m_SuperclassEnableConfirmSegBtnFnc(enabled) : false;
  };
}

QmitknnUNetToolGUI::~QmitknnUNetToolGUI()
{
  m_nnUNetThread->quit();
  m_nnUNetThread->wait();
}

void QmitknnUNetToolGUI::ConnectNewTool(mitk::SegWithPreviewTool *newTool)
{
  Superclass::ConnectNewTool(newTool);
  newTool->IsTimePointChangeAwareOff();
  m_FirstPreviewComputation = true;
}

void QmitknnUNetToolGUI::InitializeUI(QBoxLayout *mainLayout)
{
  m_Controls.setupUi(this);
#ifndef _WIN32
  m_Controls.pythonEnvComboBox->addItem("/usr/bin");
#endif
  m_Controls.pythonEnvComboBox->addItem("Select");
  AutoParsePythonPaths();
  SetGPUInfo();
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
  connect(m_Controls.multiModalBox, SIGNAL(stateChanged(int)), this, SLOT(OnCheckBoxChanged(int)));
  connect(m_Controls.pythonEnvComboBox,
#if QT_VERSION >= 0x050F00 // 5.15
          SIGNAL(textActivated(const QString &)),
#elif QT_VERSION >= 0x050C00 // 5.12
          SIGNAL(currentTextChanged(const QString &)),
#endif
          this,
          SLOT(OnPythonPathChanged(const QString &)));
  connect(m_Controls.refreshdirectoryBox, SIGNAL(clicked()), this, SLOT(OnRefreshPresssed()));
  connect(m_Controls.clearCacheButton, SIGNAL(clicked()), this, SLOT(OnClearCachePressed()));
  connect(m_Controls.startDownloadButton, SIGNAL(clicked()), this, SLOT(OnDownloadModel()));
  connect(m_Controls.stopDownloadButton, SIGNAL(clicked()), this, SLOT(OnStopDownload()));

  // Qthreads
  qRegisterMetaType<mitk::ProcessExecutor::Pointer>();
  qRegisterMetaType<mitk::ProcessExecutor::ArgumentListType>();
  connect(this, &QmitknnUNetToolGUI::Operate, m_Worker, &nnUNetDownloadWorker::DoWork);
  connect(m_Worker, &nnUNetDownloadWorker::Exit, this, &QmitknnUNetToolGUI::OnDownloadWorkerExit);
  connect(m_nnUNetThread, &QThread::finished, m_Worker, &QObject::deleteLater);

  m_Controls.multiModalValueLabel->setStyleSheet("font-weight: bold; color: white");
  m_Controls.multiModalValueLabel->setVisible(false);
  m_Controls.requiredModalitiesLabel->setVisible(false);
  m_Controls.stopDownloadButton->setVisible(false);
  m_Controls.previewButton->setEnabled(false);

  QIcon refreshIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/view-refresh.svg"));
  m_Controls.refreshdirectoryBox->setIcon(refreshIcon);
  QIcon dirIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/document-open.svg"));
  m_Controls.modeldirectoryBox->setIcon(dirIcon);
  m_Controls.refreshdirectoryBox->setEnabled(true);
  QIcon stopIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/status/dialog-error.svg"));
  m_Controls.stopDownloadButton->setIcon(stopIcon);

  m_Controls.statusLabel->setTextFormat(Qt::RichText);
  if (m_GpuLoader.GetGPUCount() != 0)
  {
    WriteStatusMessage(QString("<b>STATUS: </b><i>Welcome to nnUNet. " + QString::number(m_GpuLoader.GetGPUCount()) +
                               " GPUs were detected.</i>"));
  }
  else
  {
    WriteErrorMessage(QString("<b>STATUS: </b><i>Welcome to nnUNet. " + QString::number(m_GpuLoader.GetGPUCount()) +
                              " GPUs were detected.</i>"));
  }
  mainLayout->addLayout(m_Controls.verticalLayout);
  Superclass::InitializeUI(mainLayout);
  m_UI_ROWS = m_Controls.advancedSettingsLayout->rowCount(); // Must do. Row count is correct only here.
  this->DisableEverything();
  QString lastSelectedPyEnv = m_Settings.value("nnUNet/LastPythonPath").toString();
  m_Controls.pythonEnvComboBox->setCurrentText(lastSelectedPyEnv);
}

void QmitknnUNetToolGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
}

void QmitknnUNetToolGUI::ClearAllModalities()
{
  m_Controls.multiModalBox->setChecked(false);
  this->ClearAllModalLabels();
}

void QmitknnUNetToolGUI::ClearAllModalLabels()
{
  for (auto modalLabel : m_ModalLabels)
  {
    delete modalLabel; // delete the layout item
    m_ModalLabels.pop_back();
  }
  m_Controls.advancedSettingsLayout->update();
}

void QmitknnUNetToolGUI::DisableEverything()
{
  m_Controls.modeldirectoryBox->setEnabled(false);
  m_Controls.refreshdirectoryBox->setEnabled(false);
  m_Controls.previewButton->setEnabled(false);
  m_Controls.multiModalValueLabel->setVisible(false);
  m_Controls.multiModalBox->setEnabled(false);
  this->ClearAllComboBoxes();
  this->ClearAllModalities();
}

void QmitknnUNetToolGUI::ClearAllComboBoxes()
{
  m_Controls.modelBox->clear();
  m_Controls.taskBox->clear();
  m_Controls.foldBox->clear();
  m_Controls.trainerBox->clear();
  m_Controls.plannerBox->clear();
  for (auto &layout : m_EnsembleParams)
  {
    layout->modelBox->clear();
    layout->trainerBox->clear();
    layout->plannerBox->clear();
    layout->foldBox->clear();
  }
}

std::vector<mitk::Image::ConstPointer> QmitknnUNetToolGUI::FetchMultiModalImagesFromUI()
{
  std::vector<mitk::Image::ConstPointer> modals;
  if (m_Controls.multiModalBox->isChecked() && !m_Modalities.empty())
  {
    std::set<std::string> nodeNames; // set container for keeping names of all nodes to check if they are added twice.
    for (QmitkSingleNodeSelectionWidget *modality : m_Modalities)
    { 
      mitk::DataNode::Pointer node = modality->GetSelectedNode();
      if (nodeNames.find(node->GetName()) == nodeNames.end())
      {
        modals.push_back(dynamic_cast<const mitk::Image *>(node->GetData()));
        nodeNames.insert(node->GetName());
      }
      else
      {
        throw std::runtime_error("Same modality is selected more than once. Please change your selection.");
        break;
      }
    }
  }
  return modals;
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
  bool isExists = QFile::exists(fullPath + QDir::separator() + QString("nnUNet_predict")) &&
                  QFile::exists(fullPath + QDir::separator() + QString("python3"));
  return isExists;
}

void QmitknnUNetToolGUI::ShowErrorMessage(const std::string &message, QMessageBox::Icon icon)
{
  this->setCursor(Qt::ArrowCursor);
  QMessageBox *messageBox = new QMessageBox(icon, nullptr, message.c_str());
  messageBox->exec();
  delete messageBox;
  MITK_WARN << message;
}

void QmitknnUNetToolGUI::WriteStatusMessage(const QString &message)
{
  m_Controls.statusLabel->setText(message);
  m_Controls.statusLabel->setStyleSheet("font-weight: bold; color: white");
}

void QmitknnUNetToolGUI::WriteErrorMessage(const QString &message)
{
  m_Controls.statusLabel->setText(message);
  m_Controls.statusLabel->setStyleSheet("font-weight: bold; color: red");
}

void QmitknnUNetToolGUI::ProcessEnsembleModelsParams(mitk::nnUNetTool::Pointer tool)
{
  if (m_EnsembleParams[0]->modelBox->currentText() == m_EnsembleParams[1]->modelBox->currentText())
  {
    throw std::runtime_error("Both models you have selected for ensembling are the same.");
  }
  QString taskName = m_Controls.taskBox->currentText();
  bool isPPJson = m_Controls.postProcessingCheckBox->isChecked();
  std::vector<mitk::ModelParams> requestQ;
  QString ppDirFolderNamePart1 = "ensemble_";
  QStringList ppDirFolderNameParts;
  for (auto &layout : m_EnsembleParams)
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

    if (!this->IsModelExists(modelName, taskName, QString(trainer + "__" + planId)))
    {
      std::string errorMsg = "The configuration " + modelName.toStdString() +
                             " you have selected doesn't exist. Check your Results Folder again.";
      throw std::runtime_error(errorMsg);
    }
    std::vector<std::string> testfold = FetchSelectedFoldsFromUI(layout->foldBox);
    mitk::ModelParams modelObject = MapToRequest(modelName, taskName, trainer, planId, testfold);
    requestQ.push_back(modelObject);
    ppDirFolderNameParts << ppDirFolderName.join(QString(""));
  }
  tool->EnsembleOn();
  if (isPPJson)
  {
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
      const QString statusMsg = "<i>Post Processing JSON file found: </i>" + ppJsonFilePossibility1;
      this->WriteStatusMessage(statusMsg);
    }
    else if (QFile(ppJsonFilePossibility2).exists())
    {
      tool->SetPostProcessingJsonDirectory(ppJsonFilePossibility2.toStdString());
      const QString statusMsg = "<i>Post Processing JSON file found:</i>" + ppJsonFilePossibility2;
      this->WriteStatusMessage(statusMsg);
    }
    else
    {
      std::string errorMsg =
        "No post processing file was found for the selected ensemble combination. Continuing anyway...";
      this->ShowErrorMessage(errorMsg);
    }
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
  std::vector<std::string> fetchedFolds = this->FetchSelectedFoldsFromUI(m_Controls.foldBox);
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

void QmitknnUNetToolGUI::CheckAllInCheckableComboBox(ctkCheckableComboBox *foldBox)
{
  // Recalling all added items to check-mark it.
  const QAbstractItemModel *qaim = foldBox->checkableModel();
  auto rows = qaim->rowCount();
  for (std::remove_const_t<decltype(rows)> i = 0; i < rows; ++i)
  {
    const QModelIndex mi = qaim->index(i, 0);
    foldBox->setCheckState(mi, Qt::Checked);
  }
}

std::pair<QStringList, QStringList> QmitknnUNetToolGUI::ExtractTrainerPlannerFromString(QStringList trainerPlanners)
{
  QString splitterString = "__";
  QStringList trainers, planners;
  for (const auto &trainerPlanner : trainerPlanners)
  {
    trainers << trainerPlanner.split(splitterString, QString::SplitBehavior::SkipEmptyParts).first();
    planners << trainerPlanner.split(splitterString, QString::SplitBehavior::SkipEmptyParts).last();
  }
  trainers.removeDuplicates();
  planners.removeDuplicates();
  return std::make_pair(trainers, planners);
}

std::vector<std::string> QmitknnUNetToolGUI::FetchSelectedFoldsFromUI(ctkCheckableComboBox *foldBox)
{
  std::vector<std::string> folds;
  if (foldBox->noneChecked())
  {
    this->CheckAllInCheckableComboBox(foldBox);
  }
  QModelIndexList foldList = foldBox->checkedIndexes();
  for (const auto &index : foldList)
  {
    QString foldQString = foldBox->itemText(index.row());
    if(foldQString != "dummy_element_that_nobody_can_see")
    {
    foldQString = foldQString.split("_", QString::SplitBehavior::SkipEmptyParts).last();
    folds.push_back(foldQString.toStdString());
    }
    else
    {
      throw std::runtime_error("Folds are not recognized. Please check if your nnUNet results folder structure is legitimate");
    }
  }
  return folds;
}

void QmitknnUNetToolGUI::UpdateCacheCountOnUI()
{
  QString cacheText = m_CACHE_COUNT_BASE_LABEL + QString::number(m_Cache.size());
  m_Controls.cacheCountLabel->setText(cacheText);
}

void QmitknnUNetToolGUI::AddToCache(size_t &hashKey, mitk::LabelSetImage::ConstPointer mlPreview)
{
  nnUNetCache *newCacheObj = new nnUNetCache;
  newCacheObj->m_SegCache = mlPreview;
  m_Cache.insert(hashKey, newCacheObj);
  MITK_INFO << "New hash: " << hashKey << " " << newCacheObj->m_SegCache.GetPointer();
  this->UpdateCacheCountOnUI();
}

void QmitknnUNetToolGUI::SetGPUInfo()
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

unsigned int QmitknnUNetToolGUI::FetchSelectedGPUFromUI()
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

QString QmitknnUNetToolGUI::FetchResultsFolderFromEnv()
{
  const char *pathVal = itksys::SystemTools::GetEnv("RESULTS_FOLDER");
  QString retVal;
  if (pathVal)
  {
    retVal = QString::fromUtf8(pathVal);
  }
  else
  {
    retVal = m_Settings.value("nnUNet/LastRESULTS_FOLDERPath").toString();
  }
  return retVal;
}

void QmitknnUNetToolGUI::DumpJSONfromPickle(const QString &picklePath)
{
  const QString pickleFile = picklePath + QDir::separator() + m_PICKLE_FILENAME;
  const QString jsonFile = picklePath + QDir::separator() + m_MITK_EXPORT_JSON_FILENAME;
  if (!QFile::exists(jsonFile))
  {
    mitk::ProcessExecutor::Pointer spExec = mitk::ProcessExecutor::New();
    mitk::ProcessExecutor::ArgumentListType args;
    args.push_back("-c");
    std::string pythonCode; // python syntax to parse plans.pkl file and export as Json file.
    pythonCode.append("import pickle;");
    pythonCode.append("import json;");
    pythonCode.append("loaded_pickle = pickle.load(open('");
    pythonCode.append(pickleFile.toStdString());
    pythonCode.append("','rb'));");
    pythonCode.append("modal_dict = {key: loaded_pickle[key] for key in loaded_pickle.keys() if key in "
                      "['modalities','num_modalities']};");
    pythonCode.append("json.dump(modal_dict, open('");
    pythonCode.append(jsonFile.toStdString());
    pythonCode.append("', 'w'))");

    args.push_back(pythonCode);
    try
    {
      spExec->Execute(m_PythonPath.toStdString(), "python3", args);
    }
    catch (const mitk::Exception &e)
    {
      MITK_ERROR << "Pickle parsing FAILED!" << e.GetDescription();
      this->WriteStatusMessage(
        "Parsing failed in backend. Multiple Modalities will now have to be manually entered by the user.");
    }
  }
}

void QmitknnUNetToolGUI::ExportAvailableModelsAsJSON(const QString &resultsFolder)
{
  const QString jsonPath = resultsFolder + QDir::separator() + m_AVAILABLE_MODELS_JSON_FILENAME;
  if (!QFile::exists(jsonPath))
  {
    auto spExec = mitk::ProcessExecutor::New();
    mitk::ProcessExecutor::ArgumentListType args;
    args.push_back("--export");
    args.push_back(resultsFolder.toStdString());
    try
    {
      spExec->Execute(m_PythonPath.toStdString(), "nnUNet_print_available_pretrained_models", args);
    }
    catch (const mitk::Exception &e)
    {
      MITK_ERROR << "Exporting information FAILED." << e.GetDescription();
      this->WriteStatusMessage("Exporting information FAILED.");
    }
  }
}

void QmitknnUNetToolGUI::DisplayMultiModalInfoFromJSON(const QString &jsonPath)
{
  std::ifstream file(jsonPath.toStdString());
  if (file.is_open())
  {
    auto jsonObj = nlohmann::json::parse(file, nullptr, false);
    if (jsonObj.is_discarded() || !jsonObj.is_object())
    {
      MITK_ERROR << "Could not parse \"" << jsonPath.toStdString() << "\" as JSON object!";
      return;
    }
    auto num_mods = jsonObj["num_modalities"].get<int>();
    this->ClearAllModalLabels();
    if (num_mods > 1)
    {
      m_Controls.multiModalBox->setChecked(true);
      m_Controls.multiModalBox->setEnabled(false);
      m_Controls.multiModalValueLabel->setText(QString::number(num_mods));
      OnModalitiesNumberChanged(num_mods);
      m_Controls.advancedSettingsLayout->update();
      auto obj = jsonObj["modalities"];
      int count = 0;
      for (const auto &value : obj)
      {
        QLabel *label = new QLabel(QString::fromStdString("<i>" + value.get<std::string>() + "</i>"), this);
        m_ModalLabels.push_back(label);
        m_Controls.advancedSettingsLayout->addWidget(label, m_UI_ROWS + 1 + count, 0);
        count++;
      }
      m_Controls.advancedSettingsLayout->update();
    }
    else
    {
      m_Controls.multiModalBox->setChecked(false);
    }
  }
}

void QmitknnUNetToolGUI::FillAvailableModelsInfoFromJSON(const QString &jsonPath)
{
  std::ifstream file(jsonPath.toStdString());
  if (file.is_open() && m_Controls.availableBox->count() < 1)
  {
    auto jsonObj = nlohmann::json::parse(file, nullptr, false);
    if (jsonObj.is_discarded() || !jsonObj.is_object())
    {
      MITK_ERROR << "Could not parse \"" << jsonPath.toStdString() << "\" as JSON object!";
      return;
    }
    for (const auto &obj : jsonObj.items())
    {
      m_Controls.availableBox->addItem(QString::fromStdString(obj.key()));
    }
  }
}

mitk::ModelParams QmitknnUNetToolGUI::MapToRequest(const QString &modelName,
                                                   const QString &taskName,
                                                   const QString &trainer,
                                                   const QString &planId,
                                                   const std::vector<std::string> &folds)
{
  mitk::ModelParams requestObject;
  requestObject.model = modelName.toStdString();
  requestObject.trainer = trainer.toStdString();
  requestObject.planId = planId.toStdString();
  requestObject.task = taskName.toStdString();
  requestObject.folds = folds;
  mitk::nnUNetTool::Pointer tool = this->GetConnectedToolAs<mitk::nnUNetTool>();
  requestObject.inputName = tool->GetRefNode()->GetName();
  requestObject.timeStamp =
    std::to_string(mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint());
  return requestObject;
}

void QmitknnUNetToolGUI::SetComboBoxToNone(ctkCheckableComboBox* comboBox)
{
  comboBox->clear();
  comboBox->addItem("dummy_element_that_nobody_can_see");
  qobject_cast<QListView *>(comboBox->view())->setRowHidden(0, true); // For the cosmetic purpose of showing "None" on the combobox.
}

/* ---------------------SLOTS---------------------------------------*/

void QmitknnUNetToolGUI::OnPreviewRequested()
{
  mitk::nnUNetTool::Pointer tool = this->GetConnectedToolAs<mitk::nnUNetTool>();
  if (nullptr != tool)
  {
    QString pythonPathTextItem = "";
    try
    {
      size_t hashKey(0);
      m_Controls.previewButton->setEnabled(false); // To prevent misclicked back2back prediction.
      qApp->processEvents();
      tool->PredictOn(); // purposefully placed to make tool->GetMTime different than before.
      QString modelName = m_Controls.modelBox->currentText();
      if (modelName.startsWith("ensemble", Qt::CaseInsensitive))
      {
        this->ProcessEnsembleModelsParams(tool);
      }
      else
      {
        this->ProcessModelParams(tool);
      }
      pythonPathTextItem = m_Controls.pythonEnvComboBox->currentText();
      QString pythonPath = m_PythonPath;
      if (!this->IsNNUNetInstalled(pythonPath))
      {
        throw std::runtime_error("nnUNet is not detected in the selected python environment. Please select a valid "
                                 "python environment or install nnUNet.");
      }
      tool->SetPythonPath(pythonPath.toStdString());
      tool->SetModelDirectory(m_ParentFolder->getResultsFolder().toStdString());
      // checkboxes
      tool->SetMirror(m_Controls.mirrorBox->isChecked());
      tool->SetMixedPrecision(m_Controls.mixedPrecisionBox->isChecked());
      tool->SetNoPip(false);
      bool doCache = m_Controls.enableCachingCheckBox->isChecked();
      // Spinboxes
      tool->SetGpuId(FetchSelectedGPUFromUI());
      // Multi-Modal
      tool->MultiModalOff();
      if (m_Controls.multiModalBox->isChecked())
      {
        tool->m_OtherModalPaths.clear();
        tool->m_OtherModalPaths = FetchMultiModalImagesFromUI();
        tool->MultiModalOn();
      }
      if (doCache)
      {
        hashKey = nnUNetCache::GetUniqueHash(tool->m_ParamQ);
        if (m_Cache.contains(hashKey))
        {
          tool->PredictOff(); // purposefully placed to make tool->GetMTime different than before.
        }
      }
      if (tool->GetPredict())
      {
        tool->m_InputBuffer = nullptr;
        this->WriteStatusMessage(
          QString("<b>STATUS: </b><i>Starting Segmentation task... This might take a while.</i>"));
        tool->UpdatePreview();
        if (nullptr == tool->GetOutputBuffer())
        {
          this->SegmentationProcessFailed();
        }
        else
        {
          this->SegmentationResultHandler(tool);
          if (doCache)
          {
            this->AddToCache(hashKey, tool->GetOutputBuffer());
          }
          tool->ClearOutputBuffer();
        }
        tool->PredictOff(); // purposefully placed to make tool->GetMTime different than before.
      }
      else
      {
        MITK_INFO << "won't do segmentation. Key found: " << QString::number(hashKey).toStdString();
        if (m_Cache.contains(hashKey))
        {
          nnUNetCache *cacheObject = m_Cache[hashKey];
          MITK_INFO << "fetched pointer " << cacheObject->m_SegCache.GetPointer();
          tool->SetOutputBuffer(const_cast<mitk::LabelSetImage *>(cacheObject->m_SegCache.GetPointer()));
          this->SegmentationResultHandler(tool, true);
        }
      }
      m_Controls.previewButton->setEnabled(true);
    }
    catch (const std::exception &e)
    {
      std::stringstream errorMsg;
      errorMsg << "<b>STATUS: </b>Error while processing parameters for nnUNet segmentation. Reason: " << e.what();
      this->ShowErrorMessage(errorMsg.str());
      this->WriteErrorMessage(QString::fromStdString(errorMsg.str()));
      m_Controls.previewButton->setEnabled(true);
      tool->PredictOff();
      return;
    }
    catch (...)
    {
      std::string errorMsg = "Unkown error occured while generation nnUNet segmentation.";
      this->ShowErrorMessage(errorMsg);
      m_Controls.previewButton->setEnabled(true);
      tool->PredictOff();
      return;
    }
    if (!pythonPathTextItem.isEmpty())
    { // only cache if the prediction ended without errors.
      m_Settings.setValue("nnUNet/LastPythonPath", pythonPathTextItem);
    }
  }
}

void QmitknnUNetToolGUI::OnRefreshPresssed()
{
  const QString resultsFolder = m_Controls.modeldirectoryBox->directory();
  this->OnDirectoryChanged(resultsFolder);
}

void QmitknnUNetToolGUI::OnDirectoryChanged(const QString &resultsFolder)
{
  m_IsResultsFolderValid = false;
  m_Controls.previewButton->setEnabled(false);
  this->ClearAllComboBoxes();
  this->ClearAllModalities();
  m_ParentFolder = std::make_shared<QmitknnUNetFolderParser>(resultsFolder);
  auto tasks = m_ParentFolder->getAllTasks<QStringList>();
  tasks.removeDuplicates();
  std::for_each(tasks.begin(), tasks.end(), [this](QString task) { m_Controls.taskBox->addItem(task); });
  m_Settings.setValue("nnUNet/LastRESULTS_FOLDERPath", resultsFolder);
}

void QmitknnUNetToolGUI::OnModelChanged(const QString &model)
{
  if (model.isEmpty())
  {
    return;
  }
  this->ClearAllModalities();
  auto selectedTask = m_Controls.taskBox->currentText();
  ctkComboBox *box = qobject_cast<ctkComboBox *>(sender());
  if (box == m_Controls.modelBox)
  {
    if (model == m_VALID_MODELS.last())
    {
      m_Controls.trainerBox->setVisible(false);
      m_Controls.trainerLabel->setVisible(false);
      m_Controls.plannerBox->setVisible(false);
      m_Controls.plannerLabel->setVisible(false);
      m_Controls.foldBox->setVisible(false);
      m_Controls.foldLabel->setVisible(false);
      m_Controls.previewButton->setEnabled(false);
      this->ShowEnsembleLayout(true);
      auto models = m_ParentFolder->getModelsForTask<QStringList>(m_Controls.taskBox->currentText());
      models.removeDuplicates();
      models.removeOne(m_VALID_MODELS.last());
      for (auto &layout : m_EnsembleParams)
      {
        layout->modelBox->clear();
        layout->trainerBox->clear();
        layout->plannerBox->clear();
        std::for_each(models.begin(),
                      models.end(),
                      [&layout, this](QString model)
                      {
                        if (m_VALID_MODELS.contains(model, Qt::CaseInsensitive))
                          layout->modelBox->addItem(model);
                      });
      }
    }
    else
    {
      m_Controls.trainerBox->setVisible(true);
      m_Controls.trainerLabel->setVisible(true);
      m_Controls.plannerBox->setVisible(true);
      m_Controls.plannerLabel->setVisible(true);
      m_Controls.foldBox->setVisible(true);
      m_Controls.foldLabel->setVisible(true);
      m_Controls.previewButton->setEnabled(false);
      this->ShowEnsembleLayout(false);
      m_Controls.trainerBox->clear();
      m_Controls.plannerBox->clear();
      auto trainerPlanners = m_ParentFolder->getTrainerPlannersForTask<QStringList>(selectedTask, model);
      if(trainerPlanners.isEmpty())
      {
        this->ShowErrorMessage("No plans.pkl found for "+model.toStdString()+". Check your directory or download the task again.");
        this->SetComboBoxToNone(m_Controls.foldBox);
        return;
      }
      QStringList trainers, planners;
      std::tie(trainers, planners) = ExtractTrainerPlannerFromString(trainerPlanners);
      std::for_each(
        trainers.begin(), trainers.end(), [this](QString trainer) { m_Controls.trainerBox->addItem(trainer); });
      std::for_each(
        planners.begin(), planners.end(), [this](QString planner) { m_Controls.plannerBox->addItem(planner); });
    }
  }
  else if (!m_EnsembleParams.empty())
  { 
    m_Controls.previewButton->setEnabled(false);
    for (auto &layout : m_EnsembleParams)
    {
      if (box == layout->modelBox)
      {
        layout->trainerBox->clear();
        layout->plannerBox->clear();
        auto trainerPlanners = m_ParentFolder->getTrainerPlannersForTask<QStringList>(selectedTask, model);
        if(trainerPlanners.isEmpty())
        {
          this->ShowErrorMessage("No plans.pkl found for "+model.toStdString()+". Check your directory or download the task again.");
          this->SetComboBoxToNone(layout->foldBox);
          return;
        }
        QStringList trainers, planners;
        std::tie(trainers, planners) = ExtractTrainerPlannerFromString(trainerPlanners);
        std::for_each(trainers.begin(),
                      trainers.end(),
                      [&layout](const QString &trainer) { layout->trainerBox->addItem(trainer); });
        std::for_each(planners.begin(),
                      planners.end(),
                      [&layout](const QString &planner) { layout->plannerBox->addItem(planner); });
        break;
      }
    }
  }
}

void QmitknnUNetToolGUI::OnTaskChanged(const QString &task)
{
  if (task.isEmpty())
  {
    return;
  }
  m_Controls.modelBox->clear();
  auto models = m_ParentFolder->getModelsForTask<QStringList>(task);
  models.removeDuplicates();
  if (!models.contains(m_VALID_MODELS.last(), Qt::CaseInsensitive))
  {
    models << m_VALID_MODELS.last(); // add ensemble even if folder doesn't exist
  }
  std::for_each(models.begin(),
                models.end(),
                [this](QString model)
                {
                  if (m_VALID_MODELS.contains(model, Qt::CaseInsensitive))
                    m_Controls.modelBox->addItem(model);
                });
}

void QmitknnUNetToolGUI::OnTrainerChanged(const QString &plannerSelected)
{
  if (plannerSelected.isEmpty())
  {
    return;
  }
  m_IsResultsFolderValid = false;
  QString parentPath;
  auto *box = qobject_cast<ctkComboBox *>(sender());
  if (box == m_Controls.plannerBox)
  {
    m_Controls.foldBox->clear();
    auto selectedTrainer = m_Controls.trainerBox->currentText();
    auto selectedTask = m_Controls.taskBox->currentText();
    auto selectedModel = m_Controls.modelBox->currentText();
    auto folds = m_ParentFolder->getFoldsForTrainerPlanner<QStringList>(
      selectedTrainer, plannerSelected, selectedTask, selectedModel);
    if(folds.isEmpty())
    {
      this->ShowErrorMessage("No valid folds found. Check your directory or download the task again.");
      this->SetComboBoxToNone(m_Controls.foldBox);
      return;
    }
    std::for_each(folds.begin(),
                  folds.end(),
                  [this](QString fold)
                  {
                    if (fold.startsWith("fold_", Qt::CaseInsensitive)) // imposed by nnUNet
                      m_Controls.foldBox->addItem(fold);
                  });
    if (m_Controls.foldBox->count() != 0)
    {
      m_IsResultsFolderValid = true;
      this->CheckAllInCheckableComboBox(m_Controls.foldBox);
      auto tempPath = QStringList() << m_ParentFolder->getResultsFolder() << "nnUNet" << selectedModel << selectedTask
                                    << QString("%1__%2").arg(selectedTrainer, plannerSelected);
      parentPath = QDir::cleanPath(tempPath.join(QDir::separator()));
    }
  }
  else if (!m_EnsembleParams.empty())
  {
    for (auto &layout : m_EnsembleParams)
    {
      if (box == layout->plannerBox)
      {
        layout->foldBox->clear();
        auto selectedTrainer = layout->trainerBox->currentText();
        auto selectedTask = m_Controls.taskBox->currentText();
        auto selectedModel = layout->modelBox->currentText();
        auto folds = m_ParentFolder->getFoldsForTrainerPlanner<QStringList>(
          selectedTrainer, plannerSelected, selectedTask, selectedModel);
        if(folds.isEmpty())
        {
          this->ShowErrorMessage("No valid folds found. Check your directory.");
          this->SetComboBoxToNone(layout->foldBox);
          return;
        }
        std::for_each(folds.begin(),
                      folds.end(),
                      [&layout](const QString &fold)
                      {
                        if (fold.startsWith("fold_", Qt::CaseInsensitive)) // imposed by nnUNet
                          layout->foldBox->addItem(fold);
                      });
        if (layout->foldBox->count() != 0)
        {
          this->CheckAllInCheckableComboBox(layout->foldBox);
          m_IsResultsFolderValid = true;
          auto tempPath = QStringList() << m_ParentFolder->getResultsFolder() << "nnUNet" << selectedModel
                                        << selectedTask << QString("%1__%2").arg(selectedTrainer, plannerSelected);
          parentPath = QDir::cleanPath(tempPath.join(QDir::separator()));
        }
        break;
      }
    }
  }
  if (m_IsResultsFolderValid)
  {
    m_Controls.previewButton->setEnabled(true);
    const QString mitkJsonFile = parentPath + QDir::separator() + m_MITK_EXPORT_JSON_FILENAME;
    this->DumpJSONfromPickle(parentPath);
    if (QFile::exists(mitkJsonFile))
    {
      this->DisplayMultiModalInfoFromJSON(mitkJsonFile);
    }
  }
}

void QmitknnUNetToolGUI::OnPythonPathChanged(const QString &pyEnv)
{
  if (pyEnv == QString("Select"))
  {
    QString path =
      QFileDialog::getExistingDirectory(m_Controls.pythonEnvComboBox->parentWidget(), "Python Path", "dir");
    if (!path.isEmpty())
    {
      this->OnPythonPathChanged(path); // recall same function for new path validation
      m_Controls.pythonEnvComboBox->insertItem(0, path);
      m_Controls.pythonEnvComboBox->setCurrentIndex(0);
    }
  }
  else if (!this->IsNNUNetInstalled(pyEnv))
  {
    std::string warning =
      "WARNING: nnUNet is not detected on the Python environment you selected. Please select another "
      "environment or create one. For more info refer https://github.com/MIC-DKFZ/nnUNet";
    this->ShowErrorMessage(warning);
    this->DisableEverything();
    m_Controls.availableBox->clear();
  }
  else
  {
    m_Controls.modeldirectoryBox->setEnabled(true);
    m_Controls.refreshdirectoryBox->setEnabled(true);
    m_Controls.multiModalBox->setEnabled(true);
    QString setVal = this->FetchResultsFolderFromEnv();
    if (!setVal.isEmpty())
    {
      m_Controls.modeldirectoryBox->setDirectory(setVal);
    }
    this->OnRefreshPresssed();
    m_PythonPath = pyEnv.mid(pyEnv.indexOf(" ") + 1);
#ifdef _WIN32
    if (!(m_PythonPath.endsWith("Scripts", Qt::CaseInsensitive) || m_PythonPath.endsWith("Scripts/", Qt::CaseInsensitive)))
    {
      m_PythonPath += QDir::separator() + QString("Scripts");
    }
#else
    if (!(m_PythonPath.endsWith("bin", Qt::CaseInsensitive) || m_PythonPath.endsWith("bin/", Qt::CaseInsensitive)))
    {
      m_PythonPath += QDir::separator() + QString("bin");
    }
#endif
    // Export available model info as json and fill them for Download
    QString tempPath = QString::fromStdString(mitk::IOUtil::GetTempPath());
    this->ExportAvailableModelsAsJSON(tempPath);
    const QString jsonPath = tempPath + QDir::separator() + m_AVAILABLE_MODELS_JSON_FILENAME;
    if (QFile::exists(jsonPath))
    {
      this->FillAvailableModelsInfoFromJSON(jsonPath);
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
    if (box->objectName() == QString("multiModalBox"))
    {
      m_Controls.requiredModalitiesLabel->setVisible(visibility);
      m_Controls.multiModalValueLabel->setVisible(visibility);
      if (!visibility)
      {
        this->OnModalitiesNumberChanged(0);
        m_Controls.multiModalValueLabel->setText("0");
        this->ClearAllModalLabels();
      }
    }
  }
}

void QmitknnUNetToolGUI::OnModalitiesNumberChanged(int num)
{
  while (num > static_cast<int>(m_Modalities.size()))
  {
    QmitkSingleNodeSelectionWidget *multiModalBox = new QmitkSingleNodeSelectionWidget(this);
    mitk::nnUNetTool::Pointer tool = this->GetConnectedToolAs<mitk::nnUNetTool>();
    multiModalBox->SetDataStorage(tool->GetDataStorage());
    multiModalBox->SetInvalidInfo("Select corresponding modalities");
    multiModalBox->SetNodePredicate(m_MultiModalPredicate);
    multiModalBox->setObjectName(QString("multiModal_" + QString::number(m_Modalities.size() + 1)));
    m_Controls.advancedSettingsLayout->addWidget(multiModalBox, m_UI_ROWS + m_Modalities.size() + 1, 1, 1, 3);
    m_Modalities.push_back(multiModalBox);
  }
  while (num < static_cast<int>(m_Modalities.size()) && !m_Modalities.empty())
  {
    QmitkSingleNodeSelectionWidget *child = m_Modalities.back();
    delete child; // delete the layout item
    m_Modalities.pop_back();
  }
  m_Controls.advancedSettingsLayout->update();
}

void QmitknnUNetToolGUI::AutoParsePythonPaths()
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

void QmitknnUNetToolGUI::SegmentationProcessFailed()
{
  this->WriteErrorMessage(
    "<b>STATUS: </b><i>Error in the segmentation process. <br>No resulting segmentation can be loaded.</i>");
  this->setCursor(Qt::ArrowCursor);
  std::stringstream stream;
  stream << "Error in the segmentation process. No resulting segmentation can be loaded.";
  this->ShowErrorMessage(stream.str());
}

void QmitknnUNetToolGUI::SegmentationResultHandler(mitk::nnUNetTool *tool, bool forceRender)
{
  if (forceRender)
  {
    tool->RenderOutputBuffer();
  }
  m_FirstPreviewComputation = false;
  this->SetLabelSetPreview(tool->GetPreviewSegmentation());
  this->WriteStatusMessage("<b>STATUS: </b><i>Segmentation task finished successfully.</i>");
  this->ActualizePreviewLabelVisibility();
}

void QmitknnUNetToolGUI::ShowEnsembleLayout(bool visible)
{
  if (m_EnsembleParams.empty())
  {
    ctkCollapsibleGroupBox *groupBoxModel1 = new ctkCollapsibleGroupBox(this);
    auto lay1 = std::make_unique<QmitknnUNetTaskParamsUITemplate>(groupBoxModel1);
    groupBoxModel1->setObjectName(QString::fromUtf8("model_1_Box"));
    groupBoxModel1->setTitle(QString::fromUtf8("Model 1"));
    groupBoxModel1->setMinimumSize(QSize(0, 0));
    groupBoxModel1->setCollapsedHeight(5);
    groupBoxModel1->setCollapsed(false);
    groupBoxModel1->setFlat(true);
    groupBoxModel1->setAlignment(Qt::AlignRight);
    m_Controls.advancedSettingsLayout->addWidget(groupBoxModel1, 5, 0, 1, 2);

    connect(lay1->modelBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnModelChanged(const QString &)));
    connect(
      lay1->plannerBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnTrainerChanged(const QString &)));
    m_EnsembleParams.push_back(std::move(lay1));

    ctkCollapsibleGroupBox *groupBoxModel2 = new ctkCollapsibleGroupBox(this);
    auto lay2 = std::make_unique<QmitknnUNetTaskParamsUITemplate>(groupBoxModel2);
    groupBoxModel2->setObjectName(QString::fromUtf8("model_2_Box"));
    groupBoxModel2->setTitle(QString::fromUtf8("Model 2"));
    groupBoxModel2->setMinimumSize(QSize(0, 0));
    groupBoxModel2->setCollapsedHeight(5);
    groupBoxModel2->setCollapsed(false);
    groupBoxModel2->setFlat(true);
    groupBoxModel2->setAlignment(Qt::AlignLeft);
    m_Controls.advancedSettingsLayout->addWidget(groupBoxModel2, 5, 2, 1, 2);

    connect(lay2->modelBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnModelChanged(const QString &)));
    connect(
      lay2->plannerBox, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnTrainerChanged(const QString &)));
    m_EnsembleParams.push_back(std::move(lay2));
  }
  for (auto &layout : m_EnsembleParams)
  {
    layout->setVisible(visible);
  }
}

void QmitknnUNetToolGUI::OnDownloadModel()
{
  auto selectedTask = m_Controls.availableBox->currentText();
  if(!selectedTask.isEmpty())
  {
    auto spExec = mitk::ProcessExecutor::New();
    mitk::ProcessExecutor::ArgumentListType args;
    args.push_back(selectedTask.toStdString());
    this->WriteStatusMessage(
      "Downloading the requested task in to the selected Results Folder. This might take some time "
      "depending on your internet connection...");
    m_Processes["DOWNLOAD"] = spExec;
    if (!m_nnUNetThread->isRunning())
    {
      MITK_DEBUG << "Starting thread...";
      m_nnUNetThread->start();
    }
    QString resultsFolder = m_ParentFolder->getResultsFolder();
    emit Operate(resultsFolder, m_PythonPath, spExec, args);
    m_Controls.stopDownloadButton->setVisible(true);
    m_Controls.startDownloadButton->setVisible(false);
  }
}

void QmitknnUNetToolGUI::OnDownloadWorkerExit(const bool isSuccess, const QString message)
{
  if (isSuccess)
  {
    this->WriteStatusMessage(message + QString(" Click Refresh Results Folder to use the new Task."));
  }
  else
  {
    MITK_ERROR << "Download FAILED! " << message.toStdString();
    this->WriteStatusMessage(QString("Download failed. Check your internet connection. " + message));
  }
  m_Controls.stopDownloadButton->setVisible(false);
  m_Controls.startDownloadButton->setVisible(true);
}

void QmitknnUNetToolGUI::OnStopDownload()
{
  mitk::ProcessExecutor::Pointer spExec = m_Processes["DOWNLOAD"];
  spExec->KillProcess();
  this->WriteStatusMessage("Download Killed by the user.");
  m_Controls.stopDownloadButton->setVisible(false);
  m_Controls.startDownloadButton->setVisible(true);
}

void QmitknnUNetToolGUI::OnClearCachePressed()
{
  m_Cache.clear();
  this->UpdateCacheCountOnUI();
}
