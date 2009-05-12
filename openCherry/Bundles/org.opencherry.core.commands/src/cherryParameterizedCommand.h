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

#ifndef CHERRYPARAMETERIZEDCOMMAND_H_
#define CHERRYPARAMETERIZEDCOMMAND_H_

#include <cherryObject.h>
#include <cherryMacros.h>

#include "common/cherryCommandExceptions.h"
#include "cherryCommandsDll.h"

#include <list>
#include <map>

namespace cherry
{

struct IParameter;
class Command;
class Parameterization;

/**
 * <p>
 * A command that has had one or more of its parameters specified. This class
 * serves as a utility class for developers that need to manipulate commands
 * with parameters. It handles the behaviour of generating a parameter map and a
 * human-readable name.
 * </p>
 *
 * @since 3.1
 */
class CHERRY_COMMANDS ParameterizedCommand: public Object
{ //implements Comparable {

public:

  cherryObjectMacro(ParameterizedCommand)

  /**
   * The index of the parameter id in the parameter values.
   *
   * @deprecated no longer used
   */
  static  const int INDEX_PARAMETER_ID; // = 0;

  /**
   * The index of the human-readable name of the parameter itself, in the
   * parameter values.
   *
   * @deprecated no longer used
   */
  static const int INDEX_PARAMETER_NAME; // = 1;

  /**
   * The index of the human-readable name of the value of the parameter for
   * this command.
   *
   * @deprecated no longer used
   */
  static const int INDEX_PARAMETER_VALUE_NAME; // = 2;

  /**
   * The index of the value of the parameter that the command can understand.
   *
   * @deprecated no longer used
   */
  static const int INDEX_PARAMETER_VALUE_VALUE; // = 3;

  /**
   * Constructs a new instance of <code>ParameterizedCommand</code> with
   * specific values for zero or more of its parameters.
   *
   * @param command
   *            The command that is parameterized; must not be
   *            <code>null</code>.
   * @param parameterizations
   *            An array of parameterizations binding parameters to values for
   *            the command. This value may be <code>null</code>.
   */
  ParameterizedCommand(const SmartPointer<Command> command,
      const std::vector<Parameterization>& parameterizations);

  bool operator<(const Object* object) const;

  /*
   * (non-Javadoc)
   *
   * @see java.lang.Object#equals(java.lang.Object)
   */
  bool operator==(const Object* object) const;

  /**
   * Executes this command with its parameters. This does extra checking to
   * see if the command is enabled and defined. If it is not both enabled and
   * defined, then the execution listeners will be notified and an exception
   * thrown.
   *
   * @param trigger
   *            The object that triggered the execution; may be
   *            <code>null</code>.
   * @param applicationContext
   *            The state of the application at the time the execution was
   *            triggered; may be <code>null</code>.
   * @return The result of the execution; may be <code>null</code>.
   * @throws ExecutionException
   *             If the handler has problems executing this command.
   * @throws NotDefinedException
   *             If the command you are trying to execute is not defined.
   * @throws NotEnabledException
   *             If the command you are trying to execute is not enabled.
   * @throws NotHandledException
   *             If there is no handler.
   * @since 3.2
   */
  Object::Pointer ExecuteWithChecks(const Object::ConstPointer trigger,
      const Object::ConstPointer applicationContext) throw(ExecutionException,
      NotDefinedException, NotEnabledException, NotHandledException);

  /**
   * Returns the base command. It is possible for more than one parameterized
   * command to have the same identifier.
   *
   * @return The command; never <code>null</code>, but may be undefined.
   */
  SmartPointer<Command> GetCommand() const;

  /**
   * Returns the command's base identifier. It is possible for more than one
   * parameterized command to have the same identifier.
   *
   * @return The command id; never <code>null</code>.
   */
  std::string GetId() const;

  /**
   * Returns a human-readable representation of this command with all of its
   * parameterizations.
   *
   * @return The human-readable representation of this parameterized command;
   *         never <code>null</code>.
   * @throws NotDefinedException
   *             If the underlying command is not defined.
   */
  std::string GetName() const throw(NotDefinedException);

  /**
   * Returns the parameter map, as can be used to construct an
   * <code>ExecutionEvent</code>.
   *
   * @return The map of parameter ids (<code>String</code>) to parameter
   *         values (<code>String</code>). This map is never
   *         <code>null</code>, but may be empty.
   */
  std::map<std::string, std::string> GetParameterMap() const;

  /*
   * (non-Javadoc)
   *
   * @see java.lang.Object#hashCode()
   */
  std::size_t HashCode() const;

  /**
   * Returns a {@link String} containing the command id, parameter ids and
   * parameter values for this {@link ParameterizedCommand}. The returned
   * {@link String} can be stored by a client and later used to reconstruct an
   * equivalent {@link ParameterizedCommand} using the
   * {@link CommandManager#deserialize(String)} method.
   * <p>
   * The syntax of the returned {@link String} is as follows:
   * </p>
   *
   * <blockquote>
   * <code>serialization = <u>commandId</u> [ '(' parameters ')' ]</code><br>
   * <code>parameters = parameter [ ',' parameters ]</code><br>
   * <code>parameter = <u>parameterId</u> [ '=' <u>parameterValue</u> ]</code>
   * </blockquote>
   *
   * <p>
   * In the syntax above, sections inside square-brackets are optional. The
   * characters in single quotes (<code>(</code>, <code>)</code>,
   * <code>,</code> and <code>=</code>) indicate literal characters.
   * </p>
   * <p>
   * <code><u>commandId</u></code> represents the command id encoded with
   * separator characters escaped. <code><u>parameterId</u></code> and
   * <code><u>parameterValue</u></code> represent the parameter ids and
   * values encoded with separator characters escaped. The separator
   * characters <code>(</code>, <code>)</code>, <code>,</code> and
   * <code>=</code> are escaped by prepending a <code>%</code>. This
   * requires <code>%</code> to be escaped, which is also done by prepending
   * a <code>%</code>.
   * </p>
   * <p>
   * The order of the parameters is not defined (and not important). A missing
   * <code><u>parameterValue</u></code> indicates that the value of the
   * parameter is <code>null</code>.
   * </p>
   * <p>
   * For example, the string shown below represents a serialized parameterized
   * command that can be used to show the Resource perspective:
   * </p>
   * <p>
   * <code>org.eclipse.ui.perspectives.showPerspective(org.eclipse.ui.perspectives.showPerspective.perspectiveId=org.eclipse.ui.resourcePerspective)</code>
   * </p>
   * <p>
   * This example shows the more general form with multiple parameters,
   * <code>null</code> value parameters, and escaped <code>=</code> in the
   * third parameter value.
   * </p>
   * <p>
   * <code>command.id(param1.id=value1,param2.id,param3.id=esc%=val3)</code>
   * </p>
   *
   * @return A string containing the escaped command id, parameter ids and
   *         parameter values; never <code>null</code>.
   * @see CommandManager#deserialize(String)
   * @since 3.2
   */
  std::string Serialize();

  std::string ToString() const;

  /**
   * <p>
   * Generates all the possible combinations of command parameterizations for
   * the given command. If the command has no parameters, then this is simply
   * a parameterized version of that command. If a parameter is optional, both
   * the included and not included cases are considered.
   * </p>
   * <p>
   * If one of the parameters cannot be loaded due to a
   * <code>ParameterValuesException</code>, then it is simply ignored.
   * </p>
   *
   * @param command
   *            The command for which the parameter combinations should be
   *            generated; must not be <code>null</code>.
   * @return A collection of <code>ParameterizedCommand</code> instances
   *         representing all of the possible combinations. This value is
   *         never empty and it is never <code>null</code>.
   * @throws NotDefinedException
   *             If the command is not defined.
   */
  static std::vector<ParameterizedCommand::Pointer>
  GenerateCombinations(const SmartPointer<Command> command)
  throw(NotDefinedException);

  /**
   * Take a command and a map of parameter IDs to values, and generate the
   * appropriate parameterized command.
   *
   * @param command
   *            The command object. Must not be <code>null</code>.
   * @param parameters
   *            A map of String parameter ids to objects. May be
   *            <code>null</code>.
   * @return the parameterized command, or <code>null</code> if it could not
   *         be generated
   * @since 3.4
   */
  static ParameterizedCommand::Pointer GenerateCommand(const SmartPointer<Command> command,
      const std::map<std::string, Object::Pointer>& parameters);

private:

  /**
   * The constant integer hash code value meaning the hash code has not yet
   * been computed.
   */
  static const std::size_t HASH_CODE_NOT_COMPUTED; // = 0;

  /**
   * A factor for computing the hash code for all parameterized commands.
   */
  static const std::size_t HASH_FACTOR; // = 89;

  /**
   * The seed for the hash code for all parameterized commands.
   */
  static const std::size_t HASH_INITIAL;

  /**
   * Escapes special characters in the command id, parameter ids and parameter
   * values for {@link #serialize()}. The special characters
   * {@link CommandManager#PARAMETER_START_CHAR},
   * {@link CommandManager#PARAMETER_END_CHAR},
   * {@link CommandManager#ID_VALUE_CHAR},
   * {@link CommandManager#PARAMETER_SEPARATOR_CHAR} and
   * {@link CommandManager#ESCAPE_CHAR} are escaped by prepending a
   * {@link CommandManager#ESCAPE_CHAR} character.
   *
   * @param rawText
   *            a <code>String</code> to escape special characters in for
   *            serialization.
   * @return a <code>String</code> representing <code>rawText</code> with
   *         special serialization characters escaped
   * @since 3.2
   */
  static std::string Escape(const std::string& rawText);

  /**
   * Generates every possible combination of parameter values for the given
   * parameters. Parameters values that cannot be initialized are just
   * ignored. Optional parameters are considered.
   *
   * @param startIndex
   *            The index in the <code>parameters</code> that we should
   *            process. This must be a valid index.
   * @param parameters
   *            The parameters in to process; must not be <code>null</code>.
   * @return A collection (<code>Collection</code>) of combinations (<code>List</code>
   *         of <code>Parameterization</code>).
   */
  static std::vector<std::list<Parameterization> > ExpandParameters(unsigned int startIndex,
      const std::vector<SmartPointer<IParameter> >& parameters);

  /**
   * The base command which is being parameterized. This value is never
   * <code>null</code>.
   */
  const SmartPointer<Command> command;

  /**
   * The hash code for this object. This value is computed lazily, and marked
   * as invalid when one of the values on which it is based changes.
   */
  mutable std::size_t hashCode;

  /**
   * This is an array of parameterization defined for this command. This value
   * may be <code>null</code> if the command has no parameters.
   */
  std::vector<Parameterization> parameterizations;

  mutable std::string name;

};

}

#endif /* CHERRYPARAMETERIZEDCOMMAND_H_ */
