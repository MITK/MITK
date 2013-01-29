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

#include "SelectionViewMitk.h"

// qmitk Includes
#include <QmitkEnums.h>
#include <QmitkCustomVariants.h>

const std::string SelectionViewMitk::VIEW_ID = "org.mitk.views.selectionviewmitk";

SelectionViewMitk::SelectionViewMitk() : m_Parent(0)
{
}

void SelectionViewMitk::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls.setupUi(parent);

  //! [MITK Selection Provider data nodes]
  // create two data nodes and change there default name
  mitk::DataNode::Pointer dataNode1 = mitk::DataNode::New();
  dataNode1->SetName("DataNode 1");
  mitk::DataNode::Pointer dataNode2 = mitk::DataNode::New();
  dataNode2->SetName("DataNode 2");
  //! [MITK Selection Provider data nodes]

  //! [MITK Selection Provider listwidgetitems]
  // create two QListWidgetItems and name them after the create data nodes
  QListWidgetItem* listItemDataNode1 = new QListWidgetItem(QString::fromStdString(dataNode1->GetName()));
  QListWidgetItem* listItemDataNode2 = new QListWidgetItem(QString::fromStdString(dataNode2->GetName()));

  // set the data of created QListWidgetItems two the informations of the data nodes
  listItemDataNode1->setData(QmitkDataNodeRole, QVariant::fromValue(dataNode1));
  listItemDataNode2->setData(QmitkDataNodeRole, QVariant::fromValue(dataNode2));

  // add the items to the QListWidget of the view
  m_Controls.m_SelectionList->addItem(listItemDataNode1);
  m_Controls.m_SelectionList->addItem(listItemDataNode2);
  //! [MITK Selection Provider listwidgetitems]

  // set selection mode to single selection
  m_Controls.m_SelectionList->setSelectionMode(QAbstractItemView::SingleSelection);

  // pre-select the first item of the list
  m_Controls.m_SelectionList->setCurrentRow(0);

  m_Parent->setEnabled(true);

}

void SelectionViewMitk::SetFocus ()
{
}

//! [MITK Selection Provider method implementation]
QItemSelectionModel* SelectionViewMitk::GetDataNodeSelectionModel() const
{
  // return the selection model of the QListWidget
  return m_Controls.m_SelectionList->selectionModel();
}
//! [MITK Selection Provider method implementation]
