/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  berryObjectMacro(berry::NamedHandleObjectWithState);

  void AddState(const QString& stateId, const SmartPointer<State>& state) override;

  QString GetDescription() const override;

  QString GetName() const override;

  SmartPointer<State> GetState(const QString& stateId) const override;

  QList<QString> GetStateIds() const override;

  void RemoveState(const QString& id) override;

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
