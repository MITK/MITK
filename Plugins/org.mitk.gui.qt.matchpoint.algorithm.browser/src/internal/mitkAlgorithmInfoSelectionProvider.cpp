/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkAlgorithmInfoSelectionProvider.h"
#include "mitkMAPAlgorithmInfoSelection.h"

namespace mitk {

AlgorithmInfoSelectionProvider::AlgorithmInfoSelectionProvider() : _selection(nullptr)
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
