#include "QmitknnUNetToolGUI.h"
#include <QDir>
#include <QDirIterator>
#include <QmitknnUNetEnsembleLayout.h>
#include <algorithm>
#include <ctkCollapsibleGroupBox.h>

void QmitknnUNetToolGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
}

void QmitknnUNetToolGUI::ClearAllModalities()
{
  m_Controls.multiModalSpinBox->setMinimum(0);
  m_Controls.multiModalBox->setChecked(false);
  ClearAllModalLabels();
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
  m_Controls.multiModalSpinBox->setVisible(false);
  m_Controls.multiModalBox->setEnabled(false);
  ClearAllComboBoxes();
  ClearAllModalities();
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

void QmitknnUNetToolGUI::OnRefreshPresssed()
{
  const QString resultsFolder = m_Controls.modeldirectoryBox->directory();
  OnDirectoryChanged(resultsFolder);
}

void QmitknnUNetToolGUI::OnDirectoryChanged(const QString &resultsFolder)
{
  m_Controls.previewButton->setEnabled(false);
  ClearAllComboBoxes();
  ClearAllModalities();
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
  ClearAllModalities();
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
      ShowEnsembleLayout(true);
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
      m_Controls.previewButton->setEnabled(true);
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
      ShowEnsembleLayout(false);
      auto trainerPlanners = m_ParentFolder->getTrainerPlannersForTask<QStringList>(selectedTask, model);
      QStringList trainers, planners;
      std::tie(trainers, planners) = ExtractTrainerPlannerFromString(trainerPlanners);
      m_Controls.trainerBox->clear();
      m_Controls.plannerBox->clear();
      std::for_each(
        trainers.begin(), trainers.end(), [this](QString trainer) { m_Controls.trainerBox->addItem(trainer); });
      std::for_each(
        planners.begin(), planners.end(), [this](QString planner) { m_Controls.plannerBox->addItem(planner); });
    }
  }
  else if (!m_EnsembleParams.empty())
  {
    for (auto &layout : m_EnsembleParams)
    {
      if (box == layout->modelBox)
      {
        layout->trainerBox->clear();
        layout->plannerBox->clear();
        auto trainerPlanners = m_ParentFolder->getTrainerPlannersForTask<QStringList>(selectedTask, model);
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
  auto *box = qobject_cast<ctkComboBox *>(sender());
  if (box == m_Controls.plannerBox)
  {
    m_Controls.foldBox->clear();
    auto selectedTrainer = m_Controls.trainerBox->currentText();
    auto selectedTask = m_Controls.taskBox->currentText();
    auto selectedModel = m_Controls.modelBox->currentText();
    auto folds = m_ParentFolder->getFoldsForTrainerPlanner<QStringList>(
      selectedTrainer, plannerSelected, selectedTask, selectedModel);
    std::for_each(folds.begin(),
                  folds.end(),
                  [this](QString fold)
                  {
                    if (fold.startsWith("fold_", Qt::CaseInsensitive)) // imposed by nnUNet
                      m_Controls.foldBox->addItem(fold);
                  });
    if (m_Controls.foldBox->count() != 0)
    {
      CheckAllInCheckableComboBox(m_Controls.foldBox);
      m_Controls.previewButton->setEnabled(true);
      const QString parentPath = QDir::cleanPath(m_ParentFolder->getResultsFolder() + QDir::separator() + "nnUNet" +
                                                 QDir::separator() + selectedModel + QDir::separator() + selectedTask +
                                                 QDir::separator() + selectedTrainer + QString("__") + plannerSelected);
      const QString jsonPath = this->DumpJSONfromPickle(parentPath);
      if (!jsonPath.isEmpty())
      {
        this->DisplayMultiModalInfoFromJSON(jsonPath);
      }
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
        std::for_each(folds.begin(),
                      folds.end(),
                      [&layout](const QString &fold)
                      {
                        if (fold.startsWith("fold_", Qt::CaseInsensitive)) // imposed by nnUNet
                          layout->foldBox->addItem(fold);
                      });
        if (layout->foldBox->count() != 0)
        {
          CheckAllInCheckableComboBox(layout->foldBox);
          m_Controls.previewButton->setEnabled(true);
          const QString parentPath = QDir::cleanPath(
            m_ParentFolder->getResultsFolder() + QDir::separator() + "nnUNet" + QDir::separator() + selectedModel +
            QDir::separator() + selectedTask + QDir::separator() + selectedTrainer + QString("__") + plannerSelected);
          const QString jsonPath = this->DumpJSONfromPickle(parentPath);
          if (!jsonPath.isEmpty())
          {
            this->DisplayMultiModalInfoFromJSON(jsonPath);
          }
        }
        break;
      }
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
      OnPythonPathChanged(path); // recall same function for new path validation
      m_Controls.pythonEnvComboBox->insertItem(0, path);
      m_Controls.pythonEnvComboBox->setCurrentIndex(0);
    }
  }
  else if (!IsNNUNetInstalled(pyEnv))
  {
    std::string warning =
      "WARNING: nnUNet is not detected on the Python environment you selected. Please select another "
      "environment or create one. For more info refer https://github.com/MIC-DKFZ/nnUNet";
    ShowErrorMessage(warning);
    DisableEverything();
  }
  else
  {
    m_Controls.modeldirectoryBox->setEnabled(true);
    m_Controls.previewButton->setEnabled(true);
    m_Controls.refreshdirectoryBox->setEnabled(true);
    m_Controls.multiModalBox->setEnabled(true);
    QString setVal = FetchResultsFolderFromEnv();
    if (!setVal.isEmpty())
    {
      m_Controls.modeldirectoryBox->setDirectory(setVal);
    }
    OnRefreshPresssed();
    m_PythonPath = pyEnv.mid(pyEnv.indexOf(" ") + 1);
    if (!(m_PythonPath.endsWith("bin", Qt::CaseInsensitive) || m_PythonPath.endsWith("bin/", Qt::CaseInsensitive)))
    {
      m_PythonPath += QDir::separator() + QString("bin");
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
      m_Controls.multiModalSpinLabel->setVisible(visibility);
      m_Controls.multiModalSpinBox->setVisible(visibility);
      if (visibility)
      {
        QmitkDataStorageComboBox *defaultImage = new QmitkDataStorageComboBox(this, true);
        defaultImage->setObjectName(QString("multiModal_" + QString::number(0)));
        defaultImage->SetPredicate(m_MultiModalPredicate);
        mitk::nnUNetTool::Pointer tool = this->GetConnectedToolAs<mitk::nnUNetTool>();
        if (tool != nullptr)
        {
          defaultImage->SetDataStorage(tool->GetDataStorage());
          defaultImage->SetSelectedNode(tool->GetRefNode());
        }
        m_Controls.advancedSettingsLayout->addWidget(defaultImage, m_UI_ROWS + m_Modalities.size() + 1, 1, 1, 3);
        m_Modalities.push_back(defaultImage);
      }
      else
      {
        OnModalitiesNumberChanged(0);
        m_Controls.multiModalSpinBox->setValue(0);
        delete m_Modalities[0];
        m_Modalities.pop_back();
        ClearAllModalLabels();
      }
    }
  }
}

void QmitknnUNetToolGUI::OnModalitiesNumberChanged(int num)
{
  while (num > static_cast<int>(m_Modalities.size() - 1))
  {
    QmitkDataStorageComboBox *multiModalBox = new QmitkDataStorageComboBox(this, true);
    mitk::nnUNetTool::Pointer tool = this->GetConnectedToolAs<mitk::nnUNetTool>();
    multiModalBox->SetDataStorage(tool->GetDataStorage());
    multiModalBox->SetPredicate(m_MultiModalPredicate);
    multiModalBox->setObjectName(QString("multiModal_" + QString::number(m_Modalities.size() + 1)));
    m_Controls.advancedSettingsLayout->addWidget(multiModalBox, m_UI_ROWS + m_Modalities.size() + 1, 1, 1, 3);
    m_Modalities.push_back(multiModalBox);
  }
  while (num < static_cast<int>(m_Modalities.size() - 1) && !m_Modalities.empty())
  {
    QmitkDataStorageComboBox *child = m_Modalities.back();
    if (child->objectName() == "multiModal_0")
    {
      std::iter_swap(m_Modalities.end() - 2, m_Modalities.end() - 1);
      child = m_Modalities.back();
    }
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

void QmitknnUNetToolGUI::SegmentationProcessFailed()
{
  WriteErrorMessage(
    "<b>STATUS: </b><i>Error in the segmentation process. <br>No resulting segmentation can be loaded.</i>");
  this->setCursor(Qt::ArrowCursor);
  std::stringstream stream;
  stream << "Error in the segmentation process. No resulting segmentation can be loaded.";
  ShowErrorMessage(stream.str());
}

void QmitknnUNetToolGUI::SegmentationResultHandler(mitk::nnUNetTool *tool, bool forceRender)
{
  if (forceRender)
  {
    tool->RenderOutputBuffer();
  }
  this->SetLabelSetPreview(tool->GetMLPreview());
  WriteStatusMessage("<b>STATUS: </b><i>Segmentation task finished successfully. <br> If multiple Preview objects are selected to Confirm, "
  "they will be merged. Any unselected Preview objects will be lost.</i>");
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
