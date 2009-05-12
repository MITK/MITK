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


#ifndef CHERRYIPERSPECTIVELISTENER_H_
#define CHERRYIPERSPECTIVELISTENER_H_

#include "cherryUiDll.h"
#include "cherryIPerspectiveDescriptor.h"
#include "cherryIWorkbenchPartReference.h"

#include <cherryMacros.h>

namespace cherry {

struct IWorkbenchPage;

/**
 * Interface for listening to perspective lifecycle events.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @see IPageService#addPerspectiveListener(IPerspectiveListener)
 * @see PerspectiveAdapter
 */
struct CHERRY_UI IPerspectiveListener : public virtual Object {

  cherryInterfaceMacro(IPerspectiveListener, cherry);

  struct Events {

    Message2<SmartPointer<IWorkbenchPage>, IPerspectiveDescriptor::Pointer> perspectiveActivated;
    Message3<SmartPointer<IWorkbenchPage>, IPerspectiveDescriptor::Pointer, const std::string&> perspectiveChanged;
    Message4<SmartPointer<IWorkbenchPage>, IPerspectiveDescriptor::Pointer, IWorkbenchPartReference::Pointer, const std::string&> perspectivePartChanged;
    Message2<SmartPointer<IWorkbenchPage>, IPerspectiveDescriptor::Pointer> perspectiveOpened;
    Message2<SmartPointer<IWorkbenchPage>, IPerspectiveDescriptor::Pointer> perspectiveClosed;
    Message2<SmartPointer<IWorkbenchPage>, IPerspectiveDescriptor::Pointer> perspectiveDeactivated;
    Message3<SmartPointer<IWorkbenchPage>, IPerspectiveDescriptor::Pointer, IPerspectiveDescriptor::Pointer> perspectiveSavedAs;
    Message2<SmartPointer<IWorkbenchPage>, IPerspectiveDescriptor::Pointer> perspectivePreDeactivate;

    void AddListener(IPerspectiveListener::Pointer l);
    void RemoveListener(IPerspectiveListener::Pointer l);

  private:

    typedef MessageDelegate2<IPerspectiveListener, SmartPointer<IWorkbenchPage>, IPerspectiveDescriptor::Pointer > Delegate2;
    typedef MessageDelegate3<IPerspectiveListener, SmartPointer<IWorkbenchPage>, IPerspectiveDescriptor::Pointer, const std::string&> PerspChangedDelegate;
    typedef MessageDelegate3<IPerspectiveListener, SmartPointer<IWorkbenchPage>, IPerspectiveDescriptor::Pointer, IPerspectiveDescriptor::Pointer> PerspSavedAsDelegate;
    typedef MessageDelegate4<IPerspectiveListener, SmartPointer<IWorkbenchPage>, IPerspectiveDescriptor::Pointer, IWorkbenchPartReference::Pointer, const std::string&> PerspPartChangedDelegate;

  };

  /**
   * Notifies this listener that a perspective in the given page
   * has been activated.
   *
   * @param page the page containing the activated perspective
   * @param perspective the perspective descriptor that was activated
   * @see IWorkbenchPage#setPerspective
   */
  virtual void PerspectiveActivated(SmartPointer<IWorkbenchPage> page,
          IPerspectiveDescriptor::Pointer perspective);

  /**
   * Notifies this listener that a perspective has changed in some way
   * (for example, editor area hidden, perspective reset,
   * view show/hide, editor open/close, etc).
   *
   * @param page the page containing the affected perspective
   * @param perspective the perspective descriptor
   * @param changeId one of the <code>CHANGE_*</code> constants on IWorkbenchPage
   */
  virtual void PerspectiveChanged(SmartPointer<IWorkbenchPage> page,
          IPerspectiveDescriptor::Pointer perspective, const std::string& changeId);

  /**
   * Notifies this listener that a part in the given page's perspective
   * has changed in some way (for example, view show/hide, editor open/close, etc).
   *
   * @param page the workbench page containing the perspective
   * @param perspective the descriptor for the changed perspective
   * @param partRef the reference to the affected part
   * @param changeId one of the <code>CHANGE_*</code> constants on IWorkbenchPage
   */
  virtual void PerspectiveChanged(SmartPointer<IWorkbenchPage> page,
          IPerspectiveDescriptor::Pointer perspective,
          IWorkbenchPartReference::Pointer partRef, const std::string& changeId);

  /**
   * Notifies this listener that a perspective in the given page has been
   * opened.
   *
   * @param page
   *            the page containing the opened perspective
   * @param perspective
   *            the perspective descriptor that was opened
   * @see IWorkbenchPage#setPerspective(IPerspectiveDescriptor)
   */
  virtual void PerspectiveOpened(SmartPointer<IWorkbenchPage> page,
          IPerspectiveDescriptor::Pointer perspective);

  /**
   * Notifies this listener that a perspective in the given page has been
   * closed.
   *
   * @param page
   *            the page containing the closed perspective
   * @param perspective
   *            the perspective descriptor that was closed
   * @see IWorkbenchPage#closePerspective(IPerspectiveDescriptor, boolean, boolean)
   * @see IWorkbenchPage#closeAllPerspectives(boolean, boolean)
   */
  virtual void PerspectiveClosed(SmartPointer<IWorkbenchPage> page,
          IPerspectiveDescriptor::Pointer perspective);

  /**
   * Notifies this listener that a perspective in the given page has been
   * deactivated.
   *
   * @param page
   *            the page containing the deactivated perspective
   * @param perspective
   *            the perspective descriptor that was deactivated
   * @see IWorkbenchPage#setPerspective(IPerspectiveDescriptor)
   */
  virtual void PerspectiveDeactivated(SmartPointer<IWorkbenchPage> page,
          IPerspectiveDescriptor::Pointer perspective);

  /**
   * Notifies this listener that a perspective in the given page has been
   * saved as a new perspective with a different perspective descriptor.
   *
   * @param page
   *            the page containing the saved perspective
   * @param oldPerspective
   *            the old perspective descriptor
   * @param newPerspective
   *            the new perspective descriptor
   * @see IWorkbenchPage#savePerspectiveAs(IPerspectiveDescriptor)
   */
  virtual void PerspectiveSavedAs(SmartPointer<IWorkbenchPage> page,
          IPerspectiveDescriptor::Pointer oldPerspective,
          IPerspectiveDescriptor::Pointer newPerspective);

  /**
   * <p>
   * Notifies this listener that a perspective in the given page is about to
   * be deactivated.
   * </p>
   * <p>
   * Note: This does not have the ability to veto a perspective deactivation.
   * </p>
   *
   * @param page
   *            the page containing the deactivated perspective
   * @param perspective
   *            the perspective descriptor that was deactivated
   * @see IWorkbenchPage#setPerspective(IPerspectiveDescriptor)
   */
  virtual void PerspectivePreDeactivate(SmartPointer<IWorkbenchPage> page,
      IPerspectiveDescriptor::Pointer perspective);
};

}

#endif /* CHERRYIPERSPECTIVELISTENER_H_ */
