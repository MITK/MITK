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

#include "mitkAlgorithmInfoSelectionProvider.h"
#include "mitkMAPAlgorithmInfoSelection.h"

namespace mitk {

AlgorithmInfoSelectionProvider::AlgorithmInfoSelectionProvider() : _selection(NULL)
{

}

void AlgorithmInfoSelectionProvider::AddSelectionChangedListener( berry::ISelectionChangedListener* listener )
{
  _selectionEvents.AddListener(listener);
}

void AlgorithmInfoSelectionProvider::RemoveSelectionChangedListener( berry::ISelectionChangedListener* listener )
{
  _selectionEvents.RemoveListener(listener);
}

berry::ISelection::ConstPointer AlgorithmInfoSelectionProvider::GetSelection() const
{
  return _selection;
}

void AlgorithmInfoSelectionProvider::SetSelection(const berry::ISelection::ConstPointer& selection)
{
  _selection = selection.Cast<const MAPAlgorithmInfoSelection>();
}

MAPAlgorithmInfoSelection::ConstPointer AlgorithmInfoSelectionProvider::GetInfoSelection() const
{
  return _selection;
}

void AlgorithmInfoSelectionProvider::SetInfoSelection(MAPAlgorithmInfoSelection::ConstPointer selection)
{
  if (_selection != selection)
  {
    _selection = selection;
    berry::SelectionChangedEvent::Pointer event(new berry::SelectionChangedEvent(berry::ISelectionProvider::Pointer(this), selection));
    _selectionEvents.selectionChanged(event);
  }
}



}
