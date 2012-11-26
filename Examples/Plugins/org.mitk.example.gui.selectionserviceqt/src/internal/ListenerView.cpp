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

// berry includes
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryQModelIndexObject.h>

// Qt includes
#include <QString>
#include <QModelIndex>
#include <QVariant>

const std::string ListenerView::VIEW_ID = "org.mitk.views.listenerview";

ListenerView::ListenerView()
  : m_SelectionListener(new berry::SelectionChangedAdapter<ListenerView>(this, &ListenerView::SelectionChanged))
  , m_Parent(0)
{
}

ListenerView::~ListenerView()
{
  // remove selection service
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

void ListenerView::ToggleRadioMethod(QString selectStr)
{
  // change the radio button state according to the name of the selected element
  if (selectStr == "Selection 1") m_Controls.radioButton->toggle();
  else if (selectStr == "Selection 2") m_Controls.radioButton_2->toggle();
}

void ListenerView::SetFocus()
{
}

//! [Qt Selection Listener method implementation]
void ListenerView::SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart,
                               berry::ISelection::ConstPointer selection)
{
  // check for null selection
  if (selection.IsNull())
  {
    return;
  }
  // exclude own selection events and check whether this kind of selection can be handled
  if (sourcepart != this &&
      selection.Cast<const berry::IStructuredSelection>())
  {
    berry::IStructuredSelection::ConstPointer currentSelection = selection.Cast<const berry::IStructuredSelection>();
    // iterate over the selections (for the BlueBerry example this is always 1
    for (berry::IStructuredSelection::iterator itr = currentSelection->Begin();
         itr != currentSelection->End(); ++itr)
    {
      if (berry::QModelIndexObject::Pointer object = itr->Cast<berry::QModelIndexObject>())
      {
        // get name of selected ListWidgetElement
        QVariant text = object->GetQModelIndex().data();
        // pass name of element to method for radio button state checking
        ToggleRadioMethod(text.toString());
      }
    }
  }
}
//! [Qt Selection Listener method implementation]
