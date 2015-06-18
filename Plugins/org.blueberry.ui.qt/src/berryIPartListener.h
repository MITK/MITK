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

#ifndef BERRYIPARTLISTENER_H_
#define BERRYIPARTLISTENER_H_

#include <berryMacros.h>
#include <berryMessage.h>

#include <org_blueberry_ui_qt_Export.h>
#include "berryIWorkbenchPartReference.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Interface for listening to part lifecycle events.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @see IPartService#AddPartListener(IPartListener)
 */
struct BERRY_UI_QT IPartListener
{

  struct Events {

    enum Type {
      NONE           = 0x00000000,
      ACTIVATED      = 0x00000001,
      BROUGHT_TO_TOP = 0x00000002,
      CLOSED         = 0x00000004,
      DEACTIVATED    = 0x00000008,
      OPENED         = 0x00000010,
      HIDDEN         = 0x00000020,
      VISIBLE        = 0x00000040,
      INPUT_CHANGED  = 0x00000080,

      ALL            = 0xffffffff
    };

    Q_DECLARE_FLAGS(Types, Type)

    typedef Message1<const IWorkbenchPartReference::Pointer&> PartEvent;

    PartEvent partActivated;
    PartEvent partBroughtToTop;
    PartEvent partClosed;
    PartEvent partDeactivated;
    PartEvent partOpened;
    PartEvent partHidden;
    PartEvent partVisible;
    PartEvent partInputChanged;

    void AddListener(IPartListener* listener);
    void RemoveListener(IPartListener* listener);

  private:
    typedef MessageDelegate1<IPartListener, const IWorkbenchPartReference::Pointer&> Delegate;
  };

  virtual ~IPartListener();

  virtual Events::Types GetPartEventTypes() const = 0;

  /**
   * Notifies this listener that the given part has been activated.
   *
   * @param partRef the part that was activated
   * @see IWorkbenchPage#activate
   */
  virtual void PartActivated(const IWorkbenchPartReference::Pointer& /*partRef*/) {}

  /**
   * Notifies this listener that the given part has been brought to the top.
   * <p>
   * These events occur when an editor is brought to the top in the editor area,
   * or when a view is brought to the top in a page book with multiple views.
   * They are normally only sent when a part is brought to the top
   * programmatically (via <code>IPerspective.bringToTop</code>). When a part is
   * activated by the user clicking on it, only <code>partActivated</code> is sent.
   * </p>
   *
   * @param partRef the part that was surfaced
   * @see IWorkbenchPage#bringToTop
   */
  virtual void PartBroughtToTop(const IWorkbenchPartReference::Pointer& /*partRef*/) {}

  /**
   * Notifies this listener that the given part has been closed.
   * <p>
   * Note that if other perspectives in the same page share the view,
   * this notification is not sent.  It is only sent when the view
   * is being removed from the page entirely (it is being disposed).
   * </p>
   *
   * @param partRef the part that was closed
   * @see IWorkbenchPage#hideView
   */
  virtual void PartClosed(const IWorkbenchPartReference::Pointer& /*partRef*/) {}

  /**
   * Notifies this listener that the given part has been deactivated.
   *
   * @param partRef the part that was deactivated
   * @see IWorkbenchPage#activate
   */
  virtual void PartDeactivated(const IWorkbenchPartReference::Pointer& /*partRef*/) {}

  /**
   * Notifies this listener that the given part has been opened.
   * <p>
   * Note that if other perspectives in the same page share the view,
   * this notification is not sent.  It is only sent when the view
   * is being newly opened in the page (it is being created).
   * </p>
   *
   * @param partRef the part that was opened
   * @see IWorkbenchPage#showView
   */
  virtual void PartOpened(const IWorkbenchPartReference::Pointer& /*partRef*/) {}

  /**
   * Notifies this listener that the given part is hidden or obscured by another part.
   *
   * @param partRef the part that is hidden or obscured by another part
   */
  virtual void PartHidden(const IWorkbenchPartReference::Pointer& /*partRef*/) {}

  /**
   * Notifies this listener that the given part is visible.
   *
   * @param partRef the part that is visible
   */
  virtual void PartVisible(const IWorkbenchPartReference::Pointer& /*partRef*/) {}

  /**
   * Notifies this listener that the given part's input was changed.
   *
   * @param partRef the part whose input was changed
   */
  virtual void PartInputChanged(const IWorkbenchPartReference::Pointer& /*partRef*/) {}
};

}  // namespace berry

Q_DECLARE_OPERATORS_FOR_FLAGS(berry::IPartListener::Events::Types)

#endif /*BERRYIPARTLISTENER_H_*/
