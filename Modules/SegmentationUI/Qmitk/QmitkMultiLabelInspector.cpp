/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiLabelInspector.h>

// mitk
#include <mitkRenderingManager.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkDICOMSegmentationPropertyHelper.h>
#include <mitkVectorProperty.h>

// Qmitk
#include <QmitkMultiLabelTreeModel.h>
#include <QmitkLabelColorItemDelegate.h>
#include <QmitkLabelToggleItemDelegate.h>
#include <QmitkStyleManager.h>

// Qt
#include <QMenu>
#include <QLabel>
#include <QWidgetAction>
#include <QMessageBox>

#include <ui_QmitkMultiLabelInspectorControls.h>

QmitkMultiLabelInspector::QmitkMultiLabelInspector(QWidget* parent/* = nullptr*/)
  : QWidget(parent), m_Controls(new Ui::QmitkMultiLabelInspector), m_SegmentationNodeDataMTime(0)
{
  m_Controls->setupUi(this);

  m_Model = new QmitkMultiLabelTreeModel(this);

  m_Controls->view->setModel(m_Model);

  m_ColorItemDelegate = new QmitkLabelColorItemDelegate(this);

  auto visibleIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/visible.svg"));
  auto invisibleIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/invisible.svg"));
  m_VisibilityItemDelegate = new QmitkLabelToggleItemDelegate(visibleIcon, invisibleIcon, this);

  auto lockIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/lock.svg"));
  auto unlockIcon = QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/unlock.svg"));
  m_LockItemDelegate = new QmitkLabelToggleItemDelegate(lockIcon, unlockIcon, this);

  auto* view = this->m_Controls->view;
  view->setItemDelegateForColumn(1, m_LockItemDelegate);
  view->setItemDelegateForColumn(2, m_ColorItemDelegate);
  view->setItemDelegateForColumn(3, m_VisibilityItemDelegate);

  auto* header = view->header();
  header->setSectionResizeMode(0,QHeaderView::Stretch);
  header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
  view->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(m_Model, &QAbstractItemModel::modelReset, this, &QmitkMultiLabelInspector::OnModelReset);
  connect(m_Model, &QAbstractItemModel::dataChanged, this, &QmitkMultiLabelInspector::OnDataChanged);
  connect(view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(OnChangeModelSelection(const QItemSelection&, const QItemSelection&)));
  connect(view, &QAbstractItemView::customContextMenuRequested, this, &QmitkMultiLabelInspector::OnContextMenuRequested);
  connect(view, &QAbstractItemView::doubleClicked, this, &QmitkMultiLabelInspector::OnItemDoubleClicked);
  connect(view, &QAbstractItemView::entered, this, &QmitkMultiLabelInspector::OnEntered);
  connect(view, &QmitkMultiLabelTreeView::MouseLeave, this, &QmitkMultiLabelInspector::OnMouseLeave);
}

QmitkMultiLabelInspector::~QmitkMultiLabelInspector()
{
  delete m_Controls;
}

void QmitkMultiLabelInspector::Initialize()
{
  m_LastValidSelectedLabels = {};
  m_ModelManipulationOngoing = false;
  m_Model->SetSegmentation(m_Segmentation);
  m_Controls->view->expandAll();

  m_LastValidSelectedLabels = {};

  //in singel selection mode, if at least one label exist select the first label of the mode.
  if (m_Segmentation.IsNotNull() && !this->GetMultiSelectionMode() && m_Segmentation->GetTotalNumberOfLabels() > 0)
  {
    auto firstIndex = m_Model->FirstLabelInstanceIndex(QModelIndex());
    auto labelVariant = firstIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);

    if (labelVariant.isValid())
    {
      this->SetSelectedLabel(labelVariant.value<LabelValueType>());
      m_Controls->view->selectionModel()->setCurrentIndex(firstIndex, QItemSelectionModel::NoUpdate);
    }
  }
}

void QmitkMultiLabelInspector::SetMultiSelectionMode(bool multiMode)
{
  m_Controls->view->setSelectionMode(multiMode
     ? QAbstractItemView::SelectionMode::MultiSelection
     : QAbstractItemView::SelectionMode::SingleSelection);
}

bool QmitkMultiLabelInspector::GetMultiSelectionMode() const
{
  return QAbstractItemView::SelectionMode::MultiSelection == m_Controls->view->selectionMode();
}

void QmitkMultiLabelInspector::SetAllowVisibilityModification(bool visibilityMod)
{
  m_AllowVisibilityModification = visibilityMod;
  this->m_Model->SetAllowVisibilityModification(visibilityMod);
}

void QmitkMultiLabelInspector::SetAllowLabelModification(bool labelMod)
{
  m_AllowLabelModification = labelMod;
}

bool QmitkMultiLabelInspector::GetAllowVisibilityModification() const
{
  return m_AllowVisibilityModification;
}

void QmitkMultiLabelInspector::SetAllowLockModification(bool lockMod)
{
  m_AllowLockModification = lockMod;
  this->m_Model->SetAllowLockModification(lockMod);
}

bool QmitkMultiLabelInspector::GetAllowLockModification() const
{
  return m_AllowLockModification;
}

bool QmitkMultiLabelInspector::GetAllowLabelModification() const
{
  return m_AllowLabelModification;
}

void QmitkMultiLabelInspector::SetDefaultLabelNaming(bool defaultLabelNaming)
{
  m_DefaultLabelNaming = defaultLabelNaming;
}

void QmitkMultiLabelInspector::SetMultiLabelSegmentation(mitk::LabelSetImage* segmentation)
{
  if (segmentation != m_Segmentation)
  {
    m_Segmentation = segmentation;
    this->Initialize();
    emit SegmentationChanged();
  }
}

mitk::LabelSetImage* QmitkMultiLabelInspector::GetMultiLabelSegmentation() const
{
  return m_Segmentation;
}

void QmitkMultiLabelInspector::SetMultiLabelNode(mitk::DataNode* node)
{
  if (node != this->m_SegmentationNode.GetPointer())
  {
    m_SegmentationObserver.Reset();
    m_SegmentationNode = node;
    m_SegmentationNodeDataMTime = 0;

    if (m_SegmentationNode.IsNotNull())
    {
      auto& widget = *this;
      auto checkAndSetSeg = [&widget, node](const itk::EventObject&)
        {
          if (widget.m_SegmentationNodeDataMTime < node->GetDataReferenceChangedTime())
          {
            auto newSeg = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
            if (nullptr == newSeg) mitkThrow() << "Invalid usage. Node set does not contain a segmentation.";

            widget.m_SegmentationNodeDataMTime = node->GetDataReferenceChangedTime();
            widget.SetMultiLabelSegmentation(newSeg);
          }
        };

      m_SegmentationObserver.Reset(node, itk::ModifiedEvent(), checkAndSetSeg);
      checkAndSetSeg(itk::ModifiedEvent());
    }
    else
    {
      this->SetMultiLabelSegmentation(nullptr);
    }
  }
}

mitk::DataNode* QmitkMultiLabelInspector::GetMultiLabelNode() const
{
  return m_SegmentationNode;
}


bool QmitkMultiLabelInspector::GetModelManipulationOngoing() const
{
  return m_ModelManipulationOngoing;
}

void QmitkMultiLabelInspector::OnModelReset()
{
  m_LastValidSelectedLabels = {};
  m_ModelManipulationOngoing = false;
}

void QmitkMultiLabelInspector::OnDataChanged(const QModelIndex& topLeft, const QModelIndex& /*bottomRight*/,
  const QList<int>& /*roles*/)
{
  if (!m_ModelManipulationOngoing && topLeft.isValid())
    m_Controls->view->expand(topLeft);
}

bool EqualLabelSelections(const QmitkMultiLabelInspector::LabelValueVectorType& selection1, const QmitkMultiLabelInspector::LabelValueVectorType& selection2)
{
  if (selection1.size() == selection2.size())
  {
    // lambda to compare node pointer inside both lists
    return std::is_permutation(selection1.begin(), selection1.end(), selection2.begin());
  }

  return false;
}

void QmitkMultiLabelInspector::SetSelectedLabels(const LabelValueVectorType& selectedLabels)
{
  if (EqualLabelSelections(this->GetSelectedLabels(), selectedLabels))
  {
    return;
  }

  this->UpdateSelectionModel(selectedLabels);
  m_LastValidSelectedLabels = selectedLabels;
}

void QmitkMultiLabelInspector::UpdateSelectionModel(const LabelValueVectorType& selectedLabels)
{
  // create new selection by retrieving the corresponding indexes of the labels
  QItemSelection newCurrentSelection;
  for (const auto& labelID : selectedLabels)
  {
    QModelIndexList matched = m_Model->match(m_Model->index(0, 0), QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole, QVariant(labelID), 1, Qt::MatchRecursive);
    if (!matched.empty())
    {
      newCurrentSelection.select(matched.front(), matched.front());
    }
  }

  m_Controls->view->selectionModel()->select(newCurrentSelection, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Current);
}

void QmitkMultiLabelInspector::SetSelectedLabel(mitk::LabelSetImage::LabelValueType selectedLabel)
{
  this->SetSelectedLabels({ selectedLabel });
}

QmitkMultiLabelInspector::LabelValueVectorType QmitkMultiLabelInspector::GetSelectedLabelsFromSelectionModel() const
{
  LabelValueVectorType result;
  QModelIndexList selectedIndexes = m_Controls->view->selectionModel()->selectedIndexes();
  for (const auto& index : std::as_const(selectedIndexes))
  {
    QVariant qvariantDataNode = m_Model->data(index, QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);
    if (qvariantDataNode.canConvert<mitk::LabelSetImage::LabelValueType>())
    {
      result.push_back(qvariantDataNode.value<mitk::LabelSetImage::LabelValueType>());
    }
  }
  return result;
}

QmitkMultiLabelInspector::LabelValueVectorType QmitkMultiLabelInspector::GetSelectedLabels() const
{
  return m_LastValidSelectedLabels;
}

mitk::Label* QmitkMultiLabelInspector::GetFirstSelectedLabelObject() const
{
  if (m_LastValidSelectedLabels.empty() || m_Segmentation.IsNull())
    return nullptr;

  return m_Segmentation->GetLabel(m_LastValidSelectedLabels.front());
}

void QmitkMultiLabelInspector::OnChangeModelSelection(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
  if (!m_ModelManipulationOngoing)
  {
    auto internalSelection = GetSelectedLabelsFromSelectionModel();
    if (internalSelection.empty())
    {
      //empty selections are not allowed by UI interactions, there should always be at least on label selected.
      //but selections are e.g. also cleared if the model is updated (e.g. due to addition of labels)
      UpdateSelectionModel(m_LastValidSelectedLabels);
    }
    else
    {
      m_LastValidSelectedLabels = internalSelection;
      emit CurrentSelectionChanged(GetSelectedLabels());
    }
  }
}

void QmitkMultiLabelInspector::WaitCursorOn() const
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void QmitkMultiLabelInspector::WaitCursorOff() const
{
  this->RestoreOverrideCursor();
}

void QmitkMultiLabelInspector::RestoreOverrideCursor() const
{
  QApplication::restoreOverrideCursor();
}

mitk::Label* QmitkMultiLabelInspector::GetCurrentLabel() const
{
  auto currentIndex = this->m_Controls->view->currentIndex();
  auto labelVariant = currentIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelDataRole);
  mitk::Label::Pointer currentIndexLabel = nullptr;

  if (labelVariant.isValid())
  {
    auto uncastedLabel = labelVariant.value<void*>();
    currentIndexLabel = static_cast<mitk::Label*>(uncastedLabel);
  }
  return currentIndexLabel;
}

QmitkMultiLabelInspector::IndexLevelType QmitkMultiLabelInspector::GetCurrentLevelType() const
{
  auto currentIndex = this->m_Controls->view->currentIndex();
  auto labelInstanceVariant = currentIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceDataRole);
  auto labelVariant = currentIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelDataRole);

  if (labelInstanceVariant.isValid() )
  {
    return IndexLevelType::LabelInstance;
  }
  else if (labelVariant.isValid())
  {
    return IndexLevelType::LabelClass;
  }

  return IndexLevelType::Group;
}

QmitkMultiLabelInspector::LabelValueVectorType QmitkMultiLabelInspector::GetCurrentlyAffactedLabelInstances() const
{
  auto currentIndex = m_Controls->view->currentIndex();
  return m_Model->GetLabelsInSubTree(currentIndex);
}

QmitkMultiLabelInspector::LabelValueVectorType QmitkMultiLabelInspector::GetLabelInstancesOfSelectedFirstLabel() const
{
  if (m_Segmentation.IsNull())
    return {};

  if (this->GetSelectedLabels().empty())
    return {};

  const auto index = m_Model->indexOfLabel(this->GetSelectedLabels().front());
  return m_Model->GetLabelInstancesOfSameLabelClass(index);
}

mitk::Label* QmitkMultiLabelInspector::AddNewLabelInstanceInternal(mitk::Label* templateLabel)
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of AddNewLabelInstance.";

  if (nullptr == templateLabel)
    mitkThrow() << "QmitkMultiLabelInspector is in an invalid state. AddNewLabelInstanceInternal was called with a non existing label as template";

  auto groupID = m_Segmentation->GetGroupIndexOfLabel(templateLabel->GetValue());
  m_ModelManipulationOngoing = true;
  auto newLabel = m_Segmentation->AddLabel(templateLabel, groupID, true);
  m_ModelManipulationOngoing = false;
  this->SetSelectedLabel(newLabel->GetValue());

  auto index = m_Model->indexOfLabel(newLabel->GetValue());
  if (index.isValid())
  {
    m_Controls->view->expand(index.parent());
  }
  else
  {
    mitkThrow() << "Segmentation or QmitkMultiLabelTreeModel is in an invalid state. Label is not present in the model after adding it to the segmentation. Label value: " << newLabel->GetValue();
  }

  emit ModelUpdated();
  return newLabel;
}

mitk::Label* QmitkMultiLabelInspector::AddNewLabelInstance()
{
  auto currentLabel = this->GetFirstSelectedLabelObject();
  if (nullptr == currentLabel)
    return nullptr;

  auto result = this->AddNewLabelInstanceInternal(currentLabel);

  // this is needed as workaround for (T27307). It circumvents the fact that modifications
  // of data (here the segmentation) does not directly trigger the modification of the
  // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
  // listens to the datastorage for modification would not get notified.
  if (m_SegmentationNode.IsNotNull())
  {
    m_SegmentationNode->Modified();
  }

  return result;
}

mitk::Label* QmitkMultiLabelInspector::AddNewLabelInternal(const mitk::LabelSetImage::GroupIndexType& containingGroup)
{
  auto newLabel = mitk::LabelSetImageHelper::CreateNewLabel(m_Segmentation);

  if (!m_DefaultLabelNaming)
    emit LabelRenameRequested(newLabel, false);

  m_ModelManipulationOngoing = true;
  m_Segmentation->AddLabel(newLabel, containingGroup, false);
  m_ModelManipulationOngoing = false;

  this->SetSelectedLabel(newLabel->GetValue());

  auto index = m_Model->indexOfLabel(newLabel->GetValue());

  if (!index.isValid())
    mitkThrow() << "Segmentation or QmitkMultiLabelTreeModel is in an invalid state. Label is not present in the "
                   "model after adding it to the segmentation. Label value: " << newLabel->GetValue();

  m_Controls->view->expand(index.parent());

  emit ModelUpdated();

  return newLabel;
}

mitk::Label* QmitkMultiLabelInspector::AddNewLabel()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of AddNewLabel.";

  if (m_Segmentation.IsNull())
  {
    return nullptr;
  }

  auto currentLabel = this->GetFirstSelectedLabelObject();
  mitk::LabelSetImage::GroupIndexType groupID = nullptr != currentLabel
    ? m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue())
    : 0;

  auto result = AddNewLabelInternal(groupID);

  // this is needed as workaround for (T27307). It circumvents the fact that modifications
  // of data (here the segmentation) does not directly trigger the modification of the
  // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
  // listens to the datastorage for modification would not get notified.
  if (m_SegmentationNode.IsNotNull())
  {
    m_SegmentationNode->Modified();
  }

  return result;
}

void QmitkMultiLabelInspector::DeleteLabelInstance()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of DeleteLabelInstance.";

  if (m_Segmentation.IsNull())
    return;

  auto label = this->GetFirstSelectedLabelObject();

  if (nullptr == label)
    return;

  auto index = m_Model->indexOfLabel(label->GetValue());
  auto instanceName = index.data(Qt::DisplayRole);

  auto question = "Do you really want to delete label instance \"" + instanceName.toString() + "\"?";
  auto answer = QMessageBox::question(this, QString("Delete label instances"), question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answer == QMessageBox::Yes)
  {
    this->DeleteLabelInternal({ label->GetValue() });

    // this is needed as workaround for (T27307). It circumvents the fact that modifications
    // of data (here the segmentation) does not directly trigger the modification of the
    // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
    // listens to the datastorage for modification would not get notified.
    if (m_SegmentationNode.IsNotNull())
    {
      m_SegmentationNode->Modified();
    }
  }
}

void QmitkMultiLabelInspector::DeleteLabel()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of DeleteLabel.";

  if (m_Segmentation.IsNull())
    return;

  const auto label = this->GetFirstSelectedLabelObject();

  if (nullptr == label)
    return;

  const auto relevantLabels = this->GetLabelInstancesOfSelectedFirstLabel();

  if (relevantLabels.empty())
    return;

  auto question = "Do you really want to delete label \"" + QString::fromStdString(label->GetName());
  question = relevantLabels.size()==1 ? question + "\"?" : question + "\" with all "+QString::number(relevantLabels.size()) +" instances?";

  auto answer = QMessageBox::question(this, QString("Delete label"), question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answer == QMessageBox::Yes)
  {
    this->DeleteLabelInternal(relevantLabels);

    // this is needed as workaround for (T27307). It circumvents the fact that modifications
    // of data (here the segmentation) does not directly trigger the modification of the
    // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
    // listens to the datastorage for modification would not get notified.
    if (m_SegmentationNode.IsNotNull())
    {
      m_SegmentationNode->Modified();
    }
  }
}

void QmitkMultiLabelInspector::DeleteLabelInternal(const LabelValueVectorType& labelValues)
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of DeleteLabelInternal.";

  if (m_Segmentation.IsNull())
  {
    return;
  }

  QVariant nextLabelVariant;

  this->WaitCursorOn();
  m_ModelManipulationOngoing = true;
  for (auto labelValue : labelValues)
  {
    if (labelValue == labelValues.back())
    {
      auto currentIndex = m_Model->indexOfLabel(labelValue);
      auto nextIndex = m_Model->ClosestLabelInstanceIndex(currentIndex);
      nextLabelVariant = nextIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);
    }

    m_Segmentation->RemoveLabel(labelValue);
  }
  m_ModelManipulationOngoing = false;
  this->WaitCursorOff();

  if (nextLabelVariant.isValid())
  {
    auto newLabelValue = nextLabelVariant.value<LabelValueType>();
    this->SetSelectedLabel(newLabelValue);

    auto index = m_Model->indexOfLabel(newLabelValue); //we have to get index again, because it could have changed due to remove operation.
    if (index.isValid())
    {
      m_Controls->view->expand(index.parent());
    }
    else
    {
      mitkThrow() << "Segmentation or QmitkMultiLabelTreeModel is in an invalid state. Label is not present in the model after adding it to the segmentation. Label value: " << newLabelValue;
    }
  }
  else
  {
    this->SetSelectedLabels({});
  }

  emit ModelUpdated();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

mitk::Label* QmitkMultiLabelInspector::AddNewGroup()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of AddNewLabel.";

  if (m_Segmentation.IsNull())
  {
    return nullptr;
  }

  mitk::LabelSetImage::GroupIndexType groupID = 0;
  mitk::Label* newLabel = nullptr;
  m_ModelManipulationOngoing = true;
  try
  {
    this->WaitCursorOn();
    groupID = m_Segmentation->AddLayer();
    m_Segmentation->SetActiveLayer(groupID);
    this->WaitCursorOff();
    newLabel =  this->AddNewLabelInternal(groupID);
  }
  catch (mitk::Exception& e)
  {
    this->WaitCursorOff();
    m_ModelManipulationOngoing = false;
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Add group", "Could not add a new group. See error log for details.");
  }
  m_ModelManipulationOngoing = false;

  emit ModelUpdated();

  // this is needed as workaround for (T27307). It circumvents the fact that modifications
  // of data (here the segmentation) does not directly trigger the modification of the
  // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
  // listens to the datastorage for modification would not get notified.
  if (m_SegmentationNode.IsNotNull())
  {
    m_SegmentationNode->Modified();
  }

  return newLabel;
}

void QmitkMultiLabelInspector::RemoveGroupInternal(const mitk::LabelSetImage::GroupIndexType& groupID)
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of RemoveLabel.";

  if (m_Segmentation.IsNull())
    return;

  if (m_Segmentation->GetNumberOfLayers() < 2)
    return;

  auto currentIndex = m_Model->indexOfGroup(groupID);
  auto nextIndex = m_Model->ClosestLabelInstanceIndex(currentIndex);
  auto labelVariant = nextIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);

  try
  {
    this->WaitCursorOn();
    m_ModelManipulationOngoing = true;
    m_Segmentation->RemoveGroup(groupID);
    m_ModelManipulationOngoing = false;
    this->WaitCursorOff();
  }
  catch (mitk::Exception& e)
  {
    m_ModelManipulationOngoing = false;
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Delete group", "Could not delete the currently active group. See error log for details.");
    return;
  }

  if (labelVariant.isValid())
  {
    auto newLabelValue = labelVariant.value<LabelValueType>();
    this->SetSelectedLabel(newLabelValue);

    auto index = m_Model->indexOfLabel(newLabelValue); //we have to get index again, because it could have changed due to remove operation.
    if (index.isValid())
    {
      m_Controls->view->expand(index.parent());
    }
    else
    {
      mitkThrow() << "Segmentation or QmitkMultiLabelTreeModel is in an invalid state. Label is not present in the model after adding it to the segmentation. Label value: " << newLabelValue;
    }
  }
  else
  {
    this->SetSelectedLabels({});
  }

  emit ModelUpdated();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMultiLabelInspector::RemoveGroup()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of RemoveLabel.";

  if (m_Segmentation.IsNull())
    return;

  if (m_Segmentation->GetNumberOfLayers() < 2)
  {
    QMessageBox::information(this, "Delete group", "Cannot delete last remaining group. A segmentation must contain at least a single group.");
    return;
  }

  const auto* selectedLabel = this->GetFirstSelectedLabelObject();

  if (selectedLabel == nullptr)
    return;

  const auto group = m_Segmentation->GetGroupIndexOfLabel(selectedLabel->GetValue());

  auto question = QStringLiteral("Do you really want to delete group %1 including all of its labels?").arg(group);
  auto answer = QMessageBox::question(this, QStringLiteral("Delete group %1").arg(group), question, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

  if (answer != QMessageBox::Yes)
    return;

  this->RemoveGroupInternal(group);
}

void QmitkMultiLabelInspector::OnDeleteGroup()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of RemoveLabel.";

  if (m_Segmentation.IsNull())
    return;

  auto currentIndex = this->m_Controls->view->currentIndex();
  auto groupIDVariant = currentIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::GroupIDRole);

  if (groupIDVariant.isValid())
  {
    auto groupID = groupIDVariant.value<mitk::LabelSetImage::GroupIndexType>();

    auto question = QStringLiteral("Do you really want to delete group %1 including all of its labels?").arg(groupID);
    auto answer = QMessageBox::question(this, QString("Delete group %1").arg(groupID), question, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    if (answer != QMessageBox::Yes)
      return;

    this->RemoveGroupInternal(groupID);

    // this is needed as workaround for (T27307). It circumvents the fact that modifications
    // of data (here the segmentation) does not directly trigger the modification of the
    // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
    // listens to the datastorage for modification would not get notified.
    if (m_SegmentationNode.IsNotNull())
    {
      m_SegmentationNode->Modified();
    }
  }
};


void QmitkMultiLabelInspector::OnContextMenuRequested(const QPoint& /*pos*/)
{
  if (m_Segmentation.IsNull() || !this->isEnabled())
    return;

  const auto indexLevel = this->GetCurrentLevelType();

  auto currentIndex = this->m_Controls->view->currentIndex();
  //this ensures correct highlighting is the context menu is triggered while
  //another context menu is already open.
  if (currentIndex.isValid() && this->m_AboutToShowContextMenu) this->OnEntered(this->m_Controls->view->currentIndex());


  QMenu* menu = new QMenu(this);

  if (IndexLevelType::Group == indexLevel)
  {
    if (m_AllowLabelModification)
    {
      QAction* addInstanceAction = new QAction(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_label_add.svg")), "&Add label", this);
      QObject::connect(addInstanceAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnAddLabel);
      menu->addAction(addInstanceAction);

      if (m_Segmentation->GetNumberOfLayers() > 1)
      {
        QAction* removeAction = new QAction(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_group_delete.svg")), "Delete group", this);
        QObject::connect(removeAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnDeleteGroup);
        menu->addAction(removeAction);
      }
    }

    if (m_AllowLockModification)
    {
      menu->addSeparator();
      QAction* lockAllAction = new QAction(QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/lock.svg")), "Lock group", this);
      QObject::connect(lockAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnLockAffectedLabels);
      menu->addAction(lockAllAction);

      QAction* unlockAllAction = new QAction(QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/unlock.svg")), "Unlock group", this);
      QObject::connect(unlockAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnUnlockAffectedLabels);
      menu->addAction(unlockAllAction);
    }

    if (m_AllowVisibilityModification)
    {
      menu->addSeparator();

      QAction* viewAllAction = new QAction(QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/visible.svg")), "Show group", this);
      QObject::connect(viewAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnSetAffectedLabelsVisible);
      menu->addAction(viewAllAction);

      QAction* hideAllAction = new QAction(QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/invisible.svg")), "Hide group", this);
      QObject::connect(hideAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnSetAffectedLabelsInvisible);
      menu->addAction(hideAllAction);

      menu->addSeparator();

      auto opacityAction = this->CreateOpacityAction();
      if (nullptr != opacityAction)
        menu->addAction(opacityAction);
    }
  }
  else if (IndexLevelType::LabelClass == indexLevel)
  {
    if (m_AllowLabelModification)
    {
      QAction* addInstanceAction = new QAction(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_label_add_instance.svg")), "Add label instance", this);
      QObject::connect(addInstanceAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnAddLabelInstance);
      menu->addAction(addInstanceAction);

      QAction* renameAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "&Rename label", this);
      QObject::connect(renameAction, SIGNAL(triggered(bool)), this, SLOT(OnRenameLabel(bool)));
      menu->addAction(renameAction);

      QAction* removeAction = new QAction(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_label_delete.svg")), "&Delete label", this);
      QObject::connect(removeAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnDeleteAffectedLabel);
      menu->addAction(removeAction);
    }

    if (m_AllowLockModification)
    {
      menu->addSeparator();
      QAction* lockAllAction = new QAction(QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/lock.svg")), "Lock label instances", this);
      QObject::connect(lockAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnLockAffectedLabels);
      menu->addAction(lockAllAction);

      QAction* unlockAllAction = new QAction(QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/unlock.svg")), "Unlock label instances", this);
      QObject::connect(unlockAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnUnlockAffectedLabels);
      menu->addAction(unlockAllAction);
    }

    if (m_AllowVisibilityModification)
    {
      menu->addSeparator();

      QAction* viewAllAction = new QAction(QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/visible.svg")), "Show label instances", this);
      QObject::connect(viewAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnSetAffectedLabelsVisible);
      menu->addAction(viewAllAction);

      QAction* hideAllAction = new QAction(QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/invisible.svg")), "Hide label instances", this);
      QObject::connect(hideAllAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnSetAffectedLabelsInvisible);
      menu->addAction(hideAllAction);

      menu->addSeparator();

      auto opacityAction = this->CreateOpacityAction();
      if (nullptr!=opacityAction)
        menu->addAction(opacityAction);
    }
  }
  else
  {
    auto selectedLabelValues = this->GetSelectedLabels();
    if (selectedLabelValues.empty())
      return;

    if (this->GetMultiSelectionMode() && selectedLabelValues.size() > 1)
    {
      if (m_AllowLabelModification)
      {
        QAction* mergeAction = new QAction(QIcon(":/Qmitk/MergeLabels.png"), "Merge selection on current label", this);
        QObject::connect(mergeAction, SIGNAL(triggered(bool)), this, SLOT(OnMergeLabels(bool)));
        menu->addAction(mergeAction);

        QAction* removeLabelsAction = new QAction(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_label_delete_instance.svg")), "&Delete selected labels", this);
        QObject::connect(removeLabelsAction, SIGNAL(triggered(bool)), this, SLOT(OnDeleteLabels(bool)));
        menu->addAction(removeLabelsAction);

        QAction* clearLabelsAction = new QAction(QIcon(":/Qmitk/EraseLabel.png"), "&Clear selected labels", this);
        QObject::connect(clearLabelsAction, SIGNAL(triggered(bool)), this, SLOT(OnClearLabels(bool)));
        menu->addAction(clearLabelsAction);
      }

      if (m_AllowVisibilityModification)
      {
        if (m_AllowLabelModification) menu->addSeparator();

        QAction* viewOnlyAction = new QAction(QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/visible.svg")), "Hide everything but this", this);
        QObject::connect(viewOnlyAction, SIGNAL(triggered(bool)), this, SLOT(OnSetOnlyActiveLabelVisible(bool)));
        menu->addAction(viewOnlyAction);

        menu->addSeparator();

        auto opacityAction = this->CreateOpacityAction();
        if (nullptr != opacityAction)
          menu->addAction(opacityAction);
      }
    }
    else
    {
      if (m_AllowLabelModification)
      {
        QAction* addInstanceAction = new QAction(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_label_add_instance.svg")), "&Add label instance", this);
        QObject::connect(addInstanceAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnAddLabelInstance);
        menu->addAction(addInstanceAction);

        const auto selectedLabelIndex = m_Model->indexOfLabel(selectedLabelValues.front());

        if (m_Model->GetLabelInstancesOfSameLabelClass(selectedLabelIndex).size() > 1) // Only labels that actually appear as instance (having additional instances)
        {
          QAction* renameAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "&Rename label instance", this);
          QObject::connect(renameAction, SIGNAL(triggered(bool)), this, SLOT(OnRenameLabel(bool)));
          menu->addAction(renameAction);

          QAction* removeInstanceAction = new QAction(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_label_delete_instance.svg")), "&Delete label instance", this);
          QObject::connect(removeInstanceAction, &QAction::triggered, this, &QmitkMultiLabelInspector::DeleteLabelInstance);
          menu->addAction(removeInstanceAction);
        }
        else
        {
          QAction* renameAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "&Rename label", this);
          QObject::connect(renameAction, SIGNAL(triggered(bool)), this, SLOT(OnRenameLabel(bool)));
          menu->addAction(renameAction);
        }

        QAction* removeLabelAction = new QAction(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_label_delete.svg")), "Delete &label", this);
        QObject::connect(removeLabelAction, &QAction::triggered, this, &QmitkMultiLabelInspector::DeleteLabel);
        menu->addAction(removeLabelAction);

        QAction* clearAction = new QAction(QIcon(":/Qmitk/EraseLabel.png"), "&Clear content", this);
        QObject::connect(clearAction, SIGNAL(triggered(bool)), this, SLOT(OnClearLabel(bool)));
        menu->addAction(clearAction);
      }

      if (m_AllowVisibilityModification)
      {
        if (m_AllowLabelModification) menu->addSeparator();

        QAction* viewOnlyAction = new QAction(QmitkStyleManager::ThemeIcon(QLatin1String(":/Qmitk/visible.svg")), "Hide everything but this", this);
        QObject::connect(viewOnlyAction, SIGNAL(triggered(bool)), this, SLOT(OnSetOnlyActiveLabelVisible(bool)));
        menu->addAction(viewOnlyAction);

        menu->addSeparator();

        auto opacityAction = this->CreateOpacityAction();
        if (nullptr != opacityAction)
          menu->addAction(opacityAction);
      }
    }
  }

  QObject::connect(menu, &QMenu::aboutToHide, this, &QmitkMultiLabelInspector::OnMouseLeave);
  m_AboutToShowContextMenu = true;
  menu->popup(QCursor::pos());
}

QWidgetAction* QmitkMultiLabelInspector::CreateOpacityAction()
{
  auto selectedLabelValues = this->GetSelectedLabels();

  auto relevantLabelValues = !this->GetMultiSelectionMode() || selectedLabelValues.size() <= 1
    ? this->GetCurrentlyAffactedLabelInstances()
    : selectedLabelValues;

  std::vector<mitk::Label*> relevantLabels;

  if (!relevantLabelValues.empty())
  {
    for (auto value : relevantLabelValues)
    {
      auto label = this->m_Segmentation->GetLabel(value);
      if (nullptr == label)
        mitkThrow() << "Invalid state. Internal model returned a label value that does not exist in segmentation. Invalid value:" << value;
      relevantLabels.emplace_back(label);
    }

    auto* opacitySlider = new QSlider;
    opacitySlider->setMinimum(0);
    opacitySlider->setMaximum(100);
    opacitySlider->setOrientation(Qt::Horizontal);

    auto opacity = relevantLabels.front()->GetOpacity();
    opacitySlider->setValue(static_cast<int>(opacity * 100));
    auto segmentation = m_Segmentation;

    QObject::connect(opacitySlider, &QSlider::valueChanged, this, [segmentation, relevantLabels](const int value)
    {
      auto opacity = static_cast<float>(value) / 100.0f;
      for (auto label : relevantLabels)
      {
        label->SetOpacity(opacity);
        segmentation->UpdateLookupTable(label->GetValue());
      }
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    );

    QLabel* opacityLabel = new QLabel("Opacity: ");
    QVBoxLayout* opacityWidgetLayout = new QVBoxLayout;
    opacityWidgetLayout->setContentsMargins(4, 4, 4, 4);
    opacityWidgetLayout->addWidget(opacityLabel);
    opacityWidgetLayout->addWidget(opacitySlider);
    QWidget* opacityWidget = new QWidget;
    opacityWidget->setLayout(opacityWidgetLayout);
    QWidgetAction* opacityAction = new QWidgetAction(this);
    opacityAction->setDefaultWidget(opacityWidget);

    return opacityAction;
  }

  return nullptr;
}

void QmitkMultiLabelInspector::OnClearLabels(bool /*value*/)
{
  QString question = "Do you really want to clear the selected labels?";

  QMessageBox::StandardButton answerButton = QMessageBox::question(
    this, "Clear selected labels", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->WaitCursorOn();
    m_Segmentation->EraseLabels(this->GetSelectedLabels());
    this->WaitCursorOff();
    // this is needed as workaround for (T27307). It circumvents the fact that modifications
    // of data (here the segmentation) does not directly trigger the modification of the
    // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
    // listens to the datastorage for modification would not get notified.
    if (m_SegmentationNode.IsNotNull())
    {
      m_SegmentationNode->Modified();
    }
  }
}

void QmitkMultiLabelInspector::OnDeleteAffectedLabel()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of RemoveLabel.";

  if (m_Segmentation.IsNull())
  {
    return;
  }

  auto affectedLabels = GetCurrentlyAffactedLabelInstances();
  auto currentLabel = m_Segmentation->GetLabel(affectedLabels.front());
  QString question = "Do you really want to delete all instances of label \"" + QString::fromStdString(currentLabel->GetName()) + "\"?";

  QMessageBox::StandardButton answerButton =
    QMessageBox::question(this, "Delete label", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->DeleteLabelInternal(affectedLabels);

    // this is needed as workaround for (T27307). It circumvents the fact that modifications
    // of data (here the segmentation) does not directly trigger the modification of the
    // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
    // listens to the datastorage for modification would not get notified.
    if (m_SegmentationNode.IsNotNull())
    {
      m_SegmentationNode->Modified();
    }
  }
}

void QmitkMultiLabelInspector::OnDeleteLabels(bool /*value*/)
{
  QString question = "Do you really want to remove the selected labels?";
  QMessageBox::StandardButton answerButton = QMessageBox::question(
    this, "Remove selected labels", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->WaitCursorOn();
    m_Segmentation->RemoveLabels(this->GetSelectedLabels());
    this->WaitCursorOff();

    // this is needed as workaround for (T27307). It circumvents the fact that modifications
    // of data (here the segmentation) does not directly trigger the modification of the
    // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
    // listens to the datastorage for modification would not get notified.
    if (m_SegmentationNode.IsNotNull())
    {
      m_SegmentationNode->Modified();
    }
  }
}

void QmitkMultiLabelInspector::OnMergeLabels(bool /*value*/)
{
  auto currentLabel = GetCurrentLabel();
  QString question = "Do you really want to merge selected labels into \"" + QString::fromStdString(currentLabel->GetName())+"\"?";

  QMessageBox::StandardButton answerButton = QMessageBox::question(
    this, "Merge selected label", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->WaitCursorOn();
    m_Segmentation->MergeLabels(currentLabel->GetValue(), this->GetSelectedLabels());
    this->WaitCursorOff();

    // this is needed as workaround for (T27307). It circumvents the fact that modifications
    // of data (here the segmentation) does not directly trigger the modification of the
    // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
    // listens to the datastorage for modification would not get notified.
    if (m_SegmentationNode.IsNotNull())
    {
      m_SegmentationNode->Modified();
    }
  }
}

void QmitkMultiLabelInspector::OnAddLabel()
{
  auto currentIndex = this->m_Controls->view->currentIndex();
  auto groupIDVariant = currentIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::GroupIDRole);

  if (groupIDVariant.isValid())
  {
    auto groupID = groupIDVariant.value<mitk::LabelSetImage::GroupIndexType>();
    this->AddNewLabelInternal(groupID);

    // this is needed as workaround for (T27307). It circumvents the fact that modifications
    // of data (here the segmentation) does not directly trigger the modification of the
    // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
    // listens to the datastorage for modification would not get notified.
    if (m_SegmentationNode.IsNotNull())
    {
      m_SegmentationNode->Modified();
    }
  }
}

void QmitkMultiLabelInspector::OnAddLabelInstance()
{
  auto currentLabel = this->GetCurrentLabel();
  if (nullptr == currentLabel)
    return;

  this->AddNewLabelInstanceInternal(currentLabel);

  // this is needed as workaround for (T27307). It circumvents the fact that modifications
  // of data (here the segmentation) does not directly trigger the modification of the
  // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
  // listens to the datastorage for modification would not get notified.
  if (m_SegmentationNode.IsNotNull())
  {
    m_SegmentationNode->Modified();
  }
}

void QmitkMultiLabelInspector::OnClearLabel(bool /*value*/)
{
  auto currentLabel = GetFirstSelectedLabelObject();
  QString question = "Do you really want to clear the contents of label \"" + QString::fromStdString(currentLabel->GetName())+"\"?";

  QMessageBox::StandardButton answerButton =
    QMessageBox::question(this, "Clear label", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->WaitCursorOn();
    m_Segmentation->EraseLabel(currentLabel->GetValue());
    this->WaitCursorOff();

    // this is needed as workaround for (T27307). It circumvents the fact that modifications
    // of data (here the segmentation) does not directly trigger the modification of the
    // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
    // listens to the datastorage for modification would not get notified.
    if (m_SegmentationNode.IsNotNull())
    {
      m_SegmentationNode->Modified();
    }
  }
}

void QmitkMultiLabelInspector::OnRenameLabel(bool /*value*/)
{
  auto relevantLabelValues = this->GetCurrentlyAffactedLabelInstances();
  auto currentLabel = this->GetCurrentLabel();

  emit LabelRenameRequested(currentLabel, true);

  for (auto value : relevantLabelValues)
  {
    if (value != currentLabel->GetValue())
    {
      auto label = this->m_Segmentation->GetLabel(value);
      if (nullptr == label)
        mitkThrow() << "Invalid state. Internal model returned a label value that does not exist in segmentation. Invalid value:" << value;

      label->SetName(currentLabel->GetName());
      label->SetColor(currentLabel->GetColor());
      m_Segmentation->UpdateLookupTable(label->GetValue());
      mitk::DICOMSegmentationPropertyHelper::SetDICOMSegmentProperties(label);

      // this is needed as workaround for (T27307). It circumvents the fact that modifications
      // of data (here the segmentation) does not directly trigger the modification of the
      // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
      // listens to the datastorage for modification would not get notified.
      if (m_SegmentationNode.IsNotNull())
      {
        m_SegmentationNode->Modified();
      }
    }
  }
  emit ModelUpdated();
}


void QmitkMultiLabelInspector::SetLockOfAffectedLabels(bool locked) const
{
  auto relevantLabelValues = this->GetCurrentlyAffactedLabelInstances();

  if (!relevantLabelValues.empty())
  {
    for (auto value : relevantLabelValues)
    {
      auto label = this->m_Segmentation->GetLabel(value);
      if (nullptr == label)
        mitkThrow() << "Invalid state. Internal model returned a label value that does not exist in segmentation. Invalid value:" << value;
      label->SetLocked(locked);
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkMultiLabelInspector::OnUnlockAffectedLabels()
{
  this->SetLockOfAffectedLabels(false);
}

void QmitkMultiLabelInspector::OnLockAffectedLabels()
{
  this->SetLockOfAffectedLabels(true);
}

void QmitkMultiLabelInspector::SetVisibilityOfAffectedLabels(bool visible) const
{
  auto relevantLabelValues = this->GetCurrentlyAffactedLabelInstances();

  if (!relevantLabelValues.empty())
  {
    for (auto value : relevantLabelValues)
    {
      auto label = this->m_Segmentation->GetLabel(value);
      if (nullptr == label)
        mitkThrow() << "Invalid state. Internal model returned a label value that does not exist in segmentation. Invalid value:" << value;
      label->SetVisible(visible);
      m_Segmentation->UpdateLookupTable(label->GetValue());
    }
    m_Segmentation->GetLookupTable()->Modified();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkMultiLabelInspector::OnSetAffectedLabelsVisible()
{
  this->SetVisibilityOfAffectedLabels(true);
}

void QmitkMultiLabelInspector::OnSetAffectedLabelsInvisible()
{
  this->SetVisibilityOfAffectedLabels(false);
}

void QmitkMultiLabelInspector::OnSetOnlyActiveLabelVisible(bool /*value*/)
{
  auto selectedLabelValues = this->GetSelectedLabels();

  if (selectedLabelValues.empty()) return;

  m_Segmentation->SetAllLabelsVisible(false);

  for (auto selectedValue : selectedLabelValues)
  {
    auto currentLabel = m_Segmentation->GetLabel(selectedValue);
    currentLabel->SetVisible(true);
    m_Segmentation->UpdateLookupTable(selectedValue);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  this->PrepareGoToLabel(selectedLabelValues.front());
}

void QmitkMultiLabelInspector::OnItemDoubleClicked(const QModelIndex& index)
{
  if (!index.isValid()) return;
  if (index.column() > 0) return;

  auto labelVariant = index.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);

  if (!labelVariant.isValid()) return;

  const auto labelID = labelVariant.value<mitk::Label::PixelType>();

  if (QApplication::queryKeyboardModifiers().testFlag(Qt::AltModifier))
  {
    this->OnRenameLabel(false);
    return;
  }

  this->PrepareGoToLabel(labelID);
}

void QmitkMultiLabelInspector::PrepareGoToLabel(mitk::Label::PixelType labelID) const
{
  this->WaitCursorOn();
  m_Segmentation->UpdateCenterOfMass(labelID);
  const auto currentLabel = m_Segmentation->GetLabel(labelID);
  const mitk::Point3D& pos = currentLabel->GetCenterOfMassCoordinates();
  this->WaitCursorOff();

  if (pos.GetVnlVector().max_value() > 0.0)
  {
    emit GoToLabel(currentLabel->GetValue(), pos);
  }
}

void QmitkMultiLabelInspector::OnEntered(const QModelIndex& index)
{
  if (m_SegmentationNode.IsNotNull())
  {
    auto labelVariant = index.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);

    auto highlightedValues = m_Model->GetLabelsInSubTree(index);

    std::string propertyName = "org.mitk.multilabel.labels.highlighted";

    mitk::IntVectorProperty::Pointer prop = dynamic_cast<mitk::IntVectorProperty*>(m_SegmentationNode->GetNonConstProperty(propertyName));
    if (nullptr == prop)
    {
      prop = mitk::IntVectorProperty::New();
      m_SegmentationNode->SetProperty("org.mitk.multilabel.labels.highlighted", prop);
    }

    mitk::IntVectorProperty::VectorType intValues(highlightedValues.begin(), highlightedValues.end());
    prop->SetValue(intValues);
    prop->Modified(); //see T30386; needed because VectorProperty::SetValue does currently trigger no modified

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  m_AboutToShowContextMenu = false;
}

void QmitkMultiLabelInspector::OnMouseLeave()
{
  if (m_SegmentationNode.IsNotNull() && !m_AboutToShowContextMenu)
  {
    std::string propertyName = "org.mitk.multilabel.labels.highlighted";

    mitk::IntVectorProperty::Pointer prop = dynamic_cast<mitk::IntVectorProperty*>(m_SegmentationNode->GetNonConstProperty(propertyName));
    if (nullptr != prop)
    {
      prop->SetValue({});
      prop->Modified(); //see T30386; needed because VectorProperty::SetValue does currently trigger no modified

      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
  else
  {
    m_AboutToShowContextMenu = false;
  }
}
