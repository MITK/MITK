/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIPERSPECTIVELISTENER_H_
#define BERRYIPERSPECTIVELISTENER_H_

#include <org_blueberry_ui_qt_Export.h>
#include "berryIPerspectiveDescriptor.h"
#include "berryIWorkbenchPartReference.h"

#include <berryMacros.h>

namespace berry {

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
struct BERRY_UI_QT IPerspectiveListener
{

  struct Events {

    enum Type {
      NONE           = 0x00000000,
      ACTIVATED      = 0x00000001,
      CHANGED        = 0x00000002,
      PART_CHANGED   = 0x00000004,
      OPENED         = 0x00000008,
      CLOSED         = 0x00000010,
      DEACTIVATED    = 0x00000020,
      SAVED_AS       = 0x00000040,
      PRE_DEACTIVATE = 0x00000080,

      ALL            = 0xffffffff
    };

    Q_DECLARE_FLAGS(Types, Type)

    Message2<const SmartPointer<IWorkbenchPage>&, const IPerspectiveDescriptor::Pointer&> perspectiveActivated;
    Message3<const SmartPointer<IWorkbenchPage>&, const IPerspectiveDescriptor::Pointer&, const QString&> perspectiveChanged;
    Message4<const SmartPointer<IWorkbenchPage>&, const IPerspectiveDescriptor::Pointer&, const SmartPointer<IWorkbenchPartReference>&, const QString&> perspectivePartChanged;
    Message2<const SmartPointer<IWorkbenchPage>&, const IPerspectiveDescriptor::Pointer&> perspectiveOpened;
    Message2<const SmartPointer<IWorkbenchPage>&, const IPerspectiveDescriptor::Pointer&> perspectiveClosed;
    Message2<const SmartPointer<IWorkbenchPage>&, const IPerspectiveDescriptor::Pointer&> perspectiveDeactivated;
    Message3<const SmartPointer<IWorkbenchPage>&, const IPerspectiveDescriptor::Pointer&, const IPerspectiveDescriptor::Pointer&> perspectiveSavedAs;
    Message2<const SmartPointer<IWorkbenchPage>&, const IPerspectiveDescriptor::Pointer&> perspectivePreDeactivate;

    void AddListener(IPerspectiveListener* l);
    void RemoveListener(IPerspectiveListener* l);

  private:

    typedef MessageDelegate2<IPerspectiveListener, const SmartPointer<IWorkbenchPage>&, const IPerspectiveDescriptor::Pointer& > Delegate2;
    typedef MessageDelegate3<IPerspectiveListener, const SmartPointer<IWorkbenchPage>&, const IPerspectiveDescriptor::Pointer&, const QString&> PerspChangedDelegate;
    typedef MessageDelegate3<IPerspectiveListener, const SmartPointer<IWorkbenchPage>&, const IPerspectiveDescriptor::Pointer&, const IPerspectiveDescriptor::Pointer&> PerspSavedAsDelegate;
    typedef MessageDelegate4<IPerspectiveListener, const SmartPointer<IWorkbenchPage>&, const IPerspectiveDescriptor::Pointer&, const SmartPointer<IWorkbenchPartReference>&, const QString&> PerspPartChangedDelegate;

  };

  virtual ~IPerspectiveListener();

  virtual Events::Types GetPerspectiveEventTypes() const = 0;

  /**
   * Notifies this listener that a perspective in the given page
   * has been activated.
   *
   * @param page the page containing the activated perspective
   * @param perspective the perspective descriptor that was activated
   * @see IWorkbenchPage#setPerspective
   */
  virtual void PerspectiveActivated(const SmartPointer<IWorkbenchPage>& page,
                                    const IPerspectiveDescriptor::Pointer& perspective);

  /**
   * Notifies this listener that a perspective has changed in some way
   * (for example, editor area hidden, perspective reset,
   * view show/hide, editor open/close, etc).
   *
   * @param page the page containing the affected perspective
   * @param perspective the perspective descriptor
   * @param changeId one of the <code>CHANGE_*</code> constants on IWorkbenchPage
   */
  virtual void PerspectiveChanged(const SmartPointer<IWorkbenchPage>& page,
          const IPerspectiveDescriptor::Pointer& perspective, const QString& changeId);

  /**
   * Notifies this listener that a part in the given page's perspective
   * has changed in some way (for example, view show/hide, editor open/close, etc).
   *
   * @param page the workbench page containing the perspective
   * @param perspective the descriptor for the changed perspective
   * @param partRef the reference to the affected part
   * @param changeId one of the <code>CHANGE_*</code> constants on IWorkbenchPage
   */
  virtual void PerspectiveChanged(const SmartPointer<IWorkbenchPage>& page,
          const IPerspectiveDescriptor::Pointer& perspective,
          const SmartPointer<IWorkbenchPartReference>& partRef, const QString& changeId);

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
  virtual void PerspectiveOpened(const SmartPointer<IWorkbenchPage>& page,
          const IPerspectiveDescriptor::Pointer& perspective);

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
  virtual void PerspectiveClosed(const SmartPointer<IWorkbenchPage>& page,
          const IPerspectiveDescriptor::Pointer& perspective);

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
  virtual void PerspectiveDeactivated(const SmartPointer<IWorkbenchPage>& page,
          const IPerspectiveDescriptor::Pointer& perspective);

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
  virtual void PerspectiveSavedAs(const SmartPointer<IWorkbenchPage>& page,
          const IPerspectiveDescriptor::Pointer& oldPerspective,
          const IPerspectiveDescriptor::Pointer& newPerspective);

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
  virtual void PerspectivePreDeactivate(const SmartPointer<IWorkbenchPage>& page,
      const IPerspectiveDescriptor::Pointer& perspective);
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(berry::IPerspectiveListener::Events::Types)

#endif /* BERRYIPERSPECTIVELISTENER_H_ */
