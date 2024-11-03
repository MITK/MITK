/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentationTaskListWidget.h"

#include <mitkCoreServices.h>
#include <mitkINodeSelectionService.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <mitkDICOMQIPropertyHelper.h>
#include <mitkFileSystem.h>
#include <mitkIOUtil.h>
#include <mitkMultiLabelIOHelper.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateFunction.h>
#include <mitkRenderingManager.h>
#include <mitkSceneIO.h>
#include <mitkSegmentationHelper.h>
#include <mitkToolManagerProvider.h>

#include <QmitkFindSegmentationTaskDialog.h>
#include <QmitkStaticDynamicSegmentationDialog.h>
#include <QmitkStyleManager.h>

#include <ui_QmitkSegmentationTaskListWidget.h>

#include <QFile>
#include <QFileSystemWatcher>
#include <QMessageBox>
#include <QShortcut>
#include <QTextStream>

#include <nlohmann/json.hpp>

namespace
{
  mitk::IPreferences* GetSegmentationPreferences()
  {
    return mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.views.segmentation");
  }

  fs::path GetInputLocation(const mitk::BaseData* data)
  {
    std::string result;

    if (data != nullptr)
      data->GetPropertyList()->GetStringProperty("MITK.IO.reader.inputlocation", result);

    return result;
  }

  QString ColorString(const QString& string, const QColor& color, const QColor& backgroundColor = QColor::Invalid)
  {
    if (!color.isValid() && !backgroundColor.isValid())
      return string;

    auto result = QStringLiteral("<span style=\"");
    QStringList strings;

    if (color.isValid())
      strings << QString("color: %1;").arg(color.name());

    if (backgroundColor.isValid())
      strings << QString("background-color: %1;").arg(backgroundColor.name());

    result += strings.join(' ') + QString("\">%1</span>").arg(string);

    return result;
  }

  mitk::DataStorage::SetOfObjects::ConstPointer GetSubset(const mitk::DataStorage* dataStorage, const mitk::NodePredicateBase* condition, const mitk::DataNode* removedDataNode)
  {
    auto subset = dataStorage->GetSubset(condition);

    if (nullptr != removedDataNode)
    {
      auto actualSubset = mitk::DataStorage::SetOfObjects::New();

      for (auto node : *subset)
      {
        if (node != removedDataNode)
          actualSubset->push_back(node);
      }

      return actualSubset;
    }

    return subset;
  }

  QString ReadFileAsString(const QString& path)
  {
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      mitkThrow() << "Could not open file \"" << path.toStdString() << "\"!";

    QTextStream stream(&file);
    return stream.readAll();
  }

  std::map<int, mitk::DataNode*> SortNodesByLayer(const mitk::DataStorage::SetOfObjects* nodes)
  {
    std::map<int, mitk::DataNode*> sortedNodes;
    int noLayerIndex = -1;

    for (auto it = nodes->Begin(); it != nodes->End(); ++it)
    {
      int layer = 0;
      bool hasLayer = it->Value()->GetIntProperty("layer", layer);

      if (hasLayer)
      {
        sortedNodes[layer] = it->Value();
      }
      else
      {
        sortedNodes[noLayerIndex--] = it->Value();
      }
    }

    return sortedNodes;
  }

  void TransferDataNodes(const mitk::DataStorage* srcStorage, const mitk::DataStorage::SetOfObjects* srcNodes,
                         mitk::DataStorage* destStorage, mitk::DataNode* destRoot)
  {
    const auto sortedSrcNodes = SortNodesByLayer(srcNodes);

    for (auto [layer, srcNode] : sortedSrcNodes)
    {
      destStorage->Add(srcNode, destRoot);

      auto childNodes = srcStorage->GetDerivations(srcNode);

      if (!childNodes->empty())
        TransferDataNodes(srcStorage, childNodes, destStorage, srcNode);
    }
  }

  void TransferDataStorage(const mitk::DataStorage* srcStorage, mitk::DataStorage* destStorage, mitk::DataNode* destRoot)
  {
    auto isRootNode = mitk::NodePredicateFunction::New([srcStorage](const mitk::DataNode* node) {
      return srcStorage->GetSources(node)->empty();
    });

    auto rootNodes = srcStorage->GetSubset(isRootNode);

    TransferDataNodes(srcStorage, rootNodes, destStorage, destRoot);
  }
}

/* This constructor has three objectives:
 *   1. Do widget initialization that cannot be done in the .ui file
 *   2. Connect signals and slots (including shortcuts)
 *   3. Explicitly trigger a reset to a valid initial widget state
 */
QmitkSegmentationTaskListWidget::QmitkSegmentationTaskListWidget(QWidget* parent)
  : QWidget(parent),
    m_Ui(new Ui::QmitkSegmentationTaskListWidget),
    m_FileSystemWatcher(new QFileSystemWatcher(this)),
    m_DataStorage(nullptr),
    m_ImageNode(nullptr),
    m_SegmentationNode(nullptr),
    m_UnsavedChanges(false)
{
  m_Ui->setupUi(this);

  m_Ui->selectionWidget->SetNodePredicate(mitk::TNodePredicateDataType<mitk::SegmentationTaskList>::New());

  m_Ui->progressBar->setStyleSheet(QString("QProgressBar::chunk { background-color: %1; }").arg(QmitkStyleManager::GetIconAccentColor()));

  m_Ui->findButton->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_find.svg")));
  m_Ui->storeButton->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/document-save.svg")));

  using Self = QmitkSegmentationTaskListWidget;

  connect(m_Ui->selectionWidget, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged, this, &Self::OnSelectionChanged);
  connect(m_Ui->previousButton, &QToolButton::clicked, this, &Self::OnPreviousButtonClicked);
  connect(m_Ui->nextButton, &QToolButton::clicked, this, &Self::OnNextButtonClicked);
  connect(m_Ui->findButton, &QToolButton::clicked, this, &Self::OnFindButtonClicked);
  connect(m_Ui->loadButton, &QPushButton::clicked, this, &Self::OnLoadButtonClicked);
  connect(m_Ui->storeButton, &QPushButton::clicked, this, &Self::OnStoreButtonClicked);
  connect(m_Ui->acceptButton, &QPushButton::clicked, this, &Self::OnAcceptButtonClicked);

  connect(m_Ui->formWidget, &QmitkForm::Submit, this, &Self::OnFormSubmission);

  connect(m_FileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &Self::OnResultDirectoryChanged);

  auto* prevShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key::Key_P), this);
  connect(prevShortcut, &QShortcut::activated, this, &Self::OnPreviousTaskShortcutActivated);

  auto* prevUndoneShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key::Key_P), this);
  connect(prevUndoneShortcut, &QShortcut::activated, this, &Self::OnPreviousTaskShortcutActivated);

  auto* nextShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key::Key_N), this);
  connect(nextShortcut, &QShortcut::activated, this, &Self::OnNextTaskShortcutActivated);

  auto* nextUndoneShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key::Key_N), this);
  connect(nextUndoneShortcut, &QShortcut::activated, this, &Self::OnNextTaskShortcutActivated);

  auto *findTaskShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key::Key_F), this);
  connect(findTaskShortcut, &QShortcut::activated, this, &Self::OnFindTaskShortcutActivated);

  auto* loadShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key::Key_L), this);
  connect(loadShortcut, &QShortcut::activated, this, &Self::OnLoadTaskShortcutActivated);

  auto* storeShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key::Key_S), parent);
  connect(storeShortcut, &QShortcut::activated, this, &Self::OnStoreInterimResultShortcutActivated);

  auto* acceptShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key::Key_A), parent);
  connect(acceptShortcut, &QShortcut::activated, this, &Self::OnAcceptSegmentationShortcutActivated);

  this->ResetControls();
  this->CheckDataStorage();
}

QmitkSegmentationTaskListWidget::~QmitkSegmentationTaskListWidget()
{
}

void QmitkSegmentationTaskListWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
  m_Ui->selectionWidget->SetDataStorage(dataStorage); // Triggers OnSelectionChanged()
  m_Ui->selectionWidget->SetAutoSelectNewNodes(true);

  this->CheckDataStorage();
}

void QmitkSegmentationTaskListWidget::CheckDataStorage(const mitk::DataNode* removedNode)
{
  QString warning;

  if (nullptr == m_DataStorage)
  {
    warning = QStringLiteral(
      "<h3>Developer warning</h3><p>Call <code>SetDataStorage()</code> to fully initialize "
      "this instance of <code>QmitkSegmentationTaskListWidget</code>.</p>");
  }
  else
  {
    auto isTaskList = mitk::TNodePredicateDataType<mitk::SegmentationTaskList>::New();
    auto taskListNodes = GetSubset(m_DataStorage, isTaskList, removedNode);

    if (taskListNodes->empty())
    {
      warning = QStringLiteral(
        "<h3>No segmentation task list found</h3><p>Load a segmentation task list to use "
        "this plugin.</p>");
    }
    else if (taskListNodes->Size() > 1)
    {
      warning = QStringLiteral(
        "<h3>More than one segmentation task list found</h3><p>Unload everything but a "
        "single segmentation task list to use this plugin.</p>");
    }
    else
    {
      const auto* taskListNode = (*taskListNodes)[0].GetPointer();

      auto isTaskListNode = mitk::NodePredicateFunction::New([taskListNode](const mitk::DataNode* node) {
        return node == taskListNode;
      });

      auto isChildOfTaskListNode = mitk::NodePredicateFunction::New([this, isTaskListNode](const mitk::DataNode* node) {
        return !m_DataStorage->GetSources(node, isTaskListNode, false)->empty();
      });

      auto isHelperObject = mitk::NodePredicateProperty::New("helper object");
      auto isUndesiredNode = mitk::NodePredicateNot::New(mitk::NodePredicateOr::New(
        isTaskListNode,
        isChildOfTaskListNode,
        isHelperObject));

      auto unreleatedData = GetSubset(m_DataStorage, isUndesiredNode, removedNode);

      if (!unreleatedData->empty())
      {
        warning = QStringLiteral(
          "<h3>Unrelated data found</h3><p>Unload everything but a single segmentation task "
          "list to use this plugin.</p><p>Unreleated data:<ul>");

        for (auto node : *unreleatedData)
        {
          warning += "<li>" + node->GetName();

          if (auto data = node->GetData(); data != nullptr)
            warning += QString(" (%1)").arg(data->GetNameOfClass());

          warning += "</li>";
        }

        warning += "</ul></p>";
      }
    }
  }

  m_Ui->label->setText("<span style=\"color: " + QmitkStyleManager::GetIconAccentColor() + "\">" + warning + "</span>");
  m_Ui->label->setVisible(!warning.isEmpty());
  m_Ui->widget->setVisible(warning.isEmpty());
}

void QmitkSegmentationTaskListWidget::OnUnsavedChangesSaved()
{
  if (m_UnsavedChanges)
  {
    m_UnsavedChanges = false;

    if (this->ActiveTaskIsShown())
      this->UpdateDetailsLabel();
  }
}

/* Make sure that the widget transitions into a valid state whenever the
 * selection changes.
 */
void QmitkSegmentationTaskListWidget::OnSelectionChanged(const QmitkSingleNodeSelectionWidget::NodeList& nodes)
{
  this->UnloadTasks();
  this->ResetControls();

  if (!nodes.empty())
  {
    m_TaskListNode = nodes.front();
    auto taskList = dynamic_cast<mitk::SegmentationTaskList*>(m_TaskListNode->GetData());

    if (taskList != nullptr)
    {
      this->OnTaskListChanged(taskList);
      return;
    }
  }

  this->SetTaskList(nullptr);
  m_TaskListNode = nullptr;
}

/* Reset all controls to a default state as a common basis for further
 * adjustments.
 */
void QmitkSegmentationTaskListWidget::ResetControls()
{
  m_Ui->progressBar->setEnabled(false);
  m_Ui->progressBar->setFormat("");
  m_Ui->progressBar->setValue(0);
  m_Ui->progressBar->setMaximum(1);

  m_Ui->previousButton->setEnabled(false);
  m_Ui->nextButton->setEnabled(false);

  this->UpdateLoadButton();
  this->UpdateDetailsLabel();
  this->UpdateStoreAndAcceptButtons();
}

/* If the segmentation task changed, reset all member variables to expected
 * default values and reset the file system watcher.
 */
void QmitkSegmentationTaskListWidget::SetTaskList(mitk::SegmentationTaskList* taskList)
{
  if (m_TaskList != taskList)
  {
    m_TaskList = taskList;

    if (taskList != nullptr)
    {
      this->SetCurrentTaskIndex(0);
    }
    else
    {
      this->SetCurrentTaskIndex(std::nullopt);
    }

    this->ResetFileSystemWatcher();
  }
}

void QmitkSegmentationTaskListWidget::ResetFileSystemWatcher()
{
  auto paths = m_FileSystemWatcher->directories();

  if (!paths.empty())
    m_FileSystemWatcher->removePaths(paths);

  if (m_TaskList.IsNotNull())
  {
    for (const auto& task : *m_TaskList)
    {
      auto resultPath = m_TaskList->GetAbsolutePath(task.GetResult()).remove_filename();

      if (!fs::exists(resultPath))
      {
        try
        {
          fs::create_directories(resultPath);
        }
        catch (const fs::filesystem_error& e)
        {
          MITK_ERROR << e.what();
        }
      }

      if (fs::exists(resultPath))
        m_FileSystemWatcher->addPath(QString::fromStdString(resultPath.string()));
    }
  }
}

void QmitkSegmentationTaskListWidget::OnResultDirectoryChanged(const QString&)
{
  // TODO: If a segmentation was modified ("Unsaved changes"), saved ("Done"),
  // and then the file is deleted, the status should be "Unsaved changes"
  // instead of "Not done".
  this->UpdateProgressBar();
  this->UpdateDetailsLabel();
}

void QmitkSegmentationTaskListWidget::UpdateProgressBar()
{
  int progress = 0;

  for (size_t i = 0; i < m_TaskList->GetNumberOfTasks(); ++i)
  {
    if (m_TaskList->IsDone(i))
      ++progress;
  }

  m_Ui->progressBar->setValue(progress);
}

/* Provided that a valid segmentation task list is currently selected and the
 * widget is in its default state, update all controls accordingly.
 * TODO: Then, load the first unfinished task, if any.
 */
void QmitkSegmentationTaskListWidget::OnTaskListChanged(mitk::SegmentationTaskList* taskList)
{
  this->SetTaskList(taskList);

  const auto numTasks = taskList->GetNumberOfTasks();

  m_Ui->progressBar->setMaximum(numTasks);
  m_Ui->progressBar->setFormat(QStringLiteral("%v/%m Task(s) done"));
  m_Ui->progressBar->setEnabled(true);

  this->UpdateProgressBar();

  m_Ui->loadButton->setEnabled(true);

  if (numTasks > 1)
    m_Ui->nextButton->setEnabled(true);

  // TODO: The line below should be enough but it is happening too early
  // even before the RenderingManager has any registered render windows,
  // resulting in mismatching renderer and data geometries.

  // this->LoadNextUnfinishedTask();
}

/* If possible, change the currently displayed task to the previous task.
 * Enable/disable navigation buttons according to the task's position.
 */
void QmitkSegmentationTaskListWidget::OnPreviousButtonClicked()
{
  auto current = m_CurrentTaskIndex.value();

  // If the shift modifier key is pressed, look for the previous undone task.
  if (QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier))
  {
    if (current > 0)
    {
      for (decltype(current) i = current; i > 0; --i)
      {
        if (!m_TaskList->IsDone(i - 1))
        {
          this->SetCurrentTaskIndex(i - 1);
          break;
        }
      }
    }
  }
  else
  {
    if (current != 0)
      this->SetCurrentTaskIndex(current - 1);
  }

  this->UpdateNavigationButtons();
}

/* If possible, change the currently displayed task to the next task.
 * Enable/disable navigation buttons according to the task's position.
 */
void QmitkSegmentationTaskListWidget::OnNextButtonClicked()
{
  const auto numTasks = m_TaskList->GetNumberOfTasks();
  auto current = m_CurrentTaskIndex.value();

  // If the shift modifier key is pressed, look for the next undone task.
  if (QApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier))
  {
    for (std::remove_const_t<decltype(numTasks)> i = current + 1; i < numTasks; ++i)
    {
      if (!m_TaskList->IsDone(i))
      {
        this->SetCurrentTaskIndex(i);
        break;
      }
    }
  }
  else
  {
    if (current < numTasks - 1)
      this->SetCurrentTaskIndex(current + 1);
  }

  this->UpdateNavigationButtons();
}

void QmitkSegmentationTaskListWidget::OnFindButtonClicked()
{
  if (m_TaskList.IsNull())
    return;

  QmitkFindSegmentationTaskDialog dialog;
  dialog.SetTaskList(m_TaskList);

  if (dialog.exec() != QDialog::Accepted)
    return;

  if (!dialog.GetSelectedTask().has_value())
    return;

  this->SetCurrentTaskIndex(dialog.GetSelectedTask());

  if (dialog.LoadSelectedTask())
  {
    if (!m_ActiveTaskIndex.has_value() || m_ActiveTaskIndex.value() != dialog.GetSelectedTask().value())
      this->OnLoadButtonClicked();
  }
}

void QmitkSegmentationTaskListWidget::UpdateNavigationButtons()
{
  if (m_TaskList.IsNull() || m_TaskList->GetNumberOfTasks() == 0)
  {
    m_Ui->previousButton->setEnabled(false);
    m_Ui->nextButton->setEnabled(false);

    return;
  }

  const auto maxIndex = m_TaskList->GetNumberOfTasks() - 1;
  const auto current = m_CurrentTaskIndex.value();

  m_Ui->previousButton->setEnabled(current != 0);
  m_Ui->nextButton->setEnabled(current != maxIndex);
}

/* Update affected controls when the currently displayed task changed.
 */
void QmitkSegmentationTaskListWidget::OnCurrentTaskChanged()
{
  this->UpdateLoadButton();
  this->UpdateNavigationButtons();
  this->UpdateDetailsLabel();
  this->UpdateFormWidget();
  this->UpdateStoreAndAcceptButtons();
}

/* Update the load button according to the currently displayed task.
 */
void QmitkSegmentationTaskListWidget::UpdateLoadButton()
{
  auto text = !this->ActiveTaskIsShown()
    ? QStringLiteral("Load task")
    : QStringLiteral("Task");

  if (m_CurrentTaskIndex.has_value())
  {
    const auto current = m_CurrentTaskIndex.value();

    if (m_TaskList.IsNotNull())
    {
      text += QString(" %1/%2").arg(current + 1).arg(m_TaskList->GetNumberOfTasks());

      if (m_TaskList->HasName(current))
        text += QStringLiteral(":\n") + QString::fromStdString(m_TaskList->GetName(current));
    }

    m_Ui->loadButton->setDisabled(this->ActiveTaskIsShown());
  }
  else
  {
    m_Ui->loadButton->setEnabled(false);
  }

  m_Ui->loadButton->setText(text);
}

/* Update the details label according to the currently display task.
 * The text is composed of the status of the task and a variable number
 * of text blocks according to the optional values provided by the task.
 */
void QmitkSegmentationTaskListWidget::UpdateDetailsLabel()
{
  if (!m_CurrentTaskIndex.has_value())
  {
    m_Ui->detailsLabel->clear();
    return;
  }

  const auto current = m_CurrentTaskIndex.value();
  bool isDone = m_TaskList->IsDone(current);

  auto details = QString("<p><b>Status: %1</b> / <b>").arg(this->ActiveTaskIsShown()
    ? ColorString("Active", Qt::white, QColor(Qt::green).darker())
    : ColorString("Inactive", Qt::white, QColor(Qt::red).darker()));

  if (m_UnsavedChanges && this->ActiveTaskIsShown())
  {
    details += QString("%1</b></p>").arg(ColorString("Unsaved changes", Qt::white, QColor(Qt::red).darker()));
  }
  else
  {
    details += QString("%1</b></p>").arg(isDone
      ? ColorString("Done", Qt::white, QColor(Qt::green).darker())
      : ColorString("Not done", Qt::white, QColor(Qt::red).darker()));
  }

  if (m_TaskList->HasDescription(current))
    details += QString("<p><b>Description:</b> %1</p>").arg(QString::fromStdString(m_TaskList->GetDescription(current)));

  QStringList stringList;

  if (m_TaskList->HasImage(current))
    stringList << QString::fromStdString("<b>Image:</b> " + m_TaskList->GetImage(current).string());

  if (m_TaskList->HasSegmentation(current))
    stringList << QString::fromStdString("<b>Segmentation:</b> " + m_TaskList->GetSegmentation(current).string());

  if (m_TaskList->HasLabelName(current))
    stringList << QString::fromStdString("<b>Label name:</b> " + m_TaskList->GetLabelName(current));

  if (m_TaskList->HasLabelNameSuggestions(current))
    stringList << QString::fromStdString("<b>Label name suggestions:</b> " + m_TaskList->GetLabelNameSuggestions(current).string());

  if (m_TaskList->HasPreset(current))
    stringList << QString::fromStdString("<b>Label set preset:</b> " + m_TaskList->GetPreset(current).string());

  if (m_TaskList->HasDynamic(current))
    stringList << QString("<b>Segmentation type:</b> %1").arg(m_TaskList->GetDynamic(current) ? "Dynamic" : "Static");

  if (!stringList.empty())
    details += QString("<p>%1</p>").arg(stringList.join(QStringLiteral("<br>")));

  m_Ui->detailsLabel->setText(details);
}

void QmitkSegmentationTaskListWidget::UpdateFormWidget()
{
  if (!m_CurrentTaskIndex.has_value())
  {
    m_Ui->formWidget->hide();
    return;
  }

  const auto current = m_CurrentTaskIndex.value();

  if (!ActiveTaskIsShown() || !m_TaskList->HasForm(current))
  {
    m_Ui->formWidget->hide();
    return;
  }

  auto form = m_TaskList->GetForm(current);

  try
  {
    const auto formPath = m_TaskList->GetAbsolutePath(form.Path);
    m_Form = nlohmann::json::parse(ReadFileAsString(QString::fromStdString(formPath.string())).toStdString());
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << e.GetDescription();
    return;
  }

  m_Form.AddSupplement("Task");
  m_Ui->formWidget->SetForm(&m_Form);

  const auto responsesPath = m_TaskList->GetAbsolutePath(form.Result);
  m_Ui->formWidget->SetResponsesPath(responsesPath);

  m_Ui->formWidget->show();
}

void QmitkSegmentationTaskListWidget::OnFormSubmission()
{
  m_Form.SetSupplement("Task", m_TaskList->GetName(m_CurrentTaskIndex.value()));
}

void QmitkSegmentationTaskListWidget::UpdateStoreAndAcceptButtons()
{
  auto activeTaskIsShown = this->ActiveTaskIsShown();

  m_Ui->storeButton->setVisible(activeTaskIsShown);
  m_Ui->acceptButton->setEnabled(activeTaskIsShown);
}

/* Load/activate the currently displayed task. Unload all data nodes from
 * previously active tasks first, but spare and reuse the image if possible.
 */
void QmitkSegmentationTaskListWidget::OnLoadButtonClicked()
{
  if (!this->HandleUnsavedChanges() || m_UnsavedChanges)
    return;

  m_Ui->loadButton->setEnabled(false);

  QApplication::setOverrideCursor(Qt::BusyCursor);
  this->LoadTask(this->GetImageDataNode(m_CurrentTaskIndex.value()));
  QApplication::restoreOverrideCursor();
}

/* If present, return the image data node for the task with the specified
 * index. Otherwise, return nullptr.
 */
mitk::DataNode* QmitkSegmentationTaskListWidget::GetImageDataNode(size_t index) const
{
  if (!m_TaskList->HasImage(index))
    return nullptr;

  const auto imagePath = m_TaskList->GetAbsolutePath(m_TaskList->GetImage(index));

  auto lambda = [imagePath](const mitk::DataNode* node) {
    return imagePath == GetInputLocation(node->GetData());
  };

  auto predicate = mitk::NodePredicateFunction::New(lambda);
  auto imageNodes = m_DataStorage->GetDerivations(m_TaskListNode, predicate);

  return !imageNodes->empty()
    ? imageNodes->front()
    : nullptr;
}

/* Unload all task data nodes but spare the passed image data node.
 */
void QmitkSegmentationTaskListWidget::UnloadTasks(const mitk::DataNode* skip)
{
  this->UnsubscribeFromActiveSegmentation();

  m_ImageNode = nullptr;
  m_SegmentationNode = nullptr;

  if (m_TaskListNode.IsNotNull())
  {
    auto notSkip = mitk::NodePredicateFunction::New([skip](const mitk::DataNode* node) {
      return skip != node;
    });

    auto taskNodes = m_DataStorage->GetDerivations(m_TaskListNode, notSkip, false);

    m_DataStorage->Remove(taskNodes);

    // Removal is done in a random order instead of traveling upwards from
    // child nodes to parent nodes. This may result in temporary orphan nodes
    // hierarchically existing outside of the segmentation task list parent
    // node. To prevent the detection of unrelated data, which would put this
    // widget into a warning mode, do a final data storage check after
    // everything was removed.
    this->CheckDataStorage();
  }

  this->SetActiveTaskIndex(std::nullopt);
}

void QmitkSegmentationTaskListWidget::LoadNextUnfinishedTask()
{
  const auto current = m_CurrentTaskIndex.value();
  const auto numTasks = m_TaskList->GetNumberOfTasks();

  for (size_t unboundNext = current; unboundNext < current + numTasks; ++unboundNext)
  {
    auto next = unboundNext % numTasks;

    if (!m_TaskList->IsDone(next))
    {
      this->SetCurrentTaskIndex(next);
      this->OnLoadButtonClicked();
      break;
    }
  }
}

/* Load/activate the currently displayed task. The task must specify an image
 * or a scene. The segmentation is either created from scratch with an optional
 * name for the first label, possibly based on a label set preset specified by
 * the task, or loaded as specified by the task. If a result file does
 * exist, it is chosen as segmentation instead.
 */
void QmitkSegmentationTaskListWidget::LoadTask(mitk::DataNode::Pointer imageNode)
{
  const auto current = m_CurrentTaskIndex.value();

  mitk::Image::Pointer image;
  mitk::LabelSetImage::Pointer segmentation;
  mitk::DataStorage::Pointer scene;

  // If the task has a scene, unload everything from before and load the scene.
  // If the task has an image instead, unload everything but the previous image
  // if it is the same image and can be reused. Otherwise load the new image.

  if (m_TaskList->HasScene(current))
  {
    this->UnloadTasks();

    try
    {
      const auto scenePath = m_TaskList->GetAbsolutePath(m_TaskList->GetScene(current).Path);
      auto sceneIO = mitk::SceneIO::New();
      scene = sceneIO->LoadScene(scenePath.string());
    }
    catch (const mitk::Exception& e)
    {
      QMessageBox::critical(this, "Error on loading scene", e.GetDescription());
      MITK_ERROR << e.GetDescription();
      return;
    }
  }
  else
  {
    this->UnloadTasks(imageNode);

    if (imageNode.IsNull())
    {
      try
      {
        const auto imagePath = m_TaskList->GetAbsolutePath(m_TaskList->GetImage(current));
        image = mitk::IOUtil::Load<mitk::Image>(imagePath.string());
      }
      catch (const mitk::Exception& e)
      {
        QMessageBox::critical(this, "Error on loading image", e.GetDescription());
        MITK_ERROR << e.GetDescription();
        return;
      }
    }
  }

  // If the task already has a result segmentation, load it instead of any other
  // given segmentation. Otherwise, if the task already has an interim result
  // segmentation, load it instead of any other given segmentation. Otherwise,
  // load a given segmentation if any.

  try
  {
    const auto resultPath = m_TaskList->GetAbsolutePath(m_TaskList->GetResult(current));
    const auto interimPath = m_TaskList->GetInterimPath(resultPath);

    if (fs::exists(resultPath))
    {
      segmentation = mitk::IOUtil::Load<mitk::LabelSetImage>(resultPath.string());
    }
    else if (fs::exists(interimPath))
    {
      segmentation = mitk::IOUtil::Load<mitk::LabelSetImage>(interimPath.string());
    }
    else if (m_TaskList->HasSegmentation(current))
    {
      const auto segmentationPath = m_TaskList->GetAbsolutePath(m_TaskList->GetSegmentation(current));
      segmentation = mitk::IOUtil::Load<mitk::LabelSetImage>(segmentationPath.string());
    }
  }
  catch (const mitk::Exception& e)
  {
    QMessageBox::critical(this, "Error on loading segmentation", e.GetDescription());
    MITK_ERROR << e.GetDescription();
    return;
  }

  // If the task has a scene and a segmentation was loaded already above,
  // replace the scene's segmentation if any. Otherwise, if no segmentation
  // was loaded so far, check if the scene provides a segmentation.

  mitk::DataNode::Pointer segmentationNode;

  if (scene.IsNotNull())
  {
    const auto imageNodeName = m_TaskList->GetScene(current).Image;
    imageNode = scene->GetNamedNode(imageNodeName);

    if (imageNode.IsNull())
    {
      auto errorMessage = QString("Could not find image node \"%1\" in scene %2!")
        .arg(QString::fromStdString(imageNodeName))
        .arg(QString::fromStdString(m_TaskList->GetScene(current).Path.string()));

      QMessageBox::critical(this, "Error on loading scene", errorMessage);
      MITK_ERROR << errorMessage.toStdString();
      return;
    }

    const auto segmentationNodeName = m_TaskList->GetScene(current).Segmentation;

    if (!segmentationNodeName.empty())
    {
      segmentationNode = scene->GetNamedNode(segmentationNodeName);

      if (segmentationNode == nullptr)
      {
        auto errorMessage = QString("Could not find segmentation node \"%1\" in scene %2!")
          .arg(QString::fromStdString(segmentationNodeName))
          .arg(QString::fromStdString(m_TaskList->GetScene(current).Path.string()));

        QMessageBox::critical(this, "Error on loading scene", errorMessage);
        MITK_ERROR << errorMessage.toStdString();
        return;
      }

      if (segmentation.IsNotNull())
      {
        segmentationNode->SetData(segmentation);
      }
      else
      {
        segmentation = dynamic_cast<mitk::LabelSetImage*>(segmentationNode->GetData());

        if (segmentation.IsNull())
        {
          auto errorMessage = QString("Data node \"%1\" is not a valid segmentation!")
            .arg(QString::fromStdString(segmentationNodeName));

          QMessageBox::critical(this, "Error on loading scene", errorMessage);
          MITK_ERROR << errorMessage.toStdString();
          return;
        }
      }
    }
  }

  if (imageNode.IsNull())
  {
    imageNode = mitk::DataNode::New();
    imageNode->SetData(image);

    m_DataStorage->Add(imageNode, m_TaskListNode);

    mitk::RenderingManager::GetInstance()->InitializeViews(image->GetTimeGeometry());
  }
  else
  {
    image = dynamic_cast<mitk::Image*>(imageNode->GetData());

    if (image.IsNull())
    {
      auto errorMessage = QString("Data node \"%1\" is not a valid image!")
        .arg(QString::fromStdString(imageNode->GetName()));

      QMessageBox::critical(this, "Error on loading scene", errorMessage);
      MITK_ERROR << errorMessage.toStdString();
      return;
    }
  }

  auto name = "Task " + std::to_string(current + 1);
  imageNode->SetName(name);

  if (segmentation.IsNull())
  {
    mitk::Image::ConstPointer templateImage = image;

    if (templateImage->GetDimension() > 3)
    {
      if (m_TaskList->HasDynamic(current))
      {
        if (!m_TaskList->GetDynamic(current))
          templateImage = mitk::SegmentationHelper::GetStaticSegmentationTemplate(image);
      }
      else
      {
        QmitkStaticDynamicSegmentationDialog dialog(this);
        dialog.SetReferenceImage(templateImage);
        dialog.exec();

        templateImage = dialog.GetSegmentationTemplate();
      }
    }

    segmentationNode = mitk::LabelSetImageHelper::CreateNewSegmentationNode(imageNode, templateImage, name);
    segmentation = static_cast<mitk::LabelSetImage*>(segmentationNode->GetData());

    if (m_TaskList->HasPreset(current))
    {
      const auto path = m_TaskList->GetAbsolutePath(m_TaskList->GetPreset(current));
      mitk::MultiLabelIOHelper::LoadLabelSetImagePreset(path.string(), segmentation);
    }
    else
    {
      auto label = mitk::LabelSetImageHelper::CreateNewLabel(segmentation);

      if (m_TaskList->HasLabelName(current))
        label->SetName(m_TaskList->GetLabelName(current));

      segmentation->AddLabel(label, segmentation->GetActiveLayer());
    }

    if (scene.IsNull())
    {
      m_DataStorage->Add(segmentationNode, imageNode);
    }
    else
    {
      scene->Add(segmentationNode, imageNode);
    }
  }
  else if (scene.IsNotNull())
  {
    segmentationNode->SetName(name);
  }
  else
  {
    segmentationNode = mitk::DataNode::New();
    segmentationNode->SetName(name);
    segmentationNode->SetData(segmentation);

    m_DataStorage->Add(segmentationNode, imageNode);
  }

  if (scene.IsNotNull())
  {
    TransferDataStorage(scene, m_DataStorage, m_TaskListNode);

    auto nodeSelectionService = mitk::CoreServices::GetNodeSelectionService();
    nodeSelectionService->SendSelection("org.mitk.views.segmentation/referenceNode", { imageNode });
    nodeSelectionService->SendSelection("org.mitk.views.segmentation/workingNode", { segmentationNode });

    mitk::RenderingManager::GetInstance()->InitializeViews(segmentation->GetTimeGeometry());
  }

  // Workaround for T29431. Remove when T26953 is fixed.
  mitk::DICOMQIPropertyHelper::DeriveDICOMSourceProperties(image, segmentation);

  auto prefs = GetSegmentationPreferences();

  if (prefs != nullptr)
  {
    if (m_TaskList->HasLabelNameSuggestions(current))
    {
      auto path = m_TaskList->GetAbsolutePath(m_TaskList->GetLabelNameSuggestions(current));

      prefs->PutBool("default label naming", false);
      prefs->Put("label suggestions", path.string());
      prefs->PutBool("replace standard suggestions", true);
      prefs->PutBool("suggest once", true);
    }
    else
    {
      prefs->PutBool("default label naming", true);
      prefs->Put("label suggestions", "");
    }
  }

  m_ImageNode = imageNode;
  m_SegmentationNode = segmentationNode;
  m_UnsavedChanges = false;

  this->SetActiveTaskIndex(current);
  this->SubscribeToActiveSegmentation();

  this->OnCurrentTaskChanged();
}

void QmitkSegmentationTaskListWidget::SubscribeToActiveSegmentation()
{
  if (m_ActiveTaskIndex.has_value() && m_SegmentationNode != nullptr)
  {
    auto segmentation = static_cast<mitk::LabelSetImage*>(m_SegmentationNode->GetData());

    auto command = itk::SimpleMemberCommand<QmitkSegmentationTaskListWidget>::New();
    command->SetCallbackFunction(this, &QmitkSegmentationTaskListWidget::OnSegmentationModified);

    m_SegmentationModifiedObserverTag = segmentation->AddObserver(itk::ModifiedEvent(), command);
  }
}

void QmitkSegmentationTaskListWidget::UnsubscribeFromActiveSegmentation()
{
  if (m_ActiveTaskIndex.has_value() && m_SegmentationModifiedObserverTag.has_value())
  {
    if (m_SegmentationNode != nullptr)
    {
      auto segmentation = static_cast<mitk::LabelSetImage*>(m_SegmentationNode->GetData());
      segmentation->RemoveObserver(m_SegmentationModifiedObserverTag.value());
    }

    m_SegmentationModifiedObserverTag.reset();
  }
}

void QmitkSegmentationTaskListWidget::OnSegmentationModified()
{
  if (!m_UnsavedChanges)
  {
    m_UnsavedChanges = true;

    if (m_ActiveTaskIndex.value() == m_CurrentTaskIndex)
      this->UpdateDetailsLabel();
  }
}

void QmitkSegmentationTaskListWidget::SetActiveTaskIndex(const std::optional<size_t>& index)
{
  if (m_ActiveTaskIndex != index)
  {
    m_ActiveTaskIndex = index;
    this->UpdateStoreAndAcceptButtons();
  }
}

void QmitkSegmentationTaskListWidget::SetCurrentTaskIndex(const std::optional<size_t>& index)
{
  if (m_CurrentTaskIndex != index)
  {
    m_CurrentTaskIndex = index;
    this->OnCurrentTaskChanged();
  }
}

bool QmitkSegmentationTaskListWidget::ActiveTaskIsShown() const
{
  return m_ActiveTaskIndex.has_value() && m_CurrentTaskIndex.has_value() && m_ActiveTaskIndex == m_CurrentTaskIndex;
}

bool QmitkSegmentationTaskListWidget::HandleUnsavedChanges(const QString& alternativeTitle)
{
  if (m_UnsavedChanges)
  {
    const auto active = m_ActiveTaskIndex.value();
    const auto current = m_CurrentTaskIndex.value();
    QString title;

    if (alternativeTitle.isEmpty())
    {
      title = QString("Load task %1").arg(current + 1);

      if (m_TaskList->HasName(current))
        title += ": " + QString::fromStdString(m_TaskList->GetName(current));
    }
    else
    {
      title = alternativeTitle;
    }

    auto text = QString("The currently active task %1 ").arg(active + 1);

    if (m_TaskList->HasName(active))
      text += "(" + QString::fromStdString(m_TaskList->GetName(active)) + ") ";

    text += "has unsaved changes.";

    auto reply = QMessageBox::question(this, title, text, QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);

    switch (reply)
    {
    case QMessageBox::Save:
      this->SaveActiveTask(!fs::exists(m_TaskList->GetResult(active)));
      break;

    case QMessageBox::Discard:
      m_UnsavedChanges = false;
      break;

    default:
      return false;
    }
  }

  return true;
}

void QmitkSegmentationTaskListWidget::SaveActiveTask(bool saveAsInterimResult)
{
  if (!m_ActiveTaskIndex.has_value())
    return;

  QApplication::setOverrideCursor(Qt::BusyCursor);

  try
  {
    const auto active = m_ActiveTaskIndex.value();
    m_TaskList->SaveTask(active, m_SegmentationNode->GetData(), saveAsInterimResult);
    this->OnUnsavedChangesSaved();
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << e;
  }

  QApplication::restoreOverrideCursor();
}

bool QmitkSegmentationTaskListWidget::OnPreShutdown()
{
  return this->HandleUnsavedChanges(QStringLiteral("Application shutdown"));
}

void QmitkSegmentationTaskListWidget::OnPreviousTaskShortcutActivated()
{
  m_Ui->previousButton->click();
}

void QmitkSegmentationTaskListWidget::OnNextTaskShortcutActivated()
{
  m_Ui->nextButton->click();
}

void QmitkSegmentationTaskListWidget::OnFindTaskShortcutActivated()
{
  m_Ui->findButton->click();
}

void QmitkSegmentationTaskListWidget::OnLoadTaskShortcutActivated()
{
  m_Ui->loadButton->click();
}

void QmitkSegmentationTaskListWidget::OnStoreInterimResultShortcutActivated()
{
  m_Ui->storeButton->click();
}

void QmitkSegmentationTaskListWidget::OnAcceptSegmentationShortcutActivated()
{
  m_Ui->acceptButton->click();
}

void QmitkSegmentationTaskListWidget::OnStoreButtonClicked()
{
  this->SaveActiveTask(true);
}

void QmitkSegmentationTaskListWidget::OnAcceptButtonClicked()
{
  auto* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  int activeToolId = -1;

  if (toolManager != nullptr)
    activeToolId = toolManager->GetActiveToolID();

  this->SaveActiveTask();
  this->LoadNextUnfinishedTask();

  if (toolManager != nullptr)
    toolManager->ActivateTool(activeToolId);
}
