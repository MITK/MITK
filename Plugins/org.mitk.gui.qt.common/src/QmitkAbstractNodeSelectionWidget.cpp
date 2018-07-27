/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "QmitkAbstractNodeSelectionWidget.h"

QmitkAbstractNodeSelectionWidget::QmitkAbstractNodeSelectionWidget(QWidget* parent) : QWidget(parent), m_InvalidInfo("<b><font color=\"red\">Error. Select data.</font></b>"),
m_EmptyInfo("Empty. Make a selection."), m_PopUpTitel("Select a data node"), m_PopUpHint(""),
m_IsOptional(false), m_SelectOnlyVisibleNodes(true)
{
}

void QmitkAbstractNodeSelectionWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    m_DataStorage = dataStorage;
    this->OnDataStorageChanged();
    this->UpdateInfo();
  }
};

void QmitkAbstractNodeSelectionWidget::SetNodePredicate(mitk::NodePredicateBase* nodePredicate)
{
  if (m_NodePredicate != nodePredicate)
  {
    m_NodePredicate = nodePredicate;

    this->OnNodePredicateChanged(nodePredicate);
    this->UpdateInfo();
  }
};

mitk::NodePredicateBase* QmitkAbstractNodeSelectionWidget::GetNodePredicate() const
{
  return m_NodePredicate;
}

QString QmitkAbstractNodeSelectionWidget::GetInvalidInfo() const
{
  return m_InvalidInfo;
};

QString QmitkAbstractNodeSelectionWidget::GetEmptyInfo() const
{
  return m_EmptyInfo;
};

QString QmitkAbstractNodeSelectionWidget::GetPopUpTitel() const
{
  return m_PopUpTitel;
};

QString QmitkAbstractNodeSelectionWidget::GetPopUpHint() const
{
  return m_PopUpHint;
};

bool QmitkAbstractNodeSelectionWidget::GetSelectionIsOptional() const
{
  return m_IsOptional;
};

bool QmitkAbstractNodeSelectionWidget::GetSelectOnlyVisibleNodes() const
{
  return m_SelectOnlyVisibleNodes;
};

void QmitkAbstractNodeSelectionWidget::SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes)
{
  m_SelectOnlyVisibleNodes = selectOnlyVisibleNodes;
};

void QmitkAbstractNodeSelectionWidget::SetInvalidInfo(QString info)
{
  m_InvalidInfo = info;
  this->UpdateInfo();
};

void QmitkAbstractNodeSelectionWidget::SetEmptyInfo(QString info)
{
  m_EmptyInfo = info;
  this->UpdateInfo();
};

void QmitkAbstractNodeSelectionWidget::SetPopUpTitel(QString info)
{
  m_PopUpTitel = info;
};

void QmitkAbstractNodeSelectionWidget::SetPopUpHint(QString info)
{
  m_PopUpHint = info;
};

void QmitkAbstractNodeSelectionWidget::SetSelectionIsOptional(bool isOptional)
{
  m_IsOptional = isOptional;
  this->UpdateInfo();
};
