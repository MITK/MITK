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


#ifndef BERRYCOMMANDPERSISTENCE_H_
#define BERRYCOMMANDPERSISTENCE_H_

#include "berryRegistryPersistence.h"

#include <berryParameter>

namespace berry {

/**
 * <p>
 * A static class for accessing the registry and the preference store.
 * </p>
 *
 * @since 3.1
 */
class CommandPersistence : public RegistryPersistence {

private:

  /**
   * The index of the category elements in the indexed array.
   *
   * @see CommandPersistence#read()
   */
  static const int INDEX_CATEGORY_DEFINITIONS; // = 0;

  /**
   * The index of the command elements in the indexed array.
   *
   * @see CommandPersistence#read()
   */
  static const int INDEX_COMMAND_DEFINITIONS; // = 1;

  /**
   * The index of the commandParameterType elements in the indexed array.
   *
   * @see CommandPersistence#read()
   * @since 3.2
   */
  static const int INDEX_PARAMETER_TYPE_DEFINITIONS; // = 2;

  /**
   * Reads all of the category definitions from the commands extension point.
   *
   * @param configurationElements
   *            The configuration elements in the commands extension point;
   *            must not be <code>null</code>, but may be empty.
   * @param commandService
   *            The command service to which the categories should be added;
   *            must not be <code>null</code>.
   */
  static void ReadCategoriesFromRegistry(
      const std::vector<IConfigurationElement::Pointer>& configurationElements,
      const ICommandService::Pointer commandService);

  /**
   * Reads all of the command definitions from the commands extension point.
   *
   * @param configurationElements
   *            The configuration elements in the commands extension point;
   *            must not be <code>null</code>, but may be empty.
   * @param configurationElementCount
   *            The number of configuration elements that are really in the
   *            array.
   * @param commandService
   *            The command service to which the commands should be added;
   *            must not be <code>null</code>.
   */
  static void ReadCommandsFromRegistry(
      const std::vector<IConfigurationElement::Pointer>& configurationElements,
      const ICommandService::Pointer commandService);

  /**
   * Reads the parameters from a parent configuration element. This is used to
   * read the parameter sub-elements from a command element. Each parameter is
   * guaranteed to be valid. If invalid parameters are found, then a warning
   * status will be appended to the <code>warningsToLog</code> list.
   *
   * @param configurationElement
   *            The configuration element from which the parameters should be
   *            read; must not be <code>null</code>.
   * @param warningsToLog
   *            The list of warnings found during parsing. Warnings found
   *            while parsing the parameters will be appended to this list.
   *            This value must not be <code>null</code>.
   * @param commandService
   *            The command service from which the parameter can get parameter
   *            types; must not be <code>null</code>.
   * @return The array of parameters found for this configuration element;
   *         <code>null</code> if none can be found.
   */
  static std::vector<IParameter::Pointer> ReadParameters(
      const IConfigurationElement::Pointer configurationElement,
      std::vector<IStatus::Pointer>& warningsToLog,
      const ICommandService::Pointer const commandService);

  /**
   * Reads all of the commandParameterType definitions from the commands
   * extension point.
   *
   * @param configurationElements
   *            The configuration elements in the commands extension point;
   *            must not be <code>null</code>, but may be empty.
   * @param configurationElementCount
   *            The number of configuration elements that are really in the
   *            array.
   * @param commandService
   *            The command service to which the commands should be added;
   *            must not be <code>null</code>.
   * @since 3.2
   */
  static void ReadParameterTypesFromRegistry(
      const std::vector<IConfigurationElement::Pointer>& configurationElements,
      const ICommandService::Pointer commandService);

  /**
   * Reads the states from a parent configuration element. This is used to
   * read the state sub-elements from a command element. Each state is
   * guaranteed to be valid. If invalid states are found, then a warning
   * status will be appended to the <code>warningsToLog</code> list.
   *
   * @param configurationElement
   *            The configuration element from which the states should be
   *            read; must not be <code>null</code>.
   * @param warningsToLog
   *            The list of warnings found during parsing. Warnings found
   *            while parsing the parameters will be appended to this list.
   *            This value must not be <code>null</code>.
   * @param command
   *            The command for which the state is being read; may be
   *            <code>null</code>.
   */
  static void ReadState(
      const IConfigurationElement::Pointer configurationElement,
      std::vector<IStatus::Pointer>& warningsToLog,
      const Command::Pointer command);

  /**
   * The command service with which this persistence class is associated;
   * never <code>null</code>.
   */
  const ICommandService::Pointer commandService;

protected:

//  bool IsChangeImportant(final IRegistryChangeEvent event) {
//    return false;
//  }

  /**
   * Reads all of the commands and categories from the registry,
   *
   * @param commandService
   *            The command service which should be populated with the values
   *            from the registry; must not be <code>null</code>.
   */
  void Read();


public:

  /**
   * Constructs a new instance of <code>CommandPersistence</code>.
   *
   * @param commandService
   *            The command service which should be populated with the values
   *            from the registry; must not be <code>null</code>.
   */
  CommandPersistence(const ICommandService::Pointer commandService);

//  bool CommandsNeedUpdating(final IRegistryChangeEvent event) {
//    final IExtensionDelta[] commandDeltas = event.getExtensionDeltas(
//        PlatformUI.PLUGIN_ID, IWorkbenchRegistryConstants.PL_COMMANDS);
//    if (commandDeltas.length == 0) {
//      final IExtensionDelta[] actionDefinitionDeltas = event
//          .getExtensionDeltas(PlatformUI.PLUGIN_ID,
//              IWorkbenchRegistryConstants.PL_ACTION_DEFINITIONS);
//      if (actionDefinitionDeltas.length == 0) {
//        return false;
//      }
//    }
//
//    return true;
//  }

  void ReRead();

};

}

#endif /* BERRYCOMMANDPERSISTENCE_H_ */
