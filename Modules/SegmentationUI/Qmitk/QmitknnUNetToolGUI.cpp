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
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QmitkStyleManager.h>
#include <QtGlobal>
#include <itksys/SystemTools.hxx>
#include <set>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitknnUNetToolGUI, "")

QmitknnUNetToolGUI::QmitknnUNetToolGUI() : QmitkAutoMLSegmentationToolGUIBase()
{
  // Nvidia-smi command returning zero doesn't always imply lack of GPUs.
  // Pytorch uses its own libraries to communicate to the GPUs. Hence, only a warning can be given.
  if (m_GpuLoader.GetGPUCount() == 0)
  {
    std::string warning = "WARNING: No GPUs were detected on your machine. The nnUNet tool might not work.";
    ShowErrorMessage(warning);
  }

  // define predicates for multi modal data selection combobox
  auto imageType = mitk::TNodePredicateDataType<mitk::Image>::New();
  auto labelSetImageType = mitk::NodePredicateNot::New(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  m_MultiModalPredicate = mitk::NodePredicateAnd::New(imageType, labelSetImageType).GetPointer();
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
  connect(m_Controls.multiModalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnModalitiesNumberChanged(int)));
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

  m_Controls.multiModalSpinBox->setVisible(false);
  m_Controls.multiModalSpinBox->setEnabled(false);
  m_Controls.multiModalSpinLabel->setVisible(false);
  m_Controls.previewButton->setEnabled(false);

  QIcon refreshIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/view-refresh.svg"));
  m_Controls.refreshdirectoryBox->setIcon(refreshIcon);
  QIcon dirIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/document-open.svg"));
  m_Controls.modeldirectoryBox->setIcon(dirIcon);
  m_Controls.refreshdirectoryBox->setEnabled(true);

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
  DisableEverything();
  QString lastSelectedPyEnv = m_Settings.value("nnUNet/LastPythonPath").toString();
  m_Controls.pythonEnvComboBox->setCurrentText(lastSelectedPyEnv);
}

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
        ProcessEnsembleModelsParams(tool);
      }
      else
      {
        ProcessModelParams(tool);
      }
      pythonPathTextItem = m_Controls.pythonEnvComboBox->currentText();
      QString pythonPath = m_PythonPath;
      if (!IsNNUNetInstalled(pythonPath))
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
        if (m_Controls.multiModalSpinBox->value() > 0)
        {
          tool->m_OtherModalPaths.clear();
          tool->m_OtherModalPaths = FetchMultiModalImagesFromUI();
          tool->MultiModalOn();
        }
        else
        {
          throw std::runtime_error("Please select more than one modalities for a multi-modal task. If you "
                                   "would like to use only one modality then uncheck the Multi-Modal option.");
        }
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
        WriteStatusMessage(QString("<b>STATUS: </b><i>Starting Segmentation task... This might take a while.</i>"));
        tool->UpdatePreview();
        if (nullptr == tool->GetOutputBuffer())
        {
          SegmentationProcessFailed();
        }
        else
        {
          SegmentationResultHandler(tool);
          if (doCache)
          {
            AddToCache(hashKey, tool->GetMLPreview());
          }
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
          SegmentationResultHandler(tool, true);
        }
      }
      m_Controls.previewButton->setEnabled(true);
    }
    catch (const std::exception &e)
    {
      std::stringstream errorMsg;
      errorMsg << "<b>STATUS: </b>Error while processing parameters for nnUNet segmentation. Reason: " << e.what();
      ShowErrorMessage(errorMsg.str());
      WriteErrorMessage(QString::fromStdString(errorMsg.str()));
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
    if (!pythonPathTextItem.isEmpty())
    { // only cache if the prediction ended without errors.
      m_Settings.setValue("nnUNet/LastPythonPath", pythonPathTextItem);
    }
  }
}

std::vector<mitk::Image::ConstPointer> QmitknnUNetToolGUI::FetchMultiModalImagesFromUI()
{
  std::vector<mitk::Image::ConstPointer> modals;
  if (m_Controls.multiModalBox->isChecked() && !m_Modalities.empty())
  {
    std::set<std::string> nodeNames; // set container for keeping names of all nodes to check if they are added twice.
    for (QmitkDataStorageComboBox *modality : m_Modalities)
    {
      if (nodeNames.find(modality->GetSelectedNode()->GetName()) == nodeNames.end())
      {
        modals.push_back(dynamic_cast<const mitk::Image *>(modality->GetSelectedNode()->GetData()));
        nodeNames.insert(modality->GetSelectedNode()->GetName());
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

    if (!IsModelExists(modelName, taskName, QString(trainer + "__" + planId)))
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
      WriteStatusMessage(statusMsg);
    }
    else if (QFile(ppJsonFilePossibility2).exists())
    {
      tool->SetPostProcessingJsonDirectory(ppJsonFilePossibility2.toStdString());
      const QString statusMsg = "<i>Post Processing JSON file found:</i>" + ppJsonFilePossibility2;
      WriteStatusMessage(statusMsg);
    }
    else
    {
      std::string errorMsg =
        "No post processing file was found for the selected ensemble combination. Continuing anyway...";
      ShowErrorMessage(errorMsg);
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
  std::vector<std::string> fetchedFolds = FetchSelectedFoldsFromUI(m_Controls.foldBox);
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
    CheckAllInCheckableComboBox(foldBox);
  }
  QModelIndexList foldList = foldBox->checkedIndexes();
  for (const auto &index : foldList)
  {
    QString foldQString = foldBox->itemText(index.row()).split("_", QString::SplitBehavior::SkipEmptyParts).last();
    folds.push_back(foldQString.toStdString());
  }
  return folds;
}

void QmitknnUNetToolGUI::OnClearCachePressed()
{
  m_Cache.clear();
  UpdateCacheCountOnUI();
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
  UpdateCacheCountOnUI();
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

QString QmitknnUNetToolGUI::DumpJSONfromPickle(const QString &parentPath)
{
  const QString picklePath = parentPath + QDir::separator() + QString("plans.pkl");
  const QString jsonPath = parentPath + QDir::separator() + QString("mitk_export.json");
  if (!QFile::exists(jsonPath))
  {
    mitk::ProcessExecutor::Pointer spExec = mitk::ProcessExecutor::New();
    itk::CStyleCommand::Pointer spCommand = itk::CStyleCommand::New();
    mitk::ProcessExecutor::ArgumentListType args;
    args.push_back("-c");
    std::string pythonCode; // python syntax to parse plans.pkl file and export as Json file.
    pythonCode.append("import pickle;");
    pythonCode.append("import json;");
    pythonCode.append("loaded_pickle = pickle.load(open('");
    pythonCode.append(picklePath.toStdString());
    pythonCode.append("','rb'));");
    pythonCode.append("modal_dict = {key: loaded_pickle[key] for key in loaded_pickle.keys() if key in "
                      "['modalities','num_modalities']};");
    pythonCode.append("json.dump(modal_dict, open('");
    pythonCode.append(jsonPath.toStdString());
    pythonCode.append("', 'w'))");

    args.push_back(pythonCode);
    try
    {
      spExec->Execute(m_PythonPath.toStdString(), "python3", args);
    }
    catch (const mitk::Exception &e)
    {
      MITK_ERROR << "Pickle parsing FAILED!" << e.GetDescription(); // SHOW ERROR
      WriteStatusMessage(
        "Parsing failed in backend. Multiple Modalities will now have to be manually entered by the user.");
      return QString("");
    }
  }
  return jsonPath;
}

void QmitknnUNetToolGUI::DisplayMultiModalInfoFromJSON(const QString &jsonPath)
{
  if (QFile::exists(jsonPath))
  {
    QFile file(jsonPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QByteArray bytes = file.readAll();
      file.close();

      QJsonParseError jsonError;
      QJsonDocument document = QJsonDocument::fromJson(bytes, &jsonError);
      if (jsonError.error != QJsonParseError::NoError)
      {
        MITK_INFO << "fromJson failed: " << jsonError.errorString().toStdString() << endl;
        return;
      }
      if (document.isObject())
      {
        QJsonObject jsonObj = document.object();
        int num_mods = jsonObj["num_modalities"].toInt();
        ClearAllModalLabels();
        if (num_mods > 1)
        {
          m_Controls.multiModalBox->setChecked(true);
          m_Controls.multiModalBox->setEnabled(false);
          m_Controls.multiModalSpinBox->setValue(num_mods - 1);
          m_Controls.advancedSettingsLayout->update();
          QJsonObject obj = jsonObj.value("modalities").toObject();
          QStringList keys = obj.keys();
          int count = 0;
          for (auto key : keys)
          {
            auto value = obj.take(key);
            QLabel *label = new QLabel("<i>" + value.toString() + "</i>", this);
            m_ModalLabels.push_back(label);
            m_Controls.advancedSettingsLayout->addWidget(label, m_UI_ROWS + 1 + count, 0);
            count++;
          }
          m_Controls.multiModalSpinBox->setMinimum(num_mods - 1);
          m_Controls.advancedSettingsLayout->update();
        }
        else
        {
          m_Controls.multiModalSpinBox->setMinimum(0);
          m_Controls.multiModalBox->setChecked(false);
        }
      }
    }
  }
}
