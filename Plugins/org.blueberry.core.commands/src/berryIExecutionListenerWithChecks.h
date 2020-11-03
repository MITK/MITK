/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIEXECUTIONLISTENERWITHCHECKS_H_
#define BERRYIEXECUTIONLISTENERWITHCHECKS_H_

#include "berryIExecutionListener.h"
#include "common/berryCommandExceptions.h"

namespace berry {

/**
 * <p>
 * A listener to the execution of commands. This listener will be notified if
 * someone tries to execute a command and it is not enabled or not defined. The
 * listener also be notified if a command is about to execute, and when that
 * execution completes. It is not possible for the listener to prevent the
 * execution, only to respond to it in some way.
 * </p>
 * <p>
 * Clients may implement.
 * </p>
 */
struct BERRY_COMMANDS IExecutionListenerWithChecks : public IExecutionListener {

  struct Events : public IExecutionListener::Events {

    Message2<const QString&, const NotDefinedException*> notDefined;
    Message2<const QString&, const NotEnabledException*> notEnabled;

    void AddListener(IExecutionListener* listener) override;
    void RemoveListener(IExecutionListener* listener) override;
    bool HasListeners() const override;
    bool IsEmpty() const override;

    private:

    typedef MessageDelegate2<IExecutionListenerWithChecks, const QString&, const NotDefinedException* > NotDefinedDelegate;
    typedef MessageDelegate2<IExecutionListenerWithChecks, const QString&, const NotEnabledException*> NotEnabledDelegate;
  };

  /**
   * Notifies the listener that an attempt was made to execute a command that
   * is not defined.
   *
   * @param commandId
   *            The identifier of command that is not defined; never
   *            <code>null</code>
   * @param exception
   *            The exception that occurred; never <code>null</code>.
   */
  virtual void NotDefined(const QString& commandId, const NotDefinedException* exception) = 0;

  /**
   * Notifies the listener that an attempt was made to execute a command that
   * is disabled.
   *
   * @param commandId
   *            The identifier of command that is not enabled; never
   *            <code>null</code>
   * @param exception
   *            The exception that occurred; never <code>null</code>.
   */
  virtual void NotEnabled(const QString& commandId, const NotEnabledException* exception) = 0;
};

}

#endif /* BERRYIEXECUTIONLISTENERWITHCHECKS_H_ */
