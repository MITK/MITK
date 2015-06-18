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


#ifndef BERRYSAVEABLESLIFECYCLEEVENT_H_
#define BERRYSAVEABLESLIFECYCLEEVENT_H_

#include <berryMacros.h>
#include <org_blueberry_ui_qt_Export.h>

#include "berrySaveable.h"

#include <vector>

namespace berry {

/**
 * Event object describing a change to a set of Saveable objects.
 */
class BERRY_UI_QT SaveablesLifecycleEvent : public Object
{

public:

  berryObjectMacro(SaveablesLifecycleEvent)

  /**
   * Event type constant specifying that the given saveables have been opened.
   */
  static const int POST_OPEN; // = 1;

  /**
   * Event type constant specifying that the given saveables are about to be
   * closed. Listeners may veto the closing if isForce() is false.
   */
  static const int PRE_CLOSE; // = 2;

  /**
   * Event type constant specifying that the given saveables have been closed.
   */
  static const int POST_CLOSE; // = 3;

  /**
   * Event type constant specifying that the dirty state of the given saveables
   * has changed.
   */
  static const int DIRTY_CHANGED; // = 4;


private:

  int eventType;

  QList<Saveable::Pointer> saveables;

  bool force;

  bool veto;

  Object::Pointer source;


public:

  /**
   * Creates a new SaveablesLifecycleEvent.
   *
   * @param source
   *            The source of the event. If an ISaveablesSource notifies
   *            about changes to the saveables returned by
   *            {@link ISaveablesSource#getSaveables()}, the source must be
   *            the ISaveablesSource object.
   * @param eventType
   *            the event type, currently one of POST_OPEN, PRE_CLOSE,
   *            POST_CLOSE, DIRTY_CHANGED
   * @param saveables
   *            The affected saveables
   * @param force
   *            true if the event type is PRE_CLOSE and this is a closed force
   *            that cannot be canceled.
   */
  SaveablesLifecycleEvent(Object::Pointer source_, int eventType_,
      const QList<Saveable::Pointer>& saveables_, bool force_);

  /**
   * Returns the eventType, currently one of POST_OPEN, PRE_CLOSE, POST_CLOSE,
   * DIRTY_CHANGED. Listeners should silently ignore unknown event types since
   * new event types might be added in the future.
   *
   * @return the eventType
   */
  int GetEventType();

  Object::Pointer GetSource();

  /**
   * Returns the affected saveables.
   *
   * @return the saveables
   */
  QList<Saveable::Pointer> GetSaveables();

  /**
   * Returns the veto. This value is ignored for POST_OPEN,POST_CLOSE, and
   * DIRTY_CHANGED.
   *
   * @return Returns the veto.
   */
  bool IsVeto();

  /**
   * @param veto
   *            The veto to set.
   */
  void SetVeto(bool veto);

  /**
   * Sets the force flag. This value is ignored for POST_OPEN, POST_CLOSE, and
   * DIRTY_CHANGED.
   *
   * @return Returns the force.
   */
  bool IsForce();

};

}

#endif /* BERRYSAVEABLESLIFECYCLEEVENT_H_ */
