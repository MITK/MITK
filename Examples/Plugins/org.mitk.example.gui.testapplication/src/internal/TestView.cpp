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

#include "TestView.h"

#include <QMessageBox>

const std::string TestView::VIEW_ID = "org.mitk.views.testview"; 

//MinimalView::MinimalView()
//  : m_Parent(0)
//  , m_SelectionListener(new berry::SelectionChangedAdapter<MinimalView>(this, &MinimalView::SelectionChanged))
//{
//}

TestView::TestView()
  : m_Parent(0)
{
}

TestView::~TestView()
{
  //berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  //s->RemoveSelectionListener(m_SelectionListener);
}

void TestView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls.setupUi(parent);

  //connect(m_Controls.selectButton, SIGNAL(clicked()), this, SLOT(ToggleRadioMethod()));
  
  // register selection listener
  //GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddSelectionListener(m_SelectionListener);

  m_Parent->setEnabled(true);
}
//{
//  // create GUI widgets
//  m_Parent = parent;
//  m_Controls.setupUi(parent);
//
//  connect(m_Controls.selectButton, SIGNAL(clicked()), this, SLOT(ToggleRadioMethod()));
//  
//  // register selection listener
//  GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddSelectionListener(m_SelectionListener);
//
//  m_Parent->setEnabled(true);
//}

//void MinimalView::ToggleRadioMethod()
//{
//  bool buttonState = m_Controls.radioButton->isChecked();
//  if (buttonState) m_Controls.radioButton_2->toggle();
//  else m_Controls.radioButton->toggle();
//}

void TestView::SetFocus ()
{
}

//void MinimalView::SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart,
//                               berry::ISelection::ConstPointer selection)
//{
//  if (sourcepart != this && 
//      selection.Cast<const berry::IStructuredSelection>())
//  {
//    ToggleRadioMethod();
//    //DoSomething(selection.Cast<const berry::IStructuredSelection>());
//  }
//}