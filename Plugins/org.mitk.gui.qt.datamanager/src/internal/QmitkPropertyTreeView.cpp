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

#include "QmitkPropertyTreeView.h"
#include "QmitkPropertyTreeModel.h"
#include "QmitkLineEdit.h"
#include "QmitkPropertyTreeFilterProxyModel.h"
#include <QmitkPropertyDelegate.h>
#include <QVBoxLayout>
#include <QTreeView>
#include <sstream>

const std::string QmitkPropertyTreeView::VIEW_ID = "org.mitk.views.propertytreeview";

QmitkPropertyTreeView::QmitkPropertyTreeView()
  : m_Filter(NULL),
    m_Model(NULL),
    m_ProxyModel(NULL),
    m_Delegate(NULL),
    m_TreeView(NULL)
{
}

QmitkPropertyTreeView::~QmitkPropertyTreeView()
{
  if (m_Delegate != NULL)
    delete m_Delegate;

  if (m_ProxyModel != NULL)
    delete m_ProxyModel;

  if (m_Model != NULL)
    delete m_Model;
}

void QmitkPropertyTreeView::CreateQtPartControl(QWidget *parent)
{
  m_Filter = new QmitkLineEdit("Filter", parent);

  m_Model = new QmitkPropertyTreeModel;

  m_ProxyModel = new QmitkPropertyTreeFilterProxyModel;
  m_ProxyModel->setSourceModel(m_Model);
  m_ProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_ProxyModel->setDynamicSortFilter(true);

  m_Delegate = new QmitkPropertyDelegate;

  connect(m_Filter, SIGNAL(textChanged(const QString &)), this, SLOT(OnFilterChanged(const QString &)));

  m_TreeView = new QTreeView(parent);

  m_TreeView->setModel(m_ProxyModel);
  m_TreeView->setItemDelegateForColumn(1, m_Delegate);
  m_TreeView->setSortingEnabled(true);
  m_TreeView->setIndentation(16);
  m_TreeView->setAlternatingRowColors(true);
  m_TreeView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_TreeView->setSelectionBehavior(QAbstractItemView::SelectItems);

  QVBoxLayout *layout = new QVBoxLayout(parent);

  layout->addWidget(m_Filter);
  layout->addWidget(m_TreeView);
}

void QmitkPropertyTreeView::OnFilterChanged(const QString &filter)
{
  m_ProxyModel->setFilterWildcard(filter);
  m_TreeView->expandAll();
}

void QmitkPropertyTreeView::OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer> &nodes)
{
  std::string partName = "Properties";

  if (nodes.empty() || nodes.front().IsNull())
  {
    SetPartName(partName);
    m_Model->SetProperties(NULL);
  }
  else
  {
    std::ostringstream extPartName;
    extPartName << partName << " (" << nodes.front()->GetName() << ")";
    SetPartName(extPartName.str().c_str());

    m_Model->SetProperties(nodes.front()->GetPropertyList());
  }
}

void QmitkPropertyTreeView::SetFocus()
{
  m_Filter->setFocus();
}
