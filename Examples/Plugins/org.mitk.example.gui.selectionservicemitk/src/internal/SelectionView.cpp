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

#include <mitkDataNode.h>

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

  //mitk::DataNode node1 = mitk::DataNode;
  //node1.SetName("Node 1");
  //mitk::DataNode mode2 = mitk::DataNode;
  //node2.SetName("Node 2");

  //m_Controls.m_SelectionList->setModel();
  //m_Controls.m_SelectionList->addItem

  // set selection mode to single selection
  m_Controls.m_SelectionList->setSelectionMode(QAbstractItemView::SingleSelection);

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