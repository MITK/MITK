/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkAbstractNodeSelectionWidget.h"

QmitkAbstractNodeSelectionWidget::QmitkAbstractNodeSelectionWidget(QWidget* parent) : QWidget(parent), m_InvalidInfo("Error. Select data."),
m_EmptyInfo("Empty. Make a selection."), m_PopUpTitel("Select a data node"), m_PopUpHint(""),
m_IsOptional(false), m_SelectOnlyVisibleNodes(true)
{
}

QmitkAbstractNodeSelectionWidget::~QmitkAbstractNodeSelectionWidget()
{
  if (!m_DataStorage.IsExpired())
  {
    auto dataStorage = m_DataStorage.Lock();

    // remove Listener for the data storage itself
    dataStorage->RemoveObserver(m_DataStorageDeletedTag);

    // remove "add node listener" from data storage
    dataStorage->AddNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractNodeSelectionWidget, const mitk::DataNode*>(this, &QmitkAbstractNodeSelectionWidget::NodeAddedToStorage));

    // remove "remove node listener" from data storage
    dataStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractNodeSelectionWidget, const mitk::DataNode*>(this, &QmitkAbstractNodeSelectionWidget::NodeRemovedFromStorage));
  }
}


void QmitkAbstractNodeSelectionWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage == dataStorage)
  {
    return;
  }

  if (!m_DataStorage.IsExpired())
  {
    auto oldStorage = m_DataStorage.Lock();

    // remove Listener for the data storage itself
    oldStorage->RemoveObserver(m_DataStorageDeletedTag);

    // remove "add node listener" from old data storage
    oldStorage->AddNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractNodeSelectionWidget, const mitk::DataNode*>(this, &QmitkAbstractNodeSelectionWidget::NodeAddedToStorage));

    // remove "remove node listener" from old data storage
    oldStorage->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<QmitkAbstractNodeSelectionWidget, const mitk::DataNode*>(this, &QmitkAbstractNodeSelectionWidget::NodeRemovedFromStorage));
  }

  m_DataStorage = dataStorage;

  if (!m_DataStorage.IsExpired())
  {
    auto newStorage = m_DataStorage.Lock();

    // add Listener for the data storage itself
    auto command = itk::SimpleMemberCommand<QmitkAbstractNodeSelectionWidget>::New();
    command->SetCallbackFunction(this, &QmitkAbstractNodeSelectionWidget::SetDataStorageDeleted);
    m_DataStorageDeletedTag = newStorage->AddObserver(itk::DeleteEvent(), command);

    // add "add node listener" for new data storage
    newStorage->AddNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkAbstractNodeSelectionWidget, const mitk::DataNode*>(this, &QmitkAbstractNodeSelectionWidget::NodeAddedToStorage));

    // add remove node listener for new data storage
    newStorage->RemoveNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkAbstractNodeSelectionWidget, const mitk::DataNode*>(this, &QmitkAbstractNodeSelectionWidget::NodeRemovedFromStorage));
  }

  // update model if the data storage has been changed
  m_DataStorage = dataStorage;
  this->OnDataStorageChanged();
  this->UpdateInfo();
}

void QmitkAbstractNodeSelectionWidget::SetNodePredicate(const mitk::NodePredicateBase* nodePredicate)
{
  if (m_NodePredicate != nodePredicate)
  {
    m_NodePredicate = nodePredicate;

    this->OnNodePredicateChanged(nodePredicate);
    this->UpdateInfo();
  }
}

const mitk::NodePredicateBase* QmitkAbstractNodeSelectionWidget::GetNodePredicate() const
{
  return m_NodePredicate;
}

QString QmitkAbstractNodeSelectionWidget::GetInvalidInfo() const
{
  return m_InvalidInfo;
}

QString QmitkAbstractNodeSelectionWidget::GetEmptyInfo() const
{
  return m_EmptyInfo;
}

QString QmitkAbstractNodeSelectionWidget::GetPopUpTitel() const
{
  return m_PopUpTitel;
}

QString QmitkAbstractNodeSelectionWidget::GetPopUpHint() const
{
  return m_PopUpHint;
}

bool QmitkAbstractNodeSelectionWidget::GetSelectionIsOptional() const
{
  return m_IsOptional;
}

bool QmitkAbstractNodeSelectionWidget::GetSelectOnlyVisibleNodes() const
{
  return m_SelectOnlyVisibleNodes;
}

void QmitkAbstractNodeSelectionWidget::SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes)
{
  m_SelectOnlyVisibleNodes = selectOnlyVisibleNodes;
}

void QmitkAbstractNodeSelectionWidget::SetInvalidInfo(QString info)
{
  m_InvalidInfo = info;
  this->UpdateInfo();
}

void QmitkAbstractNodeSelectionWidget::SetEmptyInfo(QString info)
{
  m_EmptyInfo = info;
  this->UpdateInfo();
}

void QmitkAbstractNodeSelectionWidget::SetPopUpTitel(QString info)
{
  m_PopUpTitel = info;
}

void QmitkAbstractNodeSelectionWidget::SetPopUpHint(QString info)
{
  m_PopUpHint = info;
}

void QmitkAbstractNodeSelectionWidget::SetSelectionIsOptional(bool isOptional)
{
  m_IsOptional = isOptional;
  this->UpdateInfo();
}

void QmitkAbstractNodeSelectionWidget::SetDataStorageDeleted()
{
  this->SetDataStorage(nullptr);
}

void QmitkAbstractNodeSelectionWidget::NodeAddedToStorage(const mitk::DataNode* node)
{
  //default implementation does nothing;
}