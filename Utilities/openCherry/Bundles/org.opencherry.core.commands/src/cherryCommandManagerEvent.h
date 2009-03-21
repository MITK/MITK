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

#ifndef CHERRYCOMMANDMANAGEREVENT_H_
#define CHERRYCOMMANDMANAGEREVENT_H_

#include "cherryCommandsDll.h"

#include <cherryObject.h>
#include <cherryMacros.h>

namespace cherry
{

class CommandManager;

/**
 * <p>
 * An event indicating that the set of defined command identifiers has changed.
 * </p>
 *
 * @since 3.1
 * @see ICommandManagerListener#commandManagerChanged(CommandManagerEvent)
 */
class CHERRY_COMMANDS CommandManagerEvent: public Object
{

public:

  cherryObjectMacro(CommandManagerEvent)

  /**
   * Creates a new <code>CommandManagerEvent</code> instance to describe
   * changes to commands and/or categories.
   *
   * @param commandManager
   *            the instance of the interface that changed; must not be
   *            <code>null</code>.
   * @param commandId
   *            The command identifier that was added or removed; must not be
   *            <code>null</code> if commandIdChanged is <code>true</code>.
   * @param commandIdAdded
   *            Whether the command identifier became defined (otherwise, it
   *            became undefined).
   * @param commandIdChanged
   *            Whether the list of defined command identifiers has changed.
   * @param categoryId
   *            The category identifier that was added or removed; must not be
   *            <code>null</code> if categoryIdChanged is <code>true</code>.
   * @param categoryIdAdded
   *            Whether the category identifier became defined (otherwise, it
   *            became undefined).
   * @param categoryIdChanged
   *            Whether the list of defined category identifiers has changed.
   */
CommandManagerEvent  (CommandManager& commandManager,
      const std::string& commandId, const bool commandIdAdded,
      const bool commandIdChanged, const std::string& categoryId,
      const bool categoryIdAdded, const bool categoryIdChanged);

  /**
   * Creates a new <code>CommandManagerEvent</code> instance to describe
   * changes to command parameter types.
   *
   * @param commandManager
   *            the instance of the interface that changed; must not be
   *            <code>null</code>.
   * @param parameterTypeId
   *            The command parameter type identifier that was added or
   *            removed; must not be <code>null</code> if
   *            parameterTypeIdChanged is <code>true</code>.
   * @param parameterTypeIdAdded
   *            Whether the parameter type identifier became defined
   *            (otherwise, it became undefined).
   * @param parameterTypeIdChanged
   *            Whether the list of defined parameter type identifiers has
   *            changed.
   *
   * @since 3.2
   */
  CommandManagerEvent(CommandManager& commandManager,
      const std::string& parameterTypeId, const bool parameterTypeIdAdded,
      const bool parameterTypeIdChanged);

  /**
   * Returns the category identifier that was added or removed.
   *
   * @return The category identifier that was added or removed; may be
   *         <code>null</code>.
   */
  std::string GetCategoryId() const;

  /**
   * Returns the command identifier that was added or removed.
   *
   * @return The command identifier that was added or removed; may be
   *         <code>null</code>.
   */
  std::string GetCommandId() const;

  /**
   * Returns the instance of the interface that changed.
   *
   * @return the instance of the interface that changed. Guaranteed not to be
   *         <code>null</code>.
   */
  CommandManager& GetCommandManager() const;

  /**
   * Returns the command parameter type identifier that was added or removed.
   *
   * @return The command parameter type identifier that was added or removed;
   *         may be <code>null</code>.
   *
   * @since 3.2
   */
  std::string GetParameterTypeId() const;

  /**
   * Returns whether the list of defined category identifiers has changed.
   *
   * @return <code>true</code> if the list of category identifiers has
   *         changed; <code>false</code> otherwise.
   */
  bool IsCategoryChanged() const;

  /**
   * Returns whether the category identifier became defined. Otherwise, the
   * category identifier became undefined.
   *
   * @return <code>true</code> if the category identifier became defined;
   *         <code>false</code> if the category identifier became undefined.
   */
  bool IsCategoryDefined() const;

  /**
   * Returns whether the list of defined command identifiers has changed.
   *
   * @return <code>true</code> if the list of command identifiers has
   *         changed; <code>false</code> otherwise.
   */
  bool IsCommandChanged() const;

  /**
   * Returns whether the command identifier became defined. Otherwise, the
   * command identifier became undefined.
   *
   * @return <code>true</code> if the command identifier became defined;
   *         <code>false</code> if the command identifier became undefined.
   */
  bool IsCommandDefined() const;

  /**
   * Returns whether the list of defined command parameter type identifiers
   * has changed.
   *
   * @return <code>true</code> if the list of command parameter type
   *         identifiers has changed; <code>false</code> otherwise.
   *
   * @since 3.2
   */
  bool IsParameterTypeChanged() const;

  /**
   * Returns whether the command parameter type identifier became defined.
   * Otherwise, the command parameter type identifier became undefined.
   *
   * @return <code>true</code> if the command parameter type identifier
   *         became defined; <code>false</code> if the command parameter
   *         type identifier became undefined.
   *
   * @since 3.2
   */
  bool IsParameterTypeDefined() const;

private:

  /**
   * The bit used to represent whether the given category has become defined.
   * If this bit is not set and there is no category id, then no category has
   * become defined nor undefined. If this bit is not set and there is a
   * category id, then the category has become undefined.
   */
  static const int CHANGED_CATEGORY_DEFINED; // = 1;

  /**
   * The bit used to represent whether the given command has become defined.
   * If this bit is not set and there is no command id, then no command has
   * become defined nor undefined. If this bit is not set and there is a
   * command id, then the command has become undefined.
   */
  static const int CHANGED_COMMAND_DEFINED; // = 1 << 1;

  /**
   * The bit used to represent whether the given command parameter type has
   * become defined. If this bit is not set and there is no parameter type id,
   * then no parameter type has become defined nor undefined. If this bit is
   * not set and there is a parameter type id, then the parameter type has
   * become undefined.
   *
   * @since 3.2
   */
  static const int CHANGED_PARAMETER_TYPE_DEFINED; // = 1 << 2;

  /**
   * The category identifier that was added or removed from the list of
   * defined category identifiers. This value is <code>null</code> if the
   * list of defined category identifiers did not change.
   */
  const std::string categoryId;

  /**
   * A collection of bits representing whether certain values have changed. A
   * bit is set (i.e., <code>1</code>) if the corresponding property has
   * changed.
   */
  int changedValues;

  /**
   * The command identifier that was added or removed from the list of defined
   * command identifiers. This value is <code>null</code> if the list of
   * defined command identifiers did not change.
   */
  const std::string commandId;

  /**
   * The command parameter type identifier that was added or removed from the
   * list of defined parameter type identifiers. This value is
   * <code>null</code> if the list of defined parameter type identifiers did
   * not change.
   */
  const std::string parameterTypeId;

  /**
   * The command manager that has changed.
   */
  CommandManager& commandManager;

};

}

#endif /* CHERRYCOMMANDMANAGEREVENT_H_ */
