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


#ifndef BERRYIEXECUTIONLISTENER_H_
#define BERRYIEXECUTIONLISTENER_H_

#include <berryObject.h>
#include <berryMacros.h>
#include <berryMessage.h>

#include <org_blueberry_core_commands_Export.h>
#include "common/berryCommandExceptions.h"

namespace berry {

class ExecutionEvent;

/**
 * <p>
 * A listener to the execution of commands. This listener will be notified if a
 * command is about to execute, and when that execution completes. It is not
 * possible for the listener to prevent the execution, only to respond to it in
 * some way.
 * </p>
 *
 * @since 3.1
 */
struct BERRY_COMMANDS IExecutionListener : public virtual Object {

  berryInterfaceMacro(IExecutionListener, berry);

  struct Events {

    Message2<const std::string&, const NotHandledException*> notHandled;
    Message2<const std::string&, const ExecutionException*> postExecuteFailure;
    Message2<const std::string&, Object::Pointer> postExecuteSuccess;
    Message2<const std::string&, const SmartPointer<const ExecutionEvent> > preExecute;

  virtual ~Events();

    virtual void AddListener(IExecutionListener::Pointer listener);
    virtual void RemoveListener(IExecutionListener::Pointer listener);
    virtual bool HasListeners() const;
    virtual bool IsEmpty() const;

    typedef MessageDelegate2<IExecutionListener, const std::string&, const NotHandledException* > NotHandledDelegate;
    typedef MessageDelegate2<IExecutionListener, const std::string&, const ExecutionException*> PostExecuteFailureDelegate;
    typedef MessageDelegate2<IExecutionListener, const std::string&, Object::Pointer> PostExecuteSuccessDelegate;
    typedef MessageDelegate2<IExecutionListener, const std::string&, const SmartPointer<const ExecutionEvent> > PreExecuteDelegate;
  };

  virtual ~IExecutionListener();

  /**
   * Notifies the listener that an attempt was made to execute a command with
   * no handler.
   *
   * @param commandId
   *            The identifier of command that is not handled; never
   *            <code>null</code>
   * @param exception
   *            The exception that occurred; never <code>null</code>.
   */
  virtual void NotHandled(const std::string& commandId, const NotHandledException* exception) = 0;

  /**
   * Notifies the listener that a command has failed to complete execution.
   *
   * @param commandId
   *            The identifier of the command that has executed; never
   *            <code>null</code>.
   * @param exception
   *            The exception that occurred; never <code>null</code>.
   */
  virtual void PostExecuteFailure(const std::string& commandId,
      const ExecutionException* exception) = 0;

  /**
   * Notifies the listener that a command has completed execution
   * successfully.
   *
   * @param commandId
   *            The identifier of the command that has executed; never
   *            <code>null</code>.
   * @param returnValue
   *            The return value from the command; may be <code>null</code>.
   */
  virtual void PostExecuteSuccess(const std::string& commandId, const Object::Pointer returnValue) = 0;

  /**
   * Notifies the listener that a command is about to execute.
   *
   * @param commandId
   *            The identifier of the command that is about to execute, never
   *            <code>null</code>.
   * @param event
   *            The event that will be passed to the <code>execute</code>
   *            method; never <code>null</code>.
   */
  virtual void PreExecute(const std::string& commandId, const SmartPointer<const ExecutionEvent> event) = 0;
};

}

#endif /* BERRYIEXECUTIONLISTENER_H_ */
