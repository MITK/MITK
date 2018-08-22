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
#include "QmitkPatientTableInspector.h"

// mitk qt widgets module
#include "QmitkCustomVariants.h"
#include "QmitkEnums.h"

// qt
#include <QKeyEvent>
#include <QSignalMapper>

QmitkPatientTableInspector::QmitkPatientTableInspector(QWidget* parent/* =nullptr*/)
{
  m_Controls.setupUi(this);

  m_Controls.tableView->horizontalHeader()->setHighlightSections(false);
  m_Controls.tableView->verticalHeader()->setHighlightSections(false);
  m_Controls.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_Controls.tableView->setSelectionBehavior(QAbstractItemView::SelectItems);
  m_Controls.tableView->setContextMenuPolicy(Qt::CustomContextMenu);

  m_StorageModel = new QmitkPatientTableModel(this);

  m_Controls.tableView->setModel(m_StorageModel);

  SetUpConnections();
}

QAbstractItemView* QmitkPatientTableInspector::GetView()
{
  return m_Controls.tableView;
}

const QAbstractItemView* QmitkPatientTableInspector::GetView() const
{
  return m_Controls.tableView;
}

void QmitkPatientTableInspector::SetSelectionMode(SelectionMode mode)
{
  m_Controls.tableView->setSelectionMode(mode);
}

QmitkPatientTableInspector::SelectionMode QmitkPatientTableInspector::GetSelectionMode() const
{
  return m_Controls.tableView->selectionMode();
}

void QmitkPatientTableInspector::SetCaseID(const mitk::SemanticTypes::CaseID& caseID)
{
  m_StorageModel->SetCaseID(caseID);
}

void QmitkPatientTableInspector::SetLesion(const mitk::SemanticTypes::Lesion& lesion)
{
  m_StorageModel->SetLesion(lesion);
}

QItemSelectionModel* QmitkPatientTableInspector::GetSelectionModel()
{
  return m_Controls.tableView->selectionModel();
}

void QmitkPatientTableInspector::Initialize()
{
  m_StorageModel->SetDataStorage(m_DataStorage.Lock());
  m_StorageModel->SetNodePredicate(m_NodePredicate);

  m_Connector->SetView(m_Controls.tableView);
}

void QmitkPatientTableInspector::OnModelUpdated()
{
  m_Controls.tableView->resizeRowsToContents();
  m_Controls.tableView->resizeColumnsToContents();
}

void QmitkPatientTableInspector::OnNodeButtonClicked(const QString& nodeType)
{
  m_StorageModel->SetNodeType(nodeType.toStdString());
}

void QmitkPatientTableInspector::OnItemDoubleClicked(const QModelIndex& itemIndex)
{
  if (itemIndex.isValid())
  {
    QVariant qvariantDataNode = m_StorageModel->data(itemIndex, QmitkDataNodeRawPointerRole);
    if (qvariantDataNode.canConvert<mitk::DataNode*>())
    {
      mitk::DataNode* dataNode = qvariantDataNode.value<mitk::DataNode*>();
      emit DataNodeDoubleClicked(dataNode);
    }
  }
}

void QmitkPatientTableInspector::SetUpConnections()
{
  connect(m_StorageModel, &QmitkPatientTableModel::ModelUpdated, this, &QmitkPatientTableInspector::OnModelUpdated);
  connect(m_Controls.tableView, &QTableView::customContextMenuRequested, this, &QmitkPatientTableInspector::OnContextMenuRequested);

  QSignalMapper* nodeButtonSignalMapper = new QSignalMapper(this);
  nodeButtonSignalMapper->setMapping(m_Controls.imageNodeButton, QString("Image"));
  nodeButtonSignalMapper->setMapping(m_Controls.segmentationNodeButton, QString("Segmentation"));
  connect(nodeButtonSignalMapper, static_cast<void (QSignalMapper::*)(const QString&)>(&QSignalMapper::mapped), this, &QmitkPatientTableInspector::OnNodeButtonClicked);
  connect(m_Controls.imageNodeButton, SIGNAL(clicked()), nodeButtonSignalMapper, SLOT(map()));
  connect(m_Controls.segmentationNodeButton, SIGNAL(clicked()), nodeButtonSignalMapper, SLOT(map()));
  m_Controls.imageNodeButton->setChecked(true);

  connect(m_Controls.tableView, &QTableView::doubleClicked, this, &QmitkPatientTableInspector::OnItemDoubleClicked);
}

void QmitkPatientTableInspector::keyPressEvent(QKeyEvent* e)
{
  mitk::DataNode* dataNode = nullptr;
  QModelIndex selectedIndex = m_Controls.tableView->currentIndex();
  if (selectedIndex.isValid())
  {
    QVariant qvariantDataNode = m_StorageModel->data(selectedIndex, QmitkDataNodeRawPointerRole);
    if (qvariantDataNode.canConvert<mitk::DataNode*>())
    {
      dataNode = qvariantDataNode.value<mitk::DataNode*>();
    }
  }

  if (nullptr == dataNode)
  {
    return;
  }

  int key = e->key();
  switch (key)
  {
  case Qt::Key_Delete:
    emit OnNodeRemoved(dataNode);
    break;
  default:
    break;
  }
}
