/*=========================================================================

Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef CHERRYEXECUTIONEVENT_H_
#define CHERRYEXECUTIONEVENT_H_

#include <cherryMacros.h>

#include "cherryCommand.h"

#include <map>

namespace cherry {

/**
 * <p>
 * The data object to pass to the command (and its handler) as it executes. This
 * carries information about the current state of the application, and the
 * application context in which the command was executed.
 * </p>
 * <p>
 * An execution event carries three blocks of data: the parameters, the trigger,
 * and the application context. How these blocks are used is application
 * dependent. In the openCherry workbench, the trigger is an SWT event, and the
 * application context contains information about the selection and active part.
 * </p>
 *
 * @since 3.1
 */
class CHERRY_COMMANDS ExecutionEvent : public Object {

public:

  cherryObjectMacro(ExecutionEvent)

  typedef std::map<std::string, std::string> ParameterMap;

private:

  /**
   * The state of the application at the time the execution was triggered. In
   * the openCherry workbench, this might contain information about the active
   * part of the active selection (for example). This value may be
   * <code>null</code>.
   */
  const Object::ConstPointer applicationContext;

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
   * this is typically just another event. In the openCherry workbench, this is
   * typically an SWT event. This value may be <code>null</code>.
   */
  const Object::ConstPointer trigger;


public:

  /**
   * Constructs a new instance of <code>ExecutionEvent</code> with no
   * parameters, no trigger and no application context. This is just a
   * convenience method.
   *
   * @since 3.2
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
   * @since 3.2
   */
  ExecutionEvent(const Command::ConstPointer command, const ParameterMap& parameters,
      const Object::ConstPointer trigger, const Object::ConstPointer applicationContext);

  /**
   * Returns the state of the application at the time the execution was
   * triggered.
   *
   * @return The application context; may be <code>null</code>.
   */
  const Object::ConstPointer GetApplicationContext() const;

  /**
   * Returns the command being executed.
   *
   * @return The command being executed.
   * @since 3.2
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
   * @since 3.2
   */
  const Object::ConstPointer GetObjectParameterForExecution(const std::string& parameterId) const;

  /**
   * Returns the value of the parameter with the given id.
   *
   * @param parameterId
   *            The id of the parameter to retrieve; may be <code>null</code>.
   * @return The parameter value; <code>null</code> if the parameter cannot
   *         be found.
   */
  std::string GetParameter(const std::string parameterId) const;

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
  std::string ToString() const;
};

}

#endif /*CHERRYEXECUTIONEVENT_H_*/
