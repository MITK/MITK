/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSegmentationTaskWidget.h"
#include "org_mitk_gui_qt_flow_segmentation_Activator.h"

#include <mitkIDataStorageService.h>
#include <mitkIOUtil.h>
#include <mitkLabelSetIOHelper.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateFunction.h>
#include <mitkRenderingManager.h>
#include <mitkSegmentationHelper.h>

#include <QmitkStaticDynamicSegmentationDialog.h>
#include <QmitkStyleManager.h>

#include <ui_QmitkSegmentationTaskWidget.h>

#include <QFileSystemWatcher>
#include <QMessageBox>

#include <filesystem>

namespace
{
  mitk::DataStorage* GetDataStorage()
  {
    auto* pluginContext = org_mitk_gui_qt_flow_segmentation_Activator::GetContext();
    auto dataStorageServiceReference = pluginContext->getServiceReference<mitk::IDataStorageService>();

    if (dataStorageServiceReference)
    {
      auto* dataStorageService = pluginContext->getService<mitk::IDataStorageService>(dataStorageServiceReference);

      if (dataStorageService != nullptr)
      {
        auto dataStorageReference = dataStorageService->GetDataStorage();
        pluginContext->ungetService(dataStorageServiceReference);

        return dataStorageReference->GetDataStorage();
      }
    }

    return nullptr;
  }

  std::filesystem::path GetInputLocation(const mitk::BaseData* data)
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
}

/* This constructor has three objectives:
 *   1. Do widget initialization that cannot be done in the .ui file
 *   2. Connect signals and slots
 *   3. Explicitly trigger a reset to a valid initial widget state
 */
QmitkSegmentationTaskWidget::QmitkSegmentationTaskWidget(QWidget* parent)
  : QWidget(parent),
    m_Ui(new Ui::QmitkSegmentationTaskWidget),
    m_FileSystemWatcher(new QFileSystemWatcher(this)),
    m_CurrentSubtaskIndex(0),
    m_UnsavedChanges(false)
{
  m_Ui->setupUi(this);

  m_Ui->selectionWidget->SetDataStorage(GetDataStorage());
  m_Ui->selectionWidget->SetSelectionIsOptional(true);
  m_Ui->selectionWidget->SetEmptyInfo(QStringLiteral("Select a segmentation task"));
  m_Ui->selectionWidget->SetAutoSelectNewNodes(true);
  m_Ui->selectionWidget->SetNodePredicate(mitk::TNodePredicateDataType<mitk::SegmentationTask>::New());

  m_Ui->progressBar->setStyleSheet(QString("QProgressBar::chunk { background-color: %1; }").arg(QmitkStyleManager::GetIconAccentColor()));

  using Self = QmitkSegmentationTaskWidget;

  connect(m_Ui->selectionWidget, &QmitkSingleNodeSelectionWidget::CurrentSelectionChanged, this, &Self::OnSelectionChanged);
  connect(m_Ui->previousButton, &QToolButton::clicked, this, &Self::OnPreviousButtonClicked);
  connect(m_Ui->nextButton, &QToolButton::clicked, this, &Self::OnNextButtonClicked);
  connect(m_Ui->loadButton, &QPushButton::clicked, this, &Self::OnLoadButtonClicked);

  connect(m_FileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &Self::OnResultDirectoryChanged);

  this->OnSelectionChanged(m_Ui->selectionWidget->GetSelectedNodes());
}

QmitkSegmentationTaskWidget::~QmitkSegmentationTaskWidget()
{
}

mitk::SegmentationTask* QmitkSegmentationTaskWidget::GetTask() const
{
  return m_Task;
}

std::optional<size_t> QmitkSegmentationTaskWidget::GetActiveSubtaskIndex() const
{
  return m_ActiveSubtaskIndex;
}

size_t QmitkSegmentationTaskWidget::GetCurrentSubtaskIndex() const
{
  return m_CurrentSubtaskIndex;
}

void QmitkSegmentationTaskWidget::OnUnsavedChangesSaved()
{
  if (m_UnsavedChanges)
  {
    m_UnsavedChanges = false;

    if (m_ActiveSubtaskIndex.value() == m_CurrentSubtaskIndex)
      this->UpdateDetailsLabel();
  }
}

/* Make sure that the widget transitions into a valid state whenever the
 * selection changes.
 */
void QmitkSegmentationTaskWidget::OnSelectionChanged(const QmitkSingleNodeSelectionWidget::NodeList& nodes)
{
  this->UnloadSubtasks();
  this->ResetControls();

  if (!nodes.empty())
  {
    m_TaskNode = nodes.front();
    auto task = dynamic_cast<mitk::SegmentationTask*>(m_TaskNode->GetData());

    if (task != nullptr)
    {
      this->OnTaskChanged(task);
      return;
    }
  }

  this->SetTask(nullptr);
  m_TaskNode = nullptr;
}

/* Reset all controls to a default state as a common basis for further
 * adjustments.
 */
void QmitkSegmentationTaskWidget::ResetControls()
{
  m_Ui->progressBar->setEnabled(false);
  m_Ui->progressBar->setFormat("");
  m_Ui->progressBar->setValue(0);
  m_Ui->progressBar->setMaximum(1);

  m_Ui->previousButton->setEnabled(false);

  m_Ui->loadButton->setEnabled(false);
  m_Ui->loadButton->setText(QStringLiteral("Load subtask"));

  m_Ui->nextButton->setEnabled(false);

  m_Ui->detailsLabel->clear();
}

/* If the segmentation task changed, reset all member variables to expected
 * default values and reset the file system watcher.
 */
void QmitkSegmentationTaskWidget::SetTask(mitk::SegmentationTask* task)
{
  if (m_Task != task)
  {
    m_Task = task;

    this->SetCurrentSubtaskIndex(0);
    this->ResetFileSystemWatcher();
  }
}

void QmitkSegmentationTaskWidget::ResetFileSystemWatcher()
{
  auto paths = m_FileSystemWatcher->directories();

  if (!paths.empty())
    m_FileSystemWatcher->removePaths(paths);

  if (m_Task.IsNotNull())
  {
    for (const auto& subtask : *m_Task)
    {
      auto resultPath = m_Task->GetAbsolutePath(subtask.GetResult()).remove_filename();

      if (!std::filesystem::exists(resultPath))
      {
        try
        {
          std::filesystem::create_directories(resultPath);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
          MITK_ERROR << e.what();
        }
      }

      if (std::filesystem::exists(resultPath))
        m_FileSystemWatcher->addPath(QString::fromStdString(resultPath.string()));
    }
  }
}

void QmitkSegmentationTaskWidget::OnResultDirectoryChanged(const QString&)
{
  // TODO: If a segmentation was modified ("Unsaved changes"), saved ("Done"), and then the file is deleted, the status should be "Unsaved changes" instead of "Not done".
  this->UpdateProgressBar();
  this->UpdateDetailsLabel();
}

void QmitkSegmentationTaskWidget::UpdateProgressBar()
{
  int progress = 0;

  for (size_t i = 0; i < m_Task->GetNumberOfSubtasks(); ++i)
  {
    if (m_Task->IsDone(i))
      ++progress;
  }

  m_Ui->progressBar->setValue(progress);
}

/* Provided that a valid segmentation task is currently selected and the
 * widget is in its default state, update all controls accordingly.
 */
void QmitkSegmentationTaskWidget::OnTaskChanged(mitk::SegmentationTask* task)
{
  this->SetTask(task);

  const auto numSubtasks = task->GetNumberOfSubtasks();

  m_Ui->progressBar->setMaximum(numSubtasks);
  m_Ui->progressBar->setFormat(QStringLiteral("%v/%m Subtask(s) done"));
  m_Ui->progressBar->setEnabled(true);

  this->UpdateProgressBar();

  m_Ui->loadButton->setEnabled(true);

  if (numSubtasks > 1)
    m_Ui->nextButton->setEnabled(true);

  this->OnSubtaskChanged();
}

/* If possible, change the currently displayed subtask to the previous subtask.
 * Enable/disable navigation buttons according to the subtask's position.
 */
void QmitkSegmentationTaskWidget::OnPreviousButtonClicked()
{
  const auto maxIndex = m_Task->GetNumberOfSubtasks() - 1;

  if (m_CurrentSubtaskIndex != 0)
  {
    this->SetCurrentSubtaskIndex(m_CurrentSubtaskIndex - 1);
    this->OnSubtaskChanged();
  }

  if (m_CurrentSubtaskIndex == 0)
    m_Ui->previousButton->setEnabled(false);

  if (m_CurrentSubtaskIndex < maxIndex)
    m_Ui->nextButton->setEnabled(true);
}

/* If possible, change the currently displayed subtask to the next subtask.
 * Enable/disable navigation buttons according to the subtask's position.
 */
void QmitkSegmentationTaskWidget::OnNextButtonClicked()
{
  const auto maxIndex = m_Task->GetNumberOfSubtasks() - 1;

  if (m_CurrentSubtaskIndex < maxIndex)
  {
    this->SetCurrentSubtaskIndex(m_CurrentSubtaskIndex + 1);
    this->OnSubtaskChanged();
  }

  if (m_CurrentSubtaskIndex != 0)
    m_Ui->previousButton->setEnabled(true);

  if (m_CurrentSubtaskIndex >= maxIndex)
    m_Ui->nextButton->setEnabled(false);
}

/* Update affected controls when the currently displayed subtask changed.
 */
void QmitkSegmentationTaskWidget::OnSubtaskChanged()
{
  this->UpdateLoadButton();
  this->UpdateDetailsLabel();
}

/* Update the load button according to the currently displayed subtask.
 */
void QmitkSegmentationTaskWidget::UpdateLoadButton()
{
  const auto i = m_CurrentSubtaskIndex;

  auto text = !this->ActivateSubtaskIsShown()
    ? QStringLiteral("Load subtask")
    : QStringLiteral("Subtask");

  if (m_Task.IsNotNull())
  {
    text += QString(" %1/%2").arg(i + 1).arg(m_Task->GetNumberOfSubtasks());

    if (m_Task->HasName(i))
      text += QStringLiteral(":\n") + QString::fromStdString(m_Task->GetName(i));
  }

  m_Ui->loadButton->setDisabled(this->ActivateSubtaskIsShown());
  m_Ui->loadButton->setText(text);
}

/* Update the details label according to the currently display subtask.
 * The text is composed of the status of the subtask and a variable number
 * of text blocks according to the optional values provided by the subtask.
 */
void QmitkSegmentationTaskWidget::UpdateDetailsLabel()
{
  const auto i = m_CurrentSubtaskIndex;
  bool isDone = m_Task->IsDone(i);

  auto details = QString("<p><b>Status: %1</b> / <b>").arg(this->ActivateSubtaskIsShown()
    ? ColorString("Active", Qt::white, QColor(Qt::green).darker())
    : ColorString("Inactive", Qt::white, QColor(Qt::red).darker()));

  if (m_UnsavedChanges && this->ActivateSubtaskIsShown())
  {
    details += QString("%1</b></p>").arg(ColorString("Unsaved changes", Qt::white, QColor(Qt::red).darker()));
  }
  else
  {
    details += QString("%1</b></p>").arg(isDone
      ? ColorString("Done", Qt::white, QColor(Qt::green).darker())
      : ColorString("Not done", Qt::white, QColor(Qt::red).darker()));
  }

  if (m_Task->HasDescription(i))
    details += QString("<p><b>Description:</b> %1</p>").arg(QString::fromStdString(m_Task->GetDescription(i)));

  QStringList stringList;

  if (m_Task->HasImage(i))
    stringList << QString("<b>Image:</b> %1").arg(QString::fromStdString(m_Task->GetImage(i)));

  if (m_Task->HasSegmentation(i))
    stringList << QString("<b>Segmentation:</b> %1").arg(QString::fromStdString(m_Task->GetSegmentation(i)));

  if (m_Task->HasLabelName(i))
    stringList << QString("<b>Label name:</b> %1").arg(QString::fromStdString(m_Task->GetLabelName(i)));

  if (m_Task->HasPreset(i))
    stringList << QString("<b>Label set preset:</b> %1").arg(QString::fromStdString(m_Task->GetPreset(i)));

  if (m_Task->HasDynamic(i))
    stringList << QString("<b>Segmentation type:</b> %1").arg(m_Task->GetDynamic(i) ? "Dynamic" : "Static");

  if (!stringList.empty())
    details += QString("<p>%1</p>").arg(stringList.join(QStringLiteral("<br>")));

  m_Ui->detailsLabel->setText(details);
}

/* Load/activate the currently displayed subtask. Unload all data nodes from
 * previously active subtasks first, but spare and reuse the image if possible.
 */
void QmitkSegmentationTaskWidget::OnLoadButtonClicked()
{
  if (m_UnsavedChanges)
  {
    const auto i = m_ActiveSubtaskIndex.value();

    const auto title = QString("Load subtask %1").arg(m_CurrentSubtaskIndex + 1);
    const auto text = QString("The currently active subtask %1 has unsaved changes.").arg(i + 1);
    const auto reply = QMessageBox::question(this, title, text, QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel);

    if (reply == QMessageBox::Cancel)
      return;

    if (reply == QMessageBox::Save)
    {
      QApplication::setOverrideCursor(Qt::BusyCursor);

      try
      {
        m_Task->SaveSubtask(i, this->GetSegmentationDataNode(i)->GetData());
        this->OnUnsavedChangesSaved();
      }
      catch (const mitk::Exception& e)
      {
        MITK_ERROR << e;
      }

      QApplication::restoreOverrideCursor();
    }
  }

  m_Ui->loadButton->setEnabled(false);
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  auto* imageNode = this->GetImageDataNode(m_CurrentSubtaskIndex);

  this->UnloadSubtasks(imageNode);
  this->LoadSubtask(imageNode);

  this->OnSubtaskChanged();
  QApplication::restoreOverrideCursor();

  m_UnsavedChanges = false;
}

/* If present, return the image data node for the subtask with the specified
 * index. Otherwise, return nullptr.
 */
mitk::DataNode* QmitkSegmentationTaskWidget::GetImageDataNode(size_t index) const
{
  const auto imagePath = m_Task->GetAbsolutePath(m_Task->GetImage(index));

  auto imageNodes = GetDataStorage()->GetDerivations(m_TaskNode, mitk::NodePredicateFunction::New([imagePath](const mitk::DataNode* node) {
    return imagePath == GetInputLocation(node->GetData());
  }));

  return !imageNodes->empty()
    ? imageNodes->front()
    : nullptr;
}

/* If present, return the segmentation data node for the subtask with the
 * specified index. Otherwise, return nullptr.
 */
mitk::DataNode* QmitkSegmentationTaskWidget::GetSegmentationDataNode(size_t index) const
{
  const auto* imageNode = this->GetImageDataNode(index);

  if (imageNode != nullptr)
  {
    auto segmentations = GetDataStorage()->GetDerivations(imageNode, mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());

    if (!segmentations->empty())
      return segmentations->front();
  }

  return nullptr;
}

/* Unload all subtask data nodes but spare the passed image data node.
 */
void QmitkSegmentationTaskWidget::UnloadSubtasks(const mitk::DataNode* skip)
{
  this->UnsubscribeFromActiveSegmentation();

  if (m_TaskNode.IsNotNull())
  {
    mitk::DataStorage::Pointer dataStorage = GetDataStorage();

    auto imageNodes = dataStorage->GetDerivations(m_TaskNode, mitk::TNodePredicateDataType<mitk::Image>::New());

    for (auto imageNode : *imageNodes)
    {
      dataStorage->Remove(dataStorage->GetDerivations(imageNode, nullptr, false));

      if (imageNode != skip)
        dataStorage->Remove(imageNode);
    }
  }

  this->SetActiveSubtaskIndex(std::nullopt);
}

/* Load/activate the currently displayed subtask. The subtask must specify
 * an image. The segmentation is either created from scratch with an optional
 * name for the first label, possibly based on a label set preset specified by
 * the subtask, or loaded as specified by the subtask. If a result file does
 * exist, it is chosen as segmentation instead.
 */
void QmitkSegmentationTaskWidget::LoadSubtask(mitk::DataNode::Pointer imageNode)
{
  const auto i = m_CurrentSubtaskIndex;

  mitk::Image::Pointer image;
  mitk::LabelSetImage::Pointer segmentation;

  try
  {
    if (imageNode.IsNull())
    {
      const auto path = m_Task->GetAbsolutePath(m_Task->GetImage(i));
      image = mitk::IOUtil::Load<mitk::Image>(path.string());
    }

    const auto path = m_Task->GetAbsolutePath(m_Task->GetResult(i));

    if (std::filesystem::exists(path))
    {
      segmentation = mitk::IOUtil::Load<mitk::LabelSetImage>(path.string());
    }
    else if (m_Task->HasSegmentation(i))
    {
      const auto path = m_Task->GetAbsolutePath(m_Task->GetSegmentation(i));
      segmentation = mitk::IOUtil::Load<mitk::LabelSetImage>(path.string());
    }
  }
  catch (const mitk::Exception&)
  {
    return;
  }

  auto dataStorage = GetDataStorage();

  if (imageNode.IsNull())
  {
    imageNode = mitk::DataNode::New();
    imageNode->SetData(image);

    dataStorage->Add(imageNode, m_TaskNode);

    mitk::RenderingManager::GetInstance()->InitializeViews(image->GetTimeGeometry());
  }
  else
  {
    image = static_cast<mitk::Image*>(imageNode->GetData());
  }

  auto name = "Subtask " + std::to_string(i + 1);
  imageNode->SetName(name);

  if (segmentation.IsNull())
  {
    mitk::Image::ConstPointer templateImage = image;

    if (templateImage->GetDimension() > 3)
    {
      if (m_Task->HasDynamic(i))
      {
        if (!m_Task->GetDynamic(i))
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

    auto segmentationNode = mitk::LabelSetImageHelper::CreateNewSegmentationNode(imageNode, templateImage, name);
    segmentation = static_cast<mitk::LabelSetImage*>(segmentationNode->GetData());

    if (m_Task->HasPreset(i))
    {
      const auto path = m_Task->GetAbsolutePath(m_Task->GetPreset(i));
      mitk::LabelSetIOHelper::LoadLabelSetImagePreset(path.string(), segmentation);
    }
    else
    {
      auto label = mitk::LabelSetImageHelper::CreateNewLabel(segmentation);

      if (m_Task->HasLabelName(i))
        label->SetName(m_Task->GetLabelName(i));

      segmentation->GetActiveLabelSet()->AddLabel(label);
    }

    dataStorage->Add(segmentationNode, imageNode);
  }
  else
  {
    auto segmentationNode = mitk::DataNode::New();
    segmentationNode->SetName(name);
    segmentationNode->SetData(segmentation);

    dataStorage->Add(segmentationNode, imageNode);
  }

  m_UnsavedChanges = false;

  this->SetActiveSubtaskIndex(i);
  this->SubscribeToActiveSegmentation();
}

void QmitkSegmentationTaskWidget::SubscribeToActiveSegmentation()
{
  if (m_ActiveSubtaskIndex.has_value())
  {
    auto segmentationNode = this->GetSegmentationDataNode(m_ActiveSubtaskIndex.value());

    if (segmentationNode != nullptr)
    {
      auto segmentation = static_cast<mitk::LabelSetImage*>(segmentationNode->GetData());

      auto command = itk::SimpleMemberCommand<QmitkSegmentationTaskWidget>::New();
      command->SetCallbackFunction(this, &QmitkSegmentationTaskWidget::OnSegmentationModified);

      m_SegmentationModifiedObserverTag = segmentation->AddObserver(itk::ModifiedEvent(), command);
    }
  }
}

void QmitkSegmentationTaskWidget::UnsubscribeFromActiveSegmentation()
{
  if (m_ActiveSubtaskIndex.has_value() && m_SegmentationModifiedObserverTag.has_value())
  {
    auto segmentationNode = this->GetSegmentationDataNode(m_ActiveSubtaskIndex.value());

    if (segmentationNode != nullptr)
    {
      auto segmentation = static_cast<mitk::LabelSetImage*>(segmentationNode->GetData());
      segmentation->RemoveObserver(m_SegmentationModifiedObserverTag.value());
    }

    m_SegmentationModifiedObserverTag.reset();
  }
}

void QmitkSegmentationTaskWidget::OnSegmentationModified()
{
  if (!m_UnsavedChanges)
  {
    m_UnsavedChanges = true;

    if (m_ActiveSubtaskIndex.value() == m_CurrentSubtaskIndex)
      this->UpdateDetailsLabel();
  }
}

void QmitkSegmentationTaskWidget::SetActiveSubtaskIndex(const std::optional<size_t>& index)
{
  if (m_ActiveSubtaskIndex != index)
  {
    m_ActiveSubtaskIndex = index;
    emit ActiveSubtaskChanged(m_ActiveSubtaskIndex);
  }
}

void QmitkSegmentationTaskWidget::SetCurrentSubtaskIndex(size_t index)
{
  if (m_CurrentSubtaskIndex != index)
  {
    m_CurrentSubtaskIndex = index;
    emit CurrentSubtaskChanged(m_CurrentSubtaskIndex);
  }
}

bool QmitkSegmentationTaskWidget::ActivateSubtaskIsShown() const
{
  return m_ActiveSubtaskIndex.has_value() && m_ActiveSubtaskIndex == m_CurrentSubtaskIndex;
}
