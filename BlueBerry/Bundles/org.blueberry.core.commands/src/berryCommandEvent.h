/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYCOMMANDEVENT_H_
#define BERRYCOMMANDEVENT_H_

#include "common/berryAbstractNamedHandleEvent.h"

#include "berryCommandsDll.h"

namespace berry {

class Command;

/**
 * An instance of this class describes changes to an instance of
 * <code>Command</code>.
 * <p>
 * This class is not intended to be extended by clients.
 * </p>
 *
 * @see ICommandListener#CommandChanged(CommandEvent::Pointer)
 */
class BERRY_COMMANDS CommandEvent : public AbstractNamedHandleEvent {

public:

  berryObjectMacro(CommandEvent)

  /**
   * Creates a new instance of this class.
   *
   * @param command
   *            the instance of the interface that changed.
   * @param categoryChanged
   *            <code>true</code>, iff the category property changed.
   * @param definedChanged
   *            <code>true</code>, iff the defined property changed.
   * @param descriptionChanged
   *            <code>true</code>, iff the description property changed.
   * @param handledChanged
   *            <code>true</code>, iff the handled property changed.
   * @param nameChanged
   *            <code>true</code>, iff the name property changed.
   * @param parametersChanged
   *            <code>true</code> if the parameters have changed;
   *            <code>false</code> otherwise.
   * @param returnTypeChanged
   *            <code>true</code> iff the return type property changed;
   *            <code>false</code> otherwise.
   * @param helpContextIdChanged
   *            <code>true</code> iff the help context identifier changed;
   *            <code>false</code> otherwise.
   * @param enabledChanged
   *            <code>true</code> iff the comand enablement changed;
   *            <code>false</code> otherwise.
   * @since 3.3
   */
  CommandEvent(const SmartPointer<Command> command, bool categoryChanged,
      bool definedChanged, bool descriptionChanged,
      bool handledChanged, bool nameChanged,
      bool parametersChanged, bool returnTypeChanged = false,
      bool helpContextIdChanged = false, bool enabledChanged = false);

  /**
   * Returns the instance of the interface that changed.
   *
   * @return the instance of the interface that changed. Guaranteed not to be
   *         <code>null</code>.
   */
  SmartPointer<Command> GetCommand() const;

  /**
   * Returns whether or not the category property changed.
   *
   * @return <code>true</code>, iff the category property changed.
   */
  bool IsCategoryChanged() const;

  /**
   * Returns whether or not the handled property changed.
   *
   * @return <code>true</code>, iff the handled property changed.
   */
  bool IsHandledChanged() const;

  /**
   * Returns whether or not the help context identifier changed.
   *
   * @return <code>true</code>, iff the help context identifier changed.
   * @since 3.2
   */
  bool IsHelpContextIdChanged() const;

  /**
   * Returns whether or not the parameters have changed.
   *
   * @return <code>true</code>, iff the parameters property changed.
   */
  bool IsParametersChanged() const;

  /**
   * Returns whether or not the return type property changed.
   *
   * @return <code>true</code>, iff the return type property changed.
   * @since 3.2
   */
  bool IsReturnTypeChanged() const;

  /**
   * Return whether the enable property changed.
   *
   * @return <code>true</code> iff the comand enablement changed
   * @since 3.3
   */
  bool IsEnabledChanged() const;

private:

  /**
   * The bit used to represent whether the command has changed its category.
   */
  static const int CHANGED_CATEGORY; // = LAST_USED_BIT << 1;

  /**
   * The bit used to represent whether the command has changed its handler.
   */
  static const int CHANGED_HANDLED; // = LAST_USED_BIT << 2;

  /**
   * The bit used to represent whether the command has changed its parameters.
   */
  static const int CHANGED_PARAMETERS; // = LAST_USED_BIT << 3;

  /**
   * The bit used to represent whether the command has changed its return
   * type.
   *
   * @since 3.2
   */
  static const int CHANGED_RETURN_TYPE; // = LAST_USED_BIT << 4;

  /**
   * The bit used to represent whether the command has changed its help
   * context identifier.
   *
   * @since 3.2
   */
  static const int CHANGED_HELP_CONTEXT_ID; // = LAST_USED_BIT << 5;

  /**
   * The bit used to represent whether this commands active handler has
   * changed its enablement state.
   *
   * @since 3.3
   */
  static const int CHANGED_ENABLED; // = LAST_USED_BIT << 6;

  /**
   * The command that has changed; this value is never <code>null</code>.
   */
  const SmartPointer<Command> command;

};

}

#endif /* BERRYCOMMANDEVENT_H_ */
