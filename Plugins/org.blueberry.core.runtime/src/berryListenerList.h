/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYLISTENERLIST_H
#define BERRYLISTENERLIST_H

#include <QMutex>
#include <QList>
#include <QReadWriteLock>

namespace berry {

struct ListenerListEquals
{
  template<class T>
  bool operator()(const T& l1, const T& l2) const
  {
    return l1 == l2;
  }
};

/**
 * This class is a thread safe list that is designed for storing lists of listeners.
 * The implementation is optimized for minimal memory footprint, frequent reads
 * and infrequent writes.  Modification of the list is synchronized and relatively
 * expensive, while accessing the listeners is very fast.  Readers are given access
 * to the underlying array data structure for reading, with the trust that they will
 * not modify the underlying array.
 * <p>
 * A listener list handles the <i>same</i> listener being added
 * multiple times, and tolerates removal of listeners that are the same as other
 * listeners in the list.  For this purpose, listeners can be compared with each other
 * using a custom comparison functor.
 * </p>
 * <p>
 * Use the <code>GetListeners</code> method when notifying listeners. The recommended
 * code sequence for notifying all registered listeners of say,
 * <code>FooListener::EventHappened()</code>, is:
 *
 * \code
 * QList<FooListener*> listeners = myListenerList.GetListeners();
 * for (int i = 0; i &lt; listeners.size(); ++i)
 * {
 *   listeners[i]->EventHappened(event);
 * }
 * \endcode
 * </p>
 */
template<class T, typename C = ListenerListEquals>
class ListenerList
{

public:

  /**
   * Adds a listener to this list. This method has no effect if the <a href="ListenerList.html#same">same</a>
   * listener is already registered.
   *
   * @param listener the non-<code>null</code> listener to add
   */
  void Add(const T& listener)
  {
    // This method is synchronized to protect against multiple threads adding
    // or removing listeners concurrently.

    // check for duplicates
    QWriteLocker l(&mutex);
    const int oldSize = listeners.size();
    for (int i = 0; i < oldSize; ++i)
    {
      T& listener2 = listeners[i];
      if (comparator(listener, listener2))
        return;
    }

    listeners.push_back(listener);
  }

  /**
   * Returns an array containing all the registered listeners.
   * The resulting array is unaffected by subsequent adds or removes.
   * If there are no listeners registered, the result is an empty array.
   * Use this method when notifying listeners, so that any modifications
   * to the listener list during the notification will have no effect on
   * the notification itself.
   *
   * \warning Callers of this method <b>must not</b> modify the returned array.
   *
   * @return the list of registered listeners
   */
  QList<T> GetListeners() const
  {
    QReadLocker l(&mutex);
    return listeners;
  }

  /**
   * Returns whether this listener list is empty.
   *
   * @return <code>true</code> if there are no registered listeners, and
   *   <code>false</code> otherwise
   */
  bool IsEmpty() const
  {
    QReadLocker l(&mutex);
    return listeners.empty();
  }

  /**
   * Removes a listener from this list. Has no effect if the same
   * listener was not already registered.
   *
   * @param listener the listener to remove
   */
  void Remove(const T &listener)
  {
    QWriteLocker l(&mutex);
    // This method is synchronized to protect against multiple threads adding
    // or removing listeners concurrently.
    const int oldSize = listeners.size();
    for (int i = 0; i < oldSize; ++i)
    {
      T& listener2 = listeners[i];
      if (comparator(listener, listener2))
      {
        listeners.removeAt(i);
        return;
      }
    }
  }

  /**
   * Returns the number of registered listeners.
   *
   * @return the number of registered listeners
   */
  int Size() const
  {
    QReadLocker l(&mutex);
    return listeners.size();
  }

  /**
   * Removes all listeners from this list.
   */
  void Clear()
  {
    QWriteLocker l(&mutex);
    listeners.clear();
  }

private:

  /**
   * The list of listeners.
   */
  QList<T> listeners;

  mutable QReadWriteLock mutex;

  C comparator;
};

}

#endif // BERRYLISTENERLIST_H
