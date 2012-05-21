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
  berryObjectMacro(AbstractPartSelectionTracker)

private:

  /**
   * List of selection listeners for this tracker
   */
  QList<ISelectionListener::Pointer> fListeners;

  /**
   * List of post selection listeners for this tracker
   */
  QList<ISelectionListener::Pointer> fPostListeners;

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
  void AddSelectionListener(ISelectionListener::Pointer listener);

  /**
   * Adds a post selection listener to this tracker
   *
   * @param listener the listener to add
   */
  void AddPostSelectionListener(ISelectionListener::Pointer listener);

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
  void RemoveSelectionListener(ISelectionListener::Pointer listener);

  /**
   * Removes a post selection listener from this tracker.
   *
   * @param listener the listener to remove
   */
  void RemovePostSelectionListener(ISelectionListener::Pointer listener);

  /**
   * Disposes this selection tracker. This removes all listeners currently registered.
   */
  ~AbstractPartSelectionTracker();

protected:

  /**
   * Fires a selection event to the listeners.
   *
   * @param part the part or <code>null</code> if no active part
   * @param sel the selection or <code>null</code> if no active selection
   * @param listeners the list of listeners to notify
   */
  void FireSelection(IWorkbenchPart::Pointer part, ISelection::ConstPointer sel);

  /**
   * Fires a post selection event to the listeners.
   *
   * @param part the part or <code>null</code> if no active part
   * @param sel the selection or <code>null</code> if no active selection
   * @param listeners the list of listeners to notify
   */
  void FirePostSelection(IWorkbenchPart::Pointer part, ISelection::ConstPointer sel);

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
