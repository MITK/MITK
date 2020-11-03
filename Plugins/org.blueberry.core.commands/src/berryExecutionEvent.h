/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEXECUTIONEVENT_H_
#define BERRYEXECUTIONEVENT_H_

#include <berryMacros.h>

#include "berryCommand.h"

#include <QHash>

namespace berry {

/**
 * <p>
 * The data object to pass to the command (and its handler) as it executes. This
 * carries information about the current state of the application, and the
 * application context in which the command was executed.
 * </p>
 * <p>
 * An execution event carries three blocks of data: the parameters, the trigger,
 * and the application context. How these blocks are used is application
 * dependent. In the BlueBerry workbench, the trigger is an SWT event, and the
 * application context contains information about the selection and active part.
 * </p>
 */
class BERRY_COMMANDS ExecutionEvent : public Object {

public:

  berryObjectMacro(ExecutionEvent);

  typedef QHash<QString, QString> ParameterMap;

private:

  /**
   * The state of the application at the time the execution was triggered. In
   * the BlueBerry workbench, this might contain information about the active
   * part of the active selection (for example). This value may be
   * <code>null</code>.
   */
  const Object::Pointer applicationContext;

  /**
   * The command being executed. This value may be <code>null</code>.
   */
  const Command::ConstPointer command;

  /**
   * The parameters to qualify the execution. For handlers that normally
   * prompt for additional information, these can be used to avoid prompting.
   * This value may be empty, but it is never <code>null</code>.
   */
  const ParameterMap parameters;

  /**
   * The object that triggered the execution. In an event-driven architecture,
   * this is typically just another event. In the BlueBerry workbench, this is
   * typically an SWT event. This value may be <code>null</code>.
   */
  const Object::ConstPointer trigger;


public:

  /**
   * Constructs a new instance of <code>ExecutionEvent</code> with no
   * parameters, no trigger and no application context. This is just a
   * convenience method.
   */
  ExecutionEvent();

  /**
   * Constructs a new instance of <code>ExecutionEvent</code>.
   *
   * @param command
   *            The command being executed; may be <code>null</code>.
   * @param parameters
   *            The parameters to qualify the execution; must not be
   *            <code>null</code>. This must be a map of parameter ids (<code>String</code>)
   *            to parameter values (<code>String</code>).
   * @param trigger
   *            The object that triggered the execution; may be
   *            <code>null</code>.
   * @param applicationContext
   *            The state of the application at the time the execution was
   *            triggered; may be <code>null</code>.
   */
  ExecutionEvent(const Command::ConstPointer& command, const ParameterMap& parameters,
                 const Object::ConstPointer& trigger, const Object::Pointer& applicationContext);

  /**
   * Returns the state of the application at the time the execution was
   * triggered.
   *
   * @return The application context; may be <code>null</code>.
   */
  const Object::Pointer GetApplicationContext() const;

  /**
   * Returns the command being executed.
   *
   * @return The command being executed.
   */
  const Command::ConstPointer GetCommand() const;

  /**
   * Returns the object represented by the string value of the parameter with
   * the provided id.
   * <p>
   * This is intended to be used in the scope of an
   * {@link IHandler#execute(ExecutionEvent)} method, so any problem getting
   * the object value causes <code>ExecutionException</code> to be thrown.
   * </p>
   *
   * @param parameterId
   *            The id of a parameter to retrieve the object value of.
   * @return The object value of the parameter with the provided id.
   * @throws ExecutionException
   *             if the parameter object value could not be obtained for any
   *             reason
   */
  const Object::ConstPointer GetObjectParameterForExecution(const QString& parameterId) const;

  /**
   * Returns the value of the parameter with the given id.
   *
   * @param parameterId
   *            The id of the parameter to retrieve; may be <code>null</code>.
   * @return The parameter value; <code>null</code> if the parameter cannot
   *         be found.
   */
  QString GetParameter(const QString &parameterId) const;

  /**
   * Returns all of the parameters.
   *
   * @return The parameters; never <code>null</code>, but may be empty.
   */
  const ParameterMap& GetParameters() const;

  /**
   * Returns the object that triggered the execution
   *
   * @return The trigger; <code>null</code> if there was no trigger.
   */
  const Object::ConstPointer GetTrigger() const;

  /**
   * The string representation of this execution event -- for debugging
   * purposes only. This string should not be shown to an end user.
   */
  QString ToString() const override;
};

}

#endif /*BERRYEXECUTIONEVENT_H_*/
