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

#ifndef CHERRYISELECTIONLISTENER_H_
#define CHERRYISELECTIONLISTENER_H_

#include <cherryMessage.h>

#include "cherryUiDll.h"

#include "cherryIWorkbenchPart.h"
#include "cherryISelection.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 *
 * Interface for listening to selection changes.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @see ISelectionService#AddSelectionListener(ISelectionListener::Ptr)
 * @see ISelectionService#AddSelectionListener(std::string, ISelectionListener::Ptr)
 * @see org.opencherry.ui.INullSelectionListener
 */
struct CHERRY_UI ISelectionListener : public virtual Object {

  cherryInterfaceMacro(ISelectionListener, cherry);

  struct Events {

    Message2<IWorkbenchPart::Pointer, ISelection::ConstPointer> selectionChanged;

    void AddListener(ISelectionListener::Pointer listener);
    void RemoveListener(ISelectionListener::Pointer listener);

  private:

    typedef MessageDelegate2<ISelectionListener, IWorkbenchPart::Pointer, ISelection::ConstPointer> Delegate;
  };

  /**
   * Notifies this listener that the selection has changed.
   * <p>
   * This method is called when the selection changes from one to a
   * <code>non-null</code> value, but not when the selection changes to
   * <code>null</code>. If there is a requirement to be notified in the latter
   * scenario, implement <code>INullSelectionListener</code>. The event will
   * be posted through this method.
   * </p>
   *
   * @param part the workbench part containing the selection
   * @param selection the current selection. This may be <code>null</code>
   *    if <code>INullSelectionListener</code> is implemented.
   */
  virtual void SelectionChanged(IWorkbenchPart::Pointer part, ISelection::ConstPointer selection) = 0;
};

}

#endif /*CHERRYISELECTIONLISTENER_H_*/
