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

#include <mitkSegGroupOperationApplier.h>

// Qmitk
#include <QmitkCopyLabelToGroupDialog.h>
#include <QmitkMultiLabelTreeModel.h>
#include <QmitkLabelColorItemDelegate.h>
#include <QmitkLabelToggleItemDelegate.h>
#include <QmitkStyleManager.h>

// Qt
#include <QMenu>
#include <QLabel>
#include <QWidgetAction>
#include <QMessageBox>
#include <QKeyEvent>

#include <QInputDialog>

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
  connect(m_Model, &QmitkMultiLabelTreeModel::modelChanged, this, &QmitkMultiLabelInspector::OnModelChanged);
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

  //in single selection mode, if at least one label exist select the first label of the mode.
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

void QmitkMultiLabelInspector::SetMultiLabelSegmentation(mitk::MultiLabelSegmentation* segmentation)
{
  if (segmentation != m_Segmentation)
  {
    m_Segmentation = segmentation;
    this->Initialize();
    emit SegmentationChanged();
  }
}

mitk::MultiLabelSegmentation* QmitkMultiLabelInspector::GetMultiLabelSegmentation() const
{
  return m_Segmentation;
}

void QmitkMultiLabelInspector::SetMultiLabelNode(mitk::DataNode* node)
{
  if (node != this->m_SegmentationNode.GetPointer())
  {
    m_SegmentationObserver.Reset();
    m_SegmentationNode = node;
    m_LabelHighlightGuard.SetSegmentationNode(m_SegmentationNode);
    m_SegmentationNodeDataMTime = 0;

    if (m_SegmentationNode.IsNotNull())
    {
      auto& widget = *this;
      auto checkAndSetSeg = [&widget, node](const itk::EventObject&)
        {
          if (widget.m_SegmentationNodeDataMTime < node->GetDataReferenceChangedTime())
          {
            auto newSeg = dynamic_cast<mitk::MultiLabelSegmentation*>(node->GetData());
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

QmitkMultiLabelInspector::LabelValueVectorType FilterValidLabels(QmitkMultiLabelInspector::LabelValueVectorType labelValues, mitk::MultiLabelSegmentation* segmentation)
{
  mitk::MultiLabelSegmentation::LabelValueVectorType validLabels;
  std::copy_if(labelValues.cbegin(), labelValues.cend(),
    std::back_inserter(validLabels),
    [segmentation](mitk::MultiLabelSegmentation::LabelValueType x) { return segmentation->ExistLabel(x); });

  return validLabels;
}

void QmitkMultiLabelInspector::OnModelChanged()
{
  if (!m_ModelManipulationOngoing)
  { //the model was changed externally and not by this instance
    //ensure that selection state is still valid
    //If this instance is manipulating it, it will take care of direct selection directly

    QmitkMultiLabelInspector::LabelValueVectorType validLabels;

    if (m_CheckSelectionDueToExternalModelChange)
    { //selection model tried an update while label model was still updating.
      //check selection now for validity and use it if appropriated.
      m_CheckSelectionDueToExternalModelChange = false;
      validLabels = FilterValidLabels(GetSelectedLabelsFromSelectionModel(), this->GetMultiLabelSegmentation());
    }

    if (validLabels.empty())
    {
      //empty selections are not allowed by UI interactions, there should always be at least one valid label selected.
      //but selections are e.g. also cleared if the model is updated (e.g. due to addition of labels).
      //In this case all valid elements of m_LastValidSelectedLabels should be used.
      validLabels = FilterValidLabels(m_LastValidSelectedLabels, this->GetMultiLabelSegmentation());
    }

    if (validLabels.empty() && m_Model->GetNearestLabelValueToLastChange() != mitk::MultiLabelSegmentation::UNLABELED_VALUE)
    {
      //we use the nearest existing label as valid value, like we do it in DeletLabelInternal
      validLabels = { m_Model->GetNearestLabelValueToLastChange() };
    }

    if (!validLabels.empty())
    {
      UpdateSelectionModel(validLabels);
    }
    else
    {
      m_LastValidSelectedLabels = {};
      emit CurrentSelectionChanged(GetSelectedLabels());
    }
  }
}

void QmitkMultiLabelInspector::OnDataChanged(const QModelIndex& topLeft, const QModelIndex& /*bottomRight*/,
  const QList<int>& /*roles*/)
{
  if (!m_ModelManipulationOngoing && topLeft.isValid())
    m_Controls->view->expand(topLeft);
}

void QmitkMultiLabelInspector::SetSelectedLabels(const LabelValueVectorType& selectedLabels)
{
  if (mitk::Equal(this->GetSelectedLabels(), selectedLabels))
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

void QmitkMultiLabelInspector::SetSelectedLabel(mitk::MultiLabelSegmentation::LabelValueType selectedLabel)
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
    if (qvariantDataNode.canConvert<mitk::MultiLabelSegmentation::LabelValueType>())
    {
      result.push_back(qvariantDataNode.value<mitk::MultiLabelSegmentation::LabelValueType>());
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
    if(!m_Model->GetModelUpdateOngoing())
    {
      auto internallySelectedLabels = this->GetSelectedLabelsFromSelectionModel();
      auto validLabels = FilterValidLabels(internallySelectedLabels, this->GetMultiLabelSegmentation());

      if (internallySelectedLabels.size() == validLabels.size())
      { //all labels selected by the selection model are valid
        //set the state of this instance accordingly
        m_LastValidSelectedLabels = validLabels;
        emit CurrentSelectionChanged(GetSelectedLabels());
      }
      else if (!validLabels.empty())
      { //only a part of the selection model is valid. Update it accordingly, this will
        //lead to a re-triggering of this function
        UpdateSelectionModel(validLabels);
      }
      else
      {
        //empty selections are not allowed by UI interactions, there should always be at least one valid label selected.
        //but selections are e.g. also cleared if the model is updated (e.g. due to addition of labels).
        //In this case all valid elements of m_LastValidSelectedLabels should be used.
        validLabels = FilterValidLabels(m_LastValidSelectedLabels, this->GetMultiLabelSegmentation());

        if (validLabels.empty() && m_Model->GetNearestLabelValueToLastChange() != mitk::MultiLabelSegmentation::UNLABELED_VALUE)
        {
          //we use the nearest existing label as valid value, like we do it in DeletLabelInternal
          validLabels = { m_Model->GetNearestLabelValueToLastChange() };
        }

        if (!validLabels.empty())
        {
          UpdateSelectionModel(validLabels);
        }
        else
        {
          m_LastValidSelectedLabels = {};
          emit CurrentSelectionChanged(GetSelectedLabels());
        }
      }
    }
    else
    { //the model is changed by an external source (not by this instance; e.g. an undo operation or another instance)
      //need to check the selection after the model change is needed
      m_CheckSelectionDueToExternalModelChange = true;
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

  mitk::SegGroupModifyUndoRedoHelper undoRedoGenerator(m_Segmentation, { groupID },
    false, 0, false, true, true);

  m_ModelManipulationOngoing = true;
  auto newLabel = m_Segmentation->AddLabel(templateLabel, groupID, true);
  m_ModelManipulationOngoing = false;

  undoRedoGenerator.RegisterUndoRedoOperationEvent("Add label instance \"" + mitk::LabelSetImageHelper::CreateDisplayLabelName(m_Segmentation, newLabel) + "\"");

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

mitk::Label* QmitkMultiLabelInspector::AddNewLabelInternal(const mitk::MultiLabelSegmentation::GroupIndexType& containingGroup)
{
  auto newLabel = mitk::LabelSetImageHelper::CreateNewLabel(m_Segmentation);

  bool canceled = false;
  if (!m_DefaultLabelNaming)
    emit LabelRenameRequested(newLabel, false, canceled);

  if (canceled) return nullptr;

  mitk::SegGroupModifyUndoRedoHelper undoRedoGenerator(m_Segmentation, { containingGroup },
    false, 0, false, true, true);

  m_ModelManipulationOngoing = true;
  m_Segmentation->AddLabel(newLabel, containingGroup, false);
  m_ModelManipulationOngoing = false;

  undoRedoGenerator.RegisterUndoRedoOperationEvent("Add label \""+ mitk::LabelSetImageHelper::CreateDisplayLabelName(m_Segmentation, newLabel)+"\"");

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
  mitk::MultiLabelSegmentation::GroupIndexType groupID = nullptr != currentLabel
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

  if (m_Segmentation.IsNull() || labelValues.empty())
  {
    return;
  }

  this->WaitCursorOn();

  mitk::SegGroupModifyUndoRedoHelper::GroupIndexSetType containingGroups;
  for (const auto label : labelValues) containingGroups.insert(m_Segmentation->GetGroupIndexOfLabel(label));
  auto deletedLabelName = mitk::LabelSetImageHelper::CreateDisplayLabelName(m_Segmentation, m_Segmentation->GetLabel(labelValues.front()));
  mitk::SegGroupModifyUndoRedoHelper undoRedoGenerator(m_Segmentation, containingGroups, true);

  m_ModelManipulationOngoing = true;

  m_Segmentation->RemoveLabels(labelValues);

  m_ModelManipulationOngoing = false;

  std::ostringstream stream;
  stream << "Remove labels \"" << deletedLabelName << "\"";
  if (labelValues.size() > 1)
  {
    stream << "and " << labelValues.size() - 1 << "other labels";
  }
  undoRedoGenerator.RegisterUndoRedoOperationEvent(stream.str());

  this->WaitCursorOff();

  auto validSelectedLabels = FilterValidLabels(m_LastValidSelectedLabels, this->GetMultiLabelSegmentation());
  if (validSelectedLabels.empty())
  {
    auto newLabelValue = m_Model->GetNearestLabelValueToLastChange();
    if (newLabelValue != mitk::MultiLabelSegmentation::UNLABELED_VALUE)
    {
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

  mitk::MultiLabelSegmentation::GroupIndexType groupID = 0;
  mitk::Label* newLabel = nullptr;
  m_ModelManipulationOngoing = true;
  try
  {
    this->WaitCursorOn();

    groupID = m_Segmentation->AddGroup();

    mitk::SegGroupInsertUndoRedoHelper undoRedoGenerator(m_Segmentation, { groupID }, false, true);
    undoRedoGenerator.RegisterUndoRedoOperationEvent("Insert new group \"" + std::to_string(groupID) + "\"");

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

void QmitkMultiLabelInspector::RemoveGroupInternal(const mitk::MultiLabelSegmentation::GroupIndexType& groupID)
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of RemoveLabel.";

  if (m_Segmentation.IsNull())
    return;

  if (m_Segmentation->GetNumberOfGroups() < 2)
    return;

  try
  {
    this->WaitCursorOn();
    mitk::SegGroupRemoveUndoRedoHelper undoRedoGenerator(m_Segmentation, { groupID });
    auto deletedGroupName = m_Segmentation->GetGroupName(groupID);

    m_ModelManipulationOngoing = true;
    m_Segmentation->RemoveGroup(groupID);
    m_ModelManipulationOngoing = false;

    undoRedoGenerator.RegisterUndoRedoOperationEvent("Remove group \"" + deletedGroupName + "\"");
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

  auto validSelectedLabels = FilterValidLabels(m_LastValidSelectedLabels, this->GetMultiLabelSegmentation());
  if (validSelectedLabels.empty())
  {
    auto newLabelValue = m_Model->GetNearestLabelValueToLastChange();
    if (newLabelValue != mitk::MultiLabelSegmentation::UNLABELED_VALUE)
    {
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

  if (m_Segmentation->GetNumberOfGroups() < 2)
  {
    QMessageBox::information(this, "Delete group", "Cannot delete last remaining group. A segmentation must contain at least a single group.");
    return;
  }

  const auto* selectedLabel = this->GetFirstSelectedLabelObject();

  if (selectedLabel == nullptr)
    return;

  const auto groupID = m_Segmentation->GetGroupIndexOfLabel(selectedLabel->GetValue());
  auto groupName = QString::fromStdString(mitk::LabelSetImageHelper::CreateDisplayGroupName(m_Segmentation, groupID));

  auto question = QStringLiteral("Do you really want to delete group \"%1\" including all of its labels?").arg(groupName);
  auto answer = QMessageBox::question(this, QString("Delete group \"%1\"").arg(groupName), question, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

  if (answer != QMessageBox::Yes)
    return;

  this->RemoveGroupInternal(groupID);
}

void QmitkMultiLabelInspector::OnDeleteGroup()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of RemoveGroupInternal.";

  if (m_Segmentation.IsNull())
    return;

  auto currentIndex = this->m_Controls->view->currentIndex();
  auto groupIDVariant = currentIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::GroupIDRole);

  if (groupIDVariant.isValid())
  {
    auto groupID = groupIDVariant.value<mitk::MultiLabelSegmentation::GroupIndexType>();
    auto groupName = QString::fromStdString(mitk::LabelSetImageHelper::CreateDisplayGroupName(m_Segmentation, groupID));
    auto question = QStringLiteral("Do you really want to delete group \"%1\" including all of its labels?").arg(groupName);
    auto answer = QMessageBox::question(this, QString("Delete group \"%1\"").arg(groupName), question, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

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

      QAction* renameAction = new QAction("Rename group", this);
      QObject::connect(renameAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnRenameGroup);
      menu->addAction(renameAction);

      if (m_Segmentation->GetNumberOfGroups() > 1)
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

      auto copyLabelToGroupAction = new QAction("&Copy to group...", this);
      QObject::connect(copyLabelToGroupAction, &QAction::triggered, this, &QmitkMultiLabelInspector::OnCopyToGroup);
      menu->addAction(copyLabelToGroupAction);

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

void QmitkMultiLabelInspector::OnCopyToGroup()
{
  auto selectedLabels = this->GetSelectedLabels();

  if (selectedLabels.empty())
    return;

  QmitkCopyLabelToGroupDialog dialog(m_Segmentation, m_Segmentation->GetLabel(selectedLabels.front()));

  if (dialog.exec() == QDialog::Accepted)
    this->SetSelectedLabel(dialog.GetDestinationLabel()->GetValue());
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
      if (label.IsNull())
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
    auto guard = &m_LabelHighlightGuard;

    auto onChangeLambda = [segmentation, relevantLabels](const int value)
      {
        auto opacity = static_cast<float>(value) / 100.0f;
        for (auto label : relevantLabels)
        {
          label->SetOpacity(opacity);
          segmentation->UpdateLookupTable(label->GetValue());
        }
        segmentation->GetLookupTable()->Modified();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      };
    QObject::connect(opacitySlider, &QSlider::valueChanged, this, onChangeLambda);

    auto onPressedLambda = [guard]()
      {
        guard->SetHighlightedLabels({});
      };
    QObject::connect(opacitySlider, &QSlider::sliderPressed, this, onPressedLambda);

    auto onReleasedLambda = [relevantLabelValues, guard]()
      {
        guard->SetHighlightedLabels(relevantLabelValues);
      };
    QObject::connect(opacitySlider, &QSlider::sliderReleased, this, onReleasedLambda);

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

    auto labelValues = this->GetSelectedLabels();
    mitk::SegGroupModifyUndoRedoHelper::GroupIndexSetType containingGroups;
    for (const auto label : labelValues) containingGroups.insert(m_Segmentation->GetGroupIndexOfLabel(label));
    auto clearedLabelName = mitk::LabelSetImageHelper::CreateDisplayLabelName(m_Segmentation, m_Segmentation->GetLabel(labelValues.front()));
    mitk::SegGroupModifyUndoRedoHelper undoRedoGenerator(m_Segmentation, containingGroups, true, 0, true, false, true);

    m_Segmentation->EraseLabels(labelValues);

    std::ostringstream stream;
    stream << "Clear labels \"" << clearedLabelName << "\"";
    if (labelValues.size() > 1)
    {
      stream << "and " << labelValues.size() - 1 << "other labels";
    }
    undoRedoGenerator.RegisterUndoRedoOperationEvent(stream.str());

    this->WaitCursorOff();
    // this is needed as workaround for (T27307). It circumvents the fact that modifications
    // of data (here the segmentation) does not directly trigger the modification of the
    // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
    // listens to the datastorage for modification would not get notified.
    if (m_SegmentationNode.IsNotNull())
    {
      m_SegmentationNode->Modified();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
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

  if (currentLabel.IsNull())
  {
    MITK_WARN << "Ignore operation. Try to delete non-existing label. Invalid ID: " << affectedLabels.front();
    return;
  }

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
    auto labelValues = this->GetSelectedLabels();

    if (labelValues.empty()) return;

    this->WaitCursorOn();

    mitk::SegGroupModifyUndoRedoHelper::GroupIndexSetType containingGroups;
    for (const auto label : labelValues) containingGroups.insert(m_Segmentation->GetGroupIndexOfLabel(label));
    auto deletedLabelName = mitk::LabelSetImageHelper::CreateDisplayLabelName(m_Segmentation, m_Segmentation->GetLabel(labelValues.front()));
    mitk::SegGroupModifyUndoRedoHelper undoRedoGenerator(m_Segmentation, containingGroups, true);

    m_Segmentation->RemoveLabels(this->GetSelectedLabels());

    std::ostringstream stream;
    stream << "Remove labels \"" << deletedLabelName << "\"";
    if (labelValues.size() > 1)
    {
      stream << "and " << labelValues.size() - 1 << "other labels";
    }
    undoRedoGenerator.RegisterUndoRedoOperationEvent(stream.str());

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

    mitk::SegGroupModifyUndoRedoHelper undoRedoGenerator(m_Segmentation, { m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue()) }, true);

    m_Segmentation->MergeLabels(currentLabel->GetValue(), this->GetSelectedLabels());

    undoRedoGenerator.RegisterUndoRedoOperationEvent("Merge into label \"" + mitk::LabelSetImageHelper::CreateDisplayLabelName(m_Segmentation, currentLabel) + "\"");

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
    auto groupID = groupIDVariant.value<mitk::MultiLabelSegmentation::GroupIndexType>();
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
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
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

    mitk::SegGroupModifyUndoRedoHelper undoRedoGenerator(m_Segmentation, { m_Segmentation->GetGroupIndexOfLabel(currentLabel->GetValue()) }, true, 0, true, false, true);

    m_Segmentation->EraseLabel(currentLabel->GetValue());

    undoRedoGenerator.RegisterUndoRedoOperationEvent("Clear label \"" + mitk::LabelSetImageHelper::CreateDisplayLabelName(m_Segmentation, currentLabel) + "\"");

    this->WaitCursorOff();

    // this is needed as workaround for (T27307). It circumvents the fact that modifications
    // of data (here the segmentation) does not directly trigger the modification of the
    // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
    // listens to the datastorage for modification would not get notified.
    if (m_SegmentationNode.IsNotNull())
    {
      m_SegmentationNode->Modified();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void QmitkMultiLabelInspector::OnRenameGroup()
{
  if (!m_AllowLabelModification)
    mitkThrow() << "QmitkMultiLabelInspector is configured incorrectly. Set AllowLabelModification to true to allow the usage of RenameGroup.";

  if (m_Segmentation.IsNull())
    return;

  auto currentIndex = this->m_Controls->view->currentIndex();
  auto groupIDVariant = currentIndex.data(QmitkMultiLabelTreeModel::ItemModelRole::GroupIDRole);

  if (groupIDVariant.isValid())
  {
    auto groupID = groupIDVariant.value<mitk::MultiLabelSegmentation::GroupIndexType>();

    bool dlgOK;
    auto groupName = m_Segmentation->GetGroupName(groupID);
    auto newName = QInputDialog::getText(this, "Change name of the group", "Group name:", QLineEdit::Normal, QString::fromStdString(groupName), &dlgOK);
    if (dlgOK)
    {
      mitk::SegGroupModifyUndoRedoHelper undoRedoGenerator(m_Segmentation, { groupID }, true, 0, true, true);

      m_Segmentation->SetGroupName(groupID, newName.toStdString());
      // this is needed as workaround for (T27307). It circumvents the fact that modifications
      // of data (here the segmentation) does not directly trigger the modification of the
      // owning node (see T27307). Therefore other code (like renderers or model views) that e.g.
      // listens to the datastorage for modification would not get notified.

      undoRedoGenerator.RegisterUndoRedoOperationEvent("Rename group #" + std::to_string(groupID));

      if (m_SegmentationNode.IsNotNull())
      {
        m_SegmentationNode->Modified();
      }

    }
  }
}

void QmitkMultiLabelInspector::OnRenameLabel(bool /*value*/)
{
  auto relevantLabelValues = this->GetCurrentlyAffactedLabelInstances();
  auto currentLabel = this->GetCurrentLabel();

  bool canceled = false;
  emit LabelRenameRequested(currentLabel, true, canceled);

  if (canceled) return;

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
      m_Segmentation->GetLookupTable()->Modified();
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
      if (label.IsNull())
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
      if (label.IsNull())
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
  m_Segmentation->GetLookupTable()->Modified();

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
  this->WaitCursorOff();

  const auto currentLabel = m_Segmentation->GetLabel(labelID);
  if (currentLabel.IsNull())
    return;

  const auto pos = currentLabel->GetCenterOfMassIndex();

  if (pos.GetVnlVector().max_value() > 0.0)
  {
    emit GoToLabel(currentLabel->GetValue(), currentLabel->GetCenterOfMassCoordinates());
  }
}

void QmitkMultiLabelInspector::OnEntered(const QModelIndex& index)
{
  if (m_SegmentationNode.IsNotNull())
  {
    auto labelVariant = index.data(QmitkMultiLabelTreeModel::ItemModelRole::LabelInstanceValueRole);

    auto highlightedValues = m_Model->GetLabelsInSubTree(index);

    m_LabelHighlightGuard.SetHighlightedLabels(highlightedValues);
  }
  m_AboutToShowContextMenu = false;
}

void QmitkMultiLabelInspector::OnMouseLeave()
{
  if (m_SegmentationNode.IsNotNull() && !m_AboutToShowContextMenu)
  {
    m_LabelHighlightGuard.SetHighlightedLabels({});
  }
  else
  {
    m_AboutToShowContextMenu = false;
  }
}

void QmitkMultiLabelInspector::keyPressEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Shift)
  {
    m_LabelHighlightGuard.SetHighlightInvisibleLabels(true);
  }

  QWidget::keyPressEvent(event);
}

void QmitkMultiLabelInspector::keyReleaseEvent(QKeyEvent* event)
{
  if (event->key() == Qt::Key_Shift)
  {
    m_LabelHighlightGuard.SetHighlightInvisibleLabels(false);
  }

  QWidget::keyPressEvent(event);
}
