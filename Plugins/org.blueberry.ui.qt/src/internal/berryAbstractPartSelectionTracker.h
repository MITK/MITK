/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYABSTRACTPARTSELECTIONTRACKER_H_
#define BERRYABSTRACTPARTSELECTIONTRACKER_H_

#include <berryObject.h>
#include <berryMacros.h>

#include "berryISelectionService.h"
#include "berryISelectionListener.h"
#include "berryIWorkbenchPart.h"


namespace berry {

class AbstractPartSelectionTracker : public virtual Object
{

public:
  berryObjectMacro(AbstractPartSelectionTracker);

private:

  /**
   * List of selection listeners for this tracker
   */
  QList<ISelectionListener*> fListeners;

  /**
   * List of post selection listeners for this tracker
   */
  QList<ISelectionListener*> fPostListeners;

  /**
   * The id of the part this tracker tracks
   */
  QString fPartId;

public:

  /**
   * Constructs a part selection tracker for the part with the given id.
   *
   * @param id part identifier
   */
  AbstractPartSelectionTracker(const QString& partId);

  /**
   * Adds a selection listener to this tracker
   *
   * @param listener the listener to add
   */
  void AddSelectionListener(ISelectionListener* listener);

  /**
   * Adds a post selection listener to this tracker
   *
   * @param listener the listener to add
   */
  void AddPostSelectionListener(ISelectionListener* listener);

  /**
   * Returns the selection from the part being tracked,
   * or <code>null</code> if the part is closed or has no selection.
   */
  virtual ISelection::ConstPointer GetSelection() = 0;

  /**
   * Removes a selection listener from this tracker.
   *
   * @param listener the listener to remove
   */
  void RemoveSelectionListener(ISelectionListener* listener);

  /**
   * Removes a post selection listener from this tracker.
   *
   * @param listener the listener to remove
   */
  void RemovePostSelectionListener(ISelectionListener* listener);

  /**
   * Disposes this selection tracker. This removes all listeners currently registered.
   */
  ~AbstractPartSelectionTracker() override;

protected:

  /**
   * Fires a selection event to the listeners.
   *
   * @param part the part or <code>null</code> if no active part
   * @param sel the selection or <code>null</code> if no active selection
   * @param listeners the list of listeners to notify
   */
  void FireSelection(const IWorkbenchPart::Pointer& part, const ISelection::ConstPointer& sel);

  /**
   * Fires a post selection event to the listeners.
   *
   * @param part the part or <code>null</code> if no active part
   * @param sel the selection or <code>null</code> if no active selection
   * @param listeners the list of listeners to notify
   */
  void FirePostSelection(const IWorkbenchPart::Pointer& part, const ISelection::ConstPointer& sel);

  /**
   * Returns the id of the part that this tracks.
   *
   * @return part identifier
   */
  QString GetPartId();


private:

  /**
   * Sets the id of the part that this tracks.
   *
   * @param id view identifier
   */
  void SetPartId(const QString& partId);

};

}

#endif /* BERRYABSTRACTPARTSELECTIONTRACKER_H_ */
