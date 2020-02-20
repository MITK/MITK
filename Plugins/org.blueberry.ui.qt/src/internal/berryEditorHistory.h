/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEDITORHISTORY_H
#define BERRYEDITORHISTORY_H

#include <berrySmartPointer.h>

#include <QList>

namespace berry {

struct IEditorInput;
struct IEditorDescriptor;
struct IMemento;
struct IStatus;

class EditorHistoryItem;

/**
 * This class is used to record "open editor" actions as they
 * happen.  The input and type of each editor are recorded so that
 * the user can reopen an item from the recently used files list.
 */
class EditorHistory
{

public:

  /**
   * The maximum of entries in the history.
   */
  static const int MAX_SIZE; // = 15;

  /**
   * Constructs a new history.
   */
  EditorHistory();

  /**
   * Adds an item to the history.  Added in fifo fashion.
   */
  void Add(const SmartPointer<IEditorInput>& input, const SmartPointer<IEditorDescriptor>& desc);

  /**
   * Returns an array of editor history items.  The items are returned in order
   * of most recent first.
   */
  QList<SmartPointer<EditorHistoryItem> > GetItems();

  /**
   * Refresh the editor list.  Any stale items are removed.
   * Only restored items are considered.
   */
  void Refresh();

  /**
   * Removes the given history item.
   */
  void Remove(const SmartPointer<EditorHistoryItem>& item);

  /**
   * Removes all traces of an editor input from the history.
   */
  void Remove(const SmartPointer<IEditorInput>& input);

  /**
   * Restore the most-recently-used history from the given memento.
   *
   * @param memento the memento to restore the mru history from
   */
  SmartPointer<const IStatus> RestoreState(const SmartPointer<IMemento>& memento);

  /**
   * Save the most-recently-used history in the given memento.
   *
   * @param memento the memento to save the mru history in
   */
  SmartPointer<const IStatus> SaveState(const SmartPointer<IMemento>& memento) const;

private:

  /**
   * The list of editor entries, in FIFO order.
   */
  QList<SmartPointer<EditorHistoryItem> > fifoList;

  /**
   * Adds an item to the history.
   */
  void Add(const SmartPointer<EditorHistoryItem>& newItem, int index);

};

}
#endif // BERRYEDITORHISTORY_H
