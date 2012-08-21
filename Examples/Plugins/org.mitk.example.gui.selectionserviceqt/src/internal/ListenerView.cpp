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

/// Berry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

/// Qt
#include <QMessageBox>
#include <QString>
#include <QListWidgetItem>

const std::string ListenerView::VIEW_ID = "org.mitk.views.listenerview"; 

ListenerView::ListenerView()
  : m_Parent(0)
  , m_SelectionListener(new berry::SelectionChangedAdapter<ListenerView>(this, &ListenerView::SelectionChanged))
{
}

ListenerView::~ListenerView()
{
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  s->RemoveSelectionListener(m_SelectionListener);
}

void ListenerView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Parent = parent;
  m_Controls.setupUi(parent);
  
  // register selection listener
  GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddSelectionListener(m_SelectionListener);
  
  m_Parent->setEnabled(true);
}

//void ListenerView::ToggleRadioMethod(berry::IStructuredSelection::ConstPointer m_CurrentSelection)
void ListenerView::ToggleRadioMethod()
{
  
  bool buttonState = m_Controls.radioButton->isChecked();
  if (buttonState) m_Controls.radioButton_2->toggle();
  else m_Controls.radioButton->toggle();
}

void ListenerView::SetFocus ()
{
}

void ListenerView::SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart,
                               berry::ISelection::ConstPointer selection)
{
  if (selection.IsNull())
  {
    return;
  }

  if (sourcepart != this && 
      selection.Cast<const berry::IStructuredSelection>())
  {
    ToggleRadioMethod();
    m_CurrentSelection = selection.Cast<const berry::IStructuredSelection>();
    // check if we can cast it to listwidgetitem
    m_CurrentSelection->Begin();
    //berry::Object::Pointer m_CurrentItem = m_CurrentSelection->GetFirstElement();
    QMessageBox::critical(0, "Error", "TEST");
    //ToggleRadioMethod(selection.Cast<const berry::IStructuredSelection>());
  }
}