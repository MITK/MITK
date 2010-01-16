/*=========================================================================
 
 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#ifndef CHERRYMOCKSELECTIONPROVIDER_H_
#define CHERRYMOCKSELECTIONPROVIDER_H_

#include <cherryISelectionProvider.h>
#include <cherryIStructuredSelection.h>

namespace cherry
{

class MockSelectionProvider: public ISelectionProvider
{

private:

  ISelectionChangedListener::Events listeners;
  IStructuredSelection::Pointer emptySelection;

public:

  cherryObjectMacro(MockSelectionProvider)

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
  void SetSelection(ISelection::Pointer selection);
};

}

#endif /* CHERRYMOCKSELECTIONPROVIDER_H_ */
