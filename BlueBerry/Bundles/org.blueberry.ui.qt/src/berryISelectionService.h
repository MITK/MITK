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

#ifndef BERRYISELECTIONSERVICE_H_
#define BERRYISELECTIONSERVICE_H_

#include <org_blueberry_ui_qt_Export.h>

#include "berryISelection.h"
#include "berryISelectionListener.h"
#include "berryIWorkbenchPart.h"

#include <berryMessage.h>

namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 *
 * A selection service tracks the selection within an object.
 * <p>
 * A listener that wants to be notified when the selection becomes
 * <code>null</code> must implement the <code>INullSelectionListener</code>
 * interface.
 * </p>
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 * @see org.blueberry.ui.ISelectionListener
 * @see org.blueberry.ui.INullSelectionListener
 */
struct BERRY_UI_QT ISelectionService {

  struct SelectionEvents {

    typedef Message2<IWorkbenchPart::Pointer, ISelection::ConstPointer> SelectionEvent;
    typedef MessageDelegate2<ISelectionListener, IWorkbenchPart::Pointer, ISelection::ConstPointer> Delegate;

    SelectionEvent selectionChanged;
    SelectionEvent postSelectionChanged;

  };

  virtual ~ISelectionService();

  //virtual SelectionEvents& GetSelectionEvents(const QString& partId = "") = 0;

  /**
   * Adds the given selection listener.
   * Has no effect if an identical listener is already registered.
   *
   * @param listener a selection listener
   */
  virtual void AddSelectionListener(ISelectionListener* listener) = 0;

  /**
   * Adds a part-specific selection listener which is notified when selection changes
   * in the part with the given id. This is independent of part activation - the part
   * need not be active for notification to be sent.
   * <p>
   * When the part is created, the listener is passed the part's initial selection.
   * When the part is disposed, the listener is passed a <code>null</code> selection,
   * but only if the listener implements <code>INullSelectionListener</code>.
   * </p>
   * <p>
   * Note: This will not correctly track editor parts as each editor does
   * not have a unique partId.
   * </p>
   *
   * @param partId the id of the part to track
   * @param listener a selection listener
   * @since 2.0
   */
  virtual void AddSelectionListener(const QString& partId, ISelectionListener* listener) = 0;

  /**
   * Adds the given post selection listener.It is equivalent to selection
   * changed if the selection was triggered by the mouse but it has a
   * delay if the selection is triggered by the keyboard arrows.
   * Has no effect if an identical listener is already registered.
   *
   * Note: Works only for StructuredViewer(s).
   *
   * @param listener a selection listener
   */
  virtual void AddPostSelectionListener(ISelectionListener* listener) = 0;

  /**
   * Adds a part-specific selection listener which is notified when selection changes
   * in the part with the given id. This is independent of part activation - the part
   * need not be active for notification to be sent.
   * <p>
   * When the part is created, the listener is passed the part's initial selection.
   * When the part is disposed, the listener is passed a <code>null</code> selection,
   * but only if the listener implements <code>INullSelectionListener</code>.
   * </p>
   * <p>
   * Note: This will not correctly track editor parts as each editor does
   * not have a unique partId.
   * </p>
   *
   * @param partId the id of the part to track
   * @param listener a selection listener
   * @since 2.0
   */
  virtual void AddPostSelectionListener(const QString& partId,
                                        ISelectionListener* listener) = 0;

  /**
   * Returns the current selection in the active part.  If the selection in the
   * active part is <em>undefined</em> (the active part has no selection provider)
   * the result will be <code>null</code>.
   *
   * @return the current selection, or <code>null</code> if undefined
   */
  virtual ISelection::ConstPointer GetSelection() const = 0;

  /**
   * Returns the current selection in the part with the given id.  If the part is not open,
   * or if the selection in the active part is <em>undefined</em> (the active part has no selection provider)
   * the result will be <code>null</code>.
   *
   * @param partId the id of the part
   * @return the current selection, or <code>null</code> if undefined
   * @since 2.0
   */
  virtual ISelection::ConstPointer GetSelection(const QString& partId) = 0;

  /**
   * Removes the given selection listener.
   * Has no effect if an identical listener is not registered.
   *
   * @param listener a selection listener
   */
  virtual void RemoveSelectionListener(ISelectionListener* listener) = 0;

  /**
   * Removes the given part-specific selection listener.
   * Has no effect if an identical listener is not registered for the given part id.
   *
   * @param partId the id of the part to track
   * @param listener a selection listener
   * @since 2.0
   */
  virtual void RemoveSelectionListener(const QString& partId,
                                       ISelectionListener* listener) = 0;

  /**
   * Removes the given post selection listener.
   * Has no effect if an identical listener is not registered.
   *
   * @param listener a selection listener
   */
  virtual void RemovePostSelectionListener(ISelectionListener* listener) = 0;

  /**
   * Removes the given part-specific post selection listener.
   * Has no effect if an identical listener is not registered for the given part id.
   *
   * @param partId the id of the part to track
   * @param listener a selection listener
   * @since 2.0
   */
  virtual void RemovePostSelectionListener(const QString& partId,
                                           ISelectionListener* listener) = 0;
};

}  // namespace berry

Q_DECLARE_INTERFACE(berry::ISelectionService, "org.blueberry.ui.ISelectionService")

#endif /*BERRYISELECTIONSERVICE_H_*/
