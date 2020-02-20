/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYCOMMANDPERSISTENCE_H
#define BERRYCOMMANDPERSISTENCE_H

#include "berryRegistryPersistence.h"

namespace berry {

struct IParameter;

/**
 * <p>
 * A static class for accessing the registry and the preference store.
 * </p>
 */
class CommandPersistence : public RegistryPersistence
{

private:

  friend class CommandService;

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
   */
  static const int INDEX_PARAMETER_TYPE_DEFINITIONS; // = 2;

  static const int INDEX_SIZE;

  /**
   * Reads all of the category definitions from the commands extension point.
   *
   * @param configurationElements
   *            The configuration elements in the commands extension point;
   *            must not be <code>null</code>, but may be empty.
   * @param configurationElementCount
   *            The number of configuration elements that are really in the
   *            array.
   * @param commandService
   *            The command service to which the categories should be added;
   *            must not be <code>null</code>.
   */
  static void ReadCategoriesFromRegistry(
      const QList<SmartPointer<IConfigurationElement> >& configurationElements,
      ICommandService* const commandService);

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
      const QList<SmartPointer<IConfigurationElement> >& configurationElements,
      ICommandService* const commandService);

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
  static QList<SmartPointer<IParameter> > ReadParameters(
      const SmartPointer<IConfigurationElement>& configurationElement,
      QList<SmartPointer<IStatus> >& warningsToLog,
      ICommandService* const commandService);

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
   */
  static void ReadParameterTypesFromRegistry(
      const QList<SmartPointer<IConfigurationElement> >& configurationElements,
      ICommandService* const commandService);

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
      const SmartPointer<IConfigurationElement>& configurationElement,
      QList<SmartPointer<IStatus> >& warningsToLog,
      const SmartPointer<Command>& command);

  /**
   * The command service with which this persistence class is associated;
   * never <code>null</code>.
   */
  ICommandService* const commandService;

protected:

  bool IsChangeImportant(const QList<SmartPointer<IExtension> >& extensions,
                         RegistryChangeType changeType) override;

  bool IsChangeImportant(const QList<SmartPointer<IExtensionPoint> >& extensionPoints,
                         RegistryChangeType changeType) override;

  /**
   * Reads all of the commands and categories from the registry,
   *
   * @param commandService
   *            The command service which should be populated with the values
   *            from the registry; must not be <code>null</code>.
   */
  void Read() override;

public:

  /**
   * Constructs a new instance of <code>CommandPersistence</code>.
   *
   * @param commandService
   *            The command service which should be populated with the values
   *            from the registry; must not be <code>null</code>.
   */
  CommandPersistence(ICommandService* const commandService);

  bool CommandsNeedUpdating(const QList<SmartPointer<IExtension> >& extensions);

  void ReRead();
};

}

#endif // BERRYCOMMANDPERSISTENCE_H
