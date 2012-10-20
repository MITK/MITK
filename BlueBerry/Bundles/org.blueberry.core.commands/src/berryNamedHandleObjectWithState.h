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

#ifndef BERRYNAMEDHANDLEOBJECTWITHSTATE_H_
#define BERRYNAMEDHANDLEOBJECTWITHSTATE_H_

#include "common/berryNamedHandleObject.h"
#include "berryIObjectWithState.h"

#include <QHash>

namespace berry {

/**
 * <p>
 * A named handle object that can carry state with it. This state can be used to
 * override the name or description.
 * </p>
 * <p>
 * Clients may neither instantiate nor extend this class.
 * </p>
 */
class BERRY_COMMANDS NamedHandleObjectWithState : public NamedHandleObject, public IObjectWithState {

public:

  berryObjectMacro(berry::NamedHandleObjectWithState)

  void AddState(const QString& stateId, const SmartPointer<State>& state);

  QString GetDescription() const;

  QString GetName() const;

  SmartPointer<State> GetState(const QString& stateId) const;

  QList<QString> GetStateIds() const;

  void RemoveState(const QString& id);

private:

  /**
   * The map of states currently held by this command. If this command has no
   * state, then this will be empty.
   */
  QHash<QString, SmartPointer<State> > states;

protected:

  /**
   * Constructs a new instance of <code>NamedHandleObject<WithState/code>.
   *
   * @param id
   *            The identifier for this handle; must not be empty.
   */
  NamedHandleObjectWithState(const QString& id);

};

}

#endif /*BERRYNAMEDHANDLEOBJECTWITHSTATE_H_*/
