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

  m_SelectionProvider = new berry::QtSelectionProvider();
  m_SelectionProvider->SetItemSelectionModel(m_Controls.m_SelectionList->selectionModel());
  GetSite()->SetSelectionProvider(m_SelectionProvider);

  // set selection mode to single selection
  m_Controls.m_SelectionList->setSelectionMode(QAbstractItemView::SingleSelection);

  // pre-select the first item of the list
  m_Controls.m_SelectionList->setCurrentRow(0);
  
  //connect(m_Controls.m_SelectionList, SIGNAL(itemSelectionChanged()), this, SLOT(TestMethod()));  //Debugging only!

  m_Parent->setEnabled(true);

}

void SelectionView::SetFocus ()
{
}

//void SelectionView::TestMethod()  //Debugging only!
//{
//  QMessageBox::critical(0, "Error", " List selection changed! ");
//}