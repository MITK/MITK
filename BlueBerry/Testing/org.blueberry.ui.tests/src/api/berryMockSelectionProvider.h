/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYMOCKSELECTIONPROVIDER_H_
#define BERRYMOCKSELECTIONPROVIDER_H_

#include <berryISelectionProvider.h>
#include <berryIStructuredSelection.h>

namespace berry
{

class MockSelectionProvider: public ISelectionProvider
{

private:

  ISelectionChangedListener::Events listeners;
  IStructuredSelection::Pointer emptySelection;

public:

  berryObjectMacro(MockSelectionProvider)

  MockSelectionProvider();

  /**
   * Fires out a selection to all listeners.
   */
  void FireSelection();

  /**
   * Fires out a selection to all listeners.
   */
  void FireSelection(SelectionChangedEvent::Pointer event);

  /**
   * @see ISelectionProvider#addSelectionChangedListener(ISelectionChangedListener)
   */
  void AddSelectionChangedListener(ISelectionChangedListener::Pointer listener);

  /**
   * @see ISelectionProvider#getSelection()
   */
  ISelection::ConstPointer GetSelection() const;

  /**
   * @see ISelectionProvider#removeSelectionChangedListener(ISelectionChangedListener)
   */
  void RemoveSelectionChangedListener(
      ISelectionChangedListener::Pointer listener);

  /**
   * @see ISelectionProvider#setSelection(ISelection)
   */
  void SetSelection(ISelection::Pointer /*selection*/);
};

}

#endif /* BERRYMOCKSELECTIONPROVIDER_H_ */
