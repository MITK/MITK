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

// semantic relations UI module
#include "QmitkSimpleDatamanagerWidget.h"

#include "QmitkCustomVariants.h"

// semantic relations module
#include <mitkDICOMHelper.h>
#include <mitkSemanticRelationException.h>
#include <mitkSemanticTypes.h>
#include <mitkUIDGeneratorBoost.h>

// mitk core
#include "mitkNodePredicateData.h"
#include "mitkNodePredicateOr.h"
#include "mitkNodePredicateProperty.h"

QmitkSimpleDatamanagerWidget::QmitkSimpleDatamanagerWidget(mitk::DataStorage* dataStorage, QWidget* parent /*=nullptr*/)
  : QmitkSelectionWidget(dataStorage, parent)
{
  Init();
}

QmitkSimpleDatamanagerWidget::~QmitkSimpleDatamanagerWidget()
{
  // nothing here
}

void QmitkSimpleDatamanagerWidget::Init()
{
  // create GUI from the Qt Designer's .ui file
  m_Controls.setupUi(this);

  // create a new model
  m_NodeTreeModel = std::make_unique<QmitkDataStorageTreeModel>(m_DataStorage, false, this);
  m_NodeTreeModel->setParent(this);
  m_NodeTreeModel->SetPlaceNewNodesOnTop(true);
  m_NodeTreeModel->SetAllowHierarchyChange(false);
  // prepare filters
  m_HelperObjectFilterPredicate = mitk::NodePredicateOr::New(mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true)),
                                                             mitk::NodePredicateProperty::New("hidden object", mitk::BoolProperty::New(true)));
  m_NodeWithNoDataFilterPredicate = mitk::NodePredicateData::New(0);

  m_FilterModel = std::make_unique<QmitkDataStorageFilterProxyModel>(this);
  m_FilterModel->setSourceModel(m_NodeTreeModel.get());
  m_FilterModel->AddFilterPredicate(m_HelperObjectFilterPredicate);
  m_FilterModel->AddFilterPredicate(m_NodeWithNoDataFilterPredicate);

  m_Controls.dataNodeTreeView->setModel(m_FilterModel.get());
  m_Controls.dataNodeTreeView->setHeaderHidden(true);
  m_Controls.dataNodeTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Controls.dataNodeTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.dataNodeTreeView->setAlternatingRowColors(true);
  m_Controls.dataNodeTreeView->setTextElideMode(Qt::ElideMiddle);
  //m_Controls.dataNodeTreeView->installEventFilter(new QmitkNodeTableViewKeyFilter(this));

  //m_ItemDelegate = new QmitkDataManagerItemDelegate(m_Controls.dataNodeTreeView);
  //m_Controls.dataNodeTreeView->setItemDelegate(m_ItemDelegate);

  SetUpConnections();
}

void QmitkSimpleDatamanagerWidget::SetUpConnections()
{
  connect(m_Controls.dataNodeTreeView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), SLOT(OnNodeTreeViewSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

void QmitkSimpleDatamanagerWidget::OnNodeTreeViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  QModelIndex selectedIndex = m_Controls.dataNodeTreeView->currentIndex();
  if (!selectedIndex.isValid())
  {
    return;
  }

  QVariant qvariantDataNode = m_FilterModel->data(selectedIndex, QmitkDataNodeRawPointerRole);
  if (qvariantDataNode.canConvert<mitk::DataNode*>())
  {
    m_SelectedDataNode = qvariantDataNode.value<mitk::DataNode*>();
    emit SelectionChanged(m_SelectedDataNode);
  }
}
