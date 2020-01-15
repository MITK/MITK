/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkFunctionalityUtil.h"

#include "../QmitkFunctionality.h"

QmitkFunctionalitySelectionProvider::QmitkFunctionalitySelectionProvider( QmitkFunctionality* _Functionality )
: m_Functionality(_Functionality)
{

}

QmitkFunctionalitySelectionProvider::~QmitkFunctionalitySelectionProvider()
{
  m_Functionality = nullptr;
}

void QmitkFunctionalitySelectionProvider::AddSelectionChangedListener( berry::ISelectionChangedListener* listener )
{
  m_SelectionEvents.AddListener(listener);
}


berry::ISelection::ConstPointer QmitkFunctionalitySelectionProvider::GetSelection() const
{
  return m_CurrentSelection;
}

void QmitkFunctionalitySelectionProvider::RemoveSelectionChangedListener( berry::ISelectionChangedListener* listener )
{
  m_SelectionEvents.RemoveListener(listener);
}

void QmitkFunctionalitySelectionProvider::SetSelection(const berry::ISelection::ConstPointer& selection )
{
  m_CurrentSelection = selection.Cast<const mitk::DataNodeSelection>();
}

void QmitkFunctionalitySelectionProvider::FireNodesSelected(const std::vector<mitk::DataNode::Pointer>& nodes )
{
  mitk::DataNodeSelection::Pointer sel(new mitk::DataNodeSelection(nodes));
  m_CurrentSelection = sel;
  berry::SelectionChangedEvent::Pointer event(new berry::SelectionChangedEvent(berry::ISelectionProvider::Pointer(this)
    , m_CurrentSelection));
  m_SelectionEvents.selectionChanged(event);

}
