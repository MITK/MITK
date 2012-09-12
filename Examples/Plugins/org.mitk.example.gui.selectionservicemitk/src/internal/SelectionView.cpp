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

#include "SelectionView.h"

/// Qt
#include <QMessageBox>
//#include <QString>
//#include <QListWidgetItem>

//#include <mitkDataNodeFactory.h>
//#include <mitkDataNode.h>

#include <QmitkCustomVariants.h>
//#include "QmitkDataNodeItemModel.h"
//#include <QmitkDataStorageTreeModel.h>

const std::string SelectionView::VIEW_ID = "org.mitk.views.selectionview"; 

SelectionView::SelectionView() : m_Parent(0)
{
}

SelectionView::~SelectionView()
{  
}

void SelectionView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls.setupUi(parent);

  //m_NodeTreeModel = new QmitkDataStorageTreeModel(this->GetDataStorage());
  //m_NodeTreeModel->setParent( parent );

  //m_Controls.m_treeView->setModel(m_NodeTreeModel);

  //mitk::DataNodeFactory::Pointer nodeReader=mitk::DataNodeFactory::New();
  //nodeReader->SetFileName("C:/home/testImageData1.png");
  //nodeReader->Update();

  //mitk::DataNode::Pointer dataNode1 = nodeReader->GetOutput();
  //dataNode1->SetName("Node1");

  //nodeReader->SetFileName("testImageData2.png");
  //nodeReader->Update();
  //mitk::DataNode::Pointer dataNode2 = nodeReader->GetOutput();
  //dataNode2->SetName("Node2");

  mitk::DataNode::Pointer dataNode1 = mitk::DataNode::New();
  dataNode1->SetName("Node1");
  mitk::DataNode::Pointer dataNode2 = mitk::DataNode::New();
  dataNode2->SetName("Node2");

  QListWidgetItem* listItemDataNode1 = new QListWidgetItem(QString::fromStdString(dataNode1->GetName()));
  QListWidgetItem* listItemDataNode2 = new QListWidgetItem(QString::fromStdString(dataNode2->GetName()));

  listItemDataNode1->setData(64/*QmitkDataNodeRole*/, QVariant::fromValue(dataNode1));
  //listItemDataNode2->setData(64/*QmitkDataNodeRole*/, QVariant::fromValue(dataNode2));

  m_Controls.m_SelectionList->addItem(listItemDataNode1);
  m_Controls.m_SelectionList->addItem(listItemDataNode2);

  // set selection mode to single selection
  m_Controls.m_SelectionList->setSelectionMode(QAbstractItemView::SingleSelection);
  //m_Controls.m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);

  // pre-select the first item of the list
  m_Controls.m_SelectionList->setCurrentRow(0);

  m_Parent->setEnabled(true);

}

void SelectionView::SetFocus ()
{
}

QItemSelectionModel* SelectionView::GetDataNodeSelectionModel() const
{
  return m_Controls.m_SelectionList->selectionModel();
}