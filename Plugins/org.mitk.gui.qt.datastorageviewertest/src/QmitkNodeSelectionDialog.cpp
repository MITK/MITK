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


#include "QmitkNodeSelectionDialog.h"

#include <QmitkDataStorageListViewWidget.h>

QmitkNodeSelectionDialog::QmitkNodeSelectionDialog(QWidget* parent) : QDialog(parent), m_NodePredicate(nullptr), m_SelectOnlyVisibleNodes(false)
{
  m_Controls.setupUi(this);
  AddPanel(new QmitkDataStorageListViewWidget(this),"Test");
}

void QmitkNodeSelectionDialog::SetDataStorage(mitk::DataStorage* dataStorage)
{
  if (m_DataStorage != dataStorage)
  {
    m_DataStorage = dataStorage;

    if (!m_DataStorage.IsExpired())
    {
      for (auto panel : m_Panels)
      {
        panel->SetDataStorage(dataStorage);
      }
    }
  }
};

void QmitkNodeSelectionDialog::SetNodePredicate(mitk::NodePredicateBase* nodePredicate)
{
  if (m_NodePredicate != nodePredicate)
  {
    m_NodePredicate = nodePredicate;

    for (auto panel : m_Panels)
    {
      panel->SetNodePredicate(m_NodePredicate);
    }
  }
};

mitk::NodePredicateBase* QmitkNodeSelectionDialog::GetNodePredicate() const
{
  return m_NodePredicate;
}

QmitkNodeSelectionDialog::NodeList QmitkNodeSelectionDialog::GetSelectedNodes() const
{
  return m_SelectedNodes;
};

void QmitkNodeSelectionDialog::SetSelectOnlyVisibleNodes(bool selectOnlyVisibleNodes)
{
  if (m_SelectOnlyVisibleNodes != selectOnlyVisibleNodes)
  {
    m_SelectOnlyVisibleNodes = selectOnlyVisibleNodes;

    for (auto panel : m_Panels)
    {
      panel->SetSelectOnlyVisibleNodes(m_SelectOnlyVisibleNodes);
    }
  }
};

void QmitkNodeSelectionDialog::SetCurrentSelection(NodeList selectedNodes)
{
  for (auto panel : m_Panels)
  {
    panel->SetCurrentSelection(selectedNodes);
  }
};

void QmitkNodeSelectionDialog::OnSelectionChanged(NodeList selectedNodes)
{
  SetCurrentSelection(selectedNodes);
  emit CurrentSelectionChanged(selectedNodes);
};

void QmitkNodeSelectionDialog::AddPanel(QmitkAbstractDataStorageViewWidget* view, QString name)
{
  view->setParent(this);

  auto tabPanel = new QWidget();
  tabPanel->setObjectName(QString("tab_")+name);
  m_Controls.tabWidget->addTab(tabPanel, name);

  auto verticalLayout = new QVBoxLayout(tabPanel);
  verticalLayout->setSpacing(0);
  verticalLayout->setContentsMargins(0, 0, 0, 0);
  verticalLayout->addWidget(view);

  m_Panels.push_back(view);
  connect(view, &QmitkAbstractDataStorageViewWidget::CurrentSelectionChanged, this, &QmitkNodeSelectionDialog::OnSelectionChanged);
};

