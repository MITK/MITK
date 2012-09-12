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

#include "ListenerView.h"

/// Qt
#include <QMessageBox>

#include "mitkDataNodeObject.h"

const std::string ListenerView::VIEW_ID = "org.mitk.views.listenerview"; 

ListenerView::ListenerView()
  //: QmitkFunctionality()
  : m_Parent(0)
  //, m_SelectionListener(new berry::SelectionChangedAdapter<ListenerView>(this, &ListenerView::SelectionChanged))
  , m_SelectionListener(NULL)
{
}

ListenerView::~ListenerView()
{
  //berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  //s->RemoveSelectionListener(m_SelectionListener);
}

void ListenerView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls.setupUi(parent);
  
  // register selection listener
  //GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddSelectionListener(m_SelectionListener);

  m_Parent->setEnabled(true);
}

void ListenerView::ToggleRadioMethod(QString selectStr)
{
  if (selectStr == "Selection 1") m_Controls.radioButton->toggle();
  else if (selectStr == "Selection 2") m_Controls.radioButton_2->toggle();
}

void ListenerView::SetFocus ()
{
}

//void ListenerView::SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart,
//                               berry::ISelection::ConstPointer selection)
void ListenerView::OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes)
{
  //any nodes there?
  if (!nodes.empty())
  {
  //  //get the selected Node
  //mitk::DataNode::Pointer dataNode = nodes.front();
  //ToggleRadioMethod(stringstringsrting);
  //  //DoSomething(selection.Cast<const berry::IStructuredSelection>());
  }
}