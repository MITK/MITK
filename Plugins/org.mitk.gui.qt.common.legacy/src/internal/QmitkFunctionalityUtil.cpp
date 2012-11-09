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


#include "QmitkFunctionalityUtil.h"

#include "../QmitkFunctionality.h"

QmitkFunctionalitySelectionProvider::QmitkFunctionalitySelectionProvider( QmitkFunctionality* _Functionality )
: m_Functionality(_Functionality)
{

}

QmitkFunctionalitySelectionProvider::~QmitkFunctionalitySelectionProvider()
{
  m_Functionality = 0;
}

void QmitkFunctionalitySelectionProvider::AddSelectionChangedListener( berry::ISelectionChangedListener::Pointer listener )
{
  m_SelectionEvents.AddListener(listener);
}


berry::ISelection::ConstPointer QmitkFunctionalitySelectionProvider::GetSelection() const
{
  return m_CurrentSelection;
}

void QmitkFunctionalitySelectionProvider::RemoveSelectionChangedListener( berry::ISelectionChangedListener::Pointer listener )
{
  m_SelectionEvents.RemoveListener(listener);
}

void QmitkFunctionalitySelectionProvider::SetSelection( berry::ISelection::ConstPointer selection )
{
  m_CurrentSelection = selection.Cast<const mitk::DataNodeSelection>();
}

void QmitkFunctionalitySelectionProvider::FireNodesSelected( std::vector<mitk::DataNode::Pointer> nodes )
{
  mitk::DataNodeSelection::Pointer sel(new mitk::DataNodeSelection(nodes));
  m_CurrentSelection = sel;
  berry::SelectionChangedEvent::Pointer event(new berry::SelectionChangedEvent(berry::ISelectionProvider::Pointer(this)
    , m_CurrentSelection));
  m_SelectionEvents.selectionChanged(event);

}
