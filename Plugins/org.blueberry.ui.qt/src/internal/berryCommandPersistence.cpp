/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCommandPersistence.h"

#include "berryHandleObject.h"
#include "berryStatus.h"
#include "berryICommandService.h"
#include "berryIConfigurationElement.h"
#include "berryCommandCategory.h"
#include "berryCommand.h"
#include "berryCommandParameter.h"
#include "berryParameterType.h"
#include "berryParameterValueConverterProxy.h"
#include "berryCommandStateProxy.h"
#include "berryIExtensionRegistry.h"
#include "berryIExtension.h"
#include "berryPlatform.h"
#include "berryPlatformUI.h"
#include "berryWorkbenchPlugin.h"
#include "berryIPreferencesService.h"
#include "berryIPreferences.h"
#include "berryCommandService.h"

namespace berry {

const int CommandPersistence::INDEX_CATEGORY_DEFINITIONS = 0;
const int CommandPersistence::INDEX_COMMAND_DEFINITIONS = 1;
const int CommandPersistence::INDEX_PARAMETER_TYPE_DEFINITIONS = 2;
const int CommandPersistence::INDEX_SIZE = 3;

void CommandPersistence::ReadCategoriesFromRegistry(
    const QList<SmartPointer<IConfigurationElement> >& configurationElements,
    ICommandService* const commandService)
{
  // Undefine all the previous handle objects.
  const QList<CommandCategory::Pointer> handleObjects = commandService->GetDefinedCategories();
  foreach (const CommandCategory::Pointer handleObject, handleObjects)
  {
    handleObject->Undefine();
  }

  // Define the uncategorized category.
  commandService->DefineUncategorizedCategory(
        QObject::tr("Uncategorized"),
        QObject::tr("Commands that were either auto-generated or have no category"));

  QList<IStatus::Pointer> warningsToLog;

  foreach (const IConfigurationElement::Pointer& configurationElement, configurationElements)
  {
    // Read out the category identifier.
    const QString categoryId = ReadRequired(configurationElement,
                                            ATT_ID, warningsToLog, "Categories need an id");
    if (categoryId.isNull())
    {
      continue;
    }

    // Read out the name.
    const QString name = ReadRequired(configurationElement, ATT_NAME,
                                      warningsToLog, "Categories need a name",
                                      categoryId);
    if (name.isNull())
    {
      continue;
    }

    // Read out the description.
    const QString description = ReadOptional(configurationElement, ATT_DESCRIPTION);

    const CommandCategory::Pointer category = commandService->GetCategory(categoryId);
    category->Define(name, description);
  }

  // If there were any warnings, then log them now.
  LogWarnings(warningsToLog,
              "Warnings while parsing the commands from the 'org.blueberry.ui.commands' and 'org.blueberry.ui.actionDefinitions' extension points.");
}

void CommandPersistence::ReadCommandsFromRegistry(
    const QList<SmartPointer<IConfigurationElement> >& configurationElements,
    ICommandService* const commandService)
{
  // Undefine all the previous handle objects.
  const QList<Command::Pointer> handleObjects = commandService->GetDefinedCommands();
  foreach (const Command::Pointer& handleObject, handleObjects)
  {
    handleObject->Undefine();
  }

  QList<IStatus::Pointer> warningsToLog;

  foreach (const IConfigurationElement::Pointer& configurationElement, configurationElements)
  {
    // Read out the command identifier.
    const QString commandId = ReadRequired(configurationElement, ATT_ID,
                                           warningsToLog, "Commands need an id");
    if (commandId.isNull())
    {
      continue;
    }

    // Read out the name.
    const QString name = ReadRequired(configurationElement, ATT_NAME,
                                      warningsToLog, "Commands need a name");
    if (name.isNull())
    {
      continue;
    }

    // Read out the description.
    const QString description = ReadOptional(configurationElement, ATT_DESCRIPTION);

    // Read out the category id.
    QString categoryId = configurationElement->GetAttribute(ATT_CATEGORY_ID);
    if (categoryId.isEmpty())
    {
      categoryId = configurationElement->GetAttribute(ATT_CATEGORY);
      if (categoryId.isEmpty())
      {
        categoryId = QString();
      }
    }

    // Read out the parameters.
    const QList<IParameter::Pointer> parameters = ReadParameters(configurationElement,
                                                                 warningsToLog, commandService);

    // Read out the returnTypeId.
    const QString returnTypeId = ReadOptional(configurationElement, ATT_RETURN_TYPE_ID);

    // Read out the help context identifier.
    const QString helpContextId = ReadOptional(configurationElement, ATT_HELP_CONTEXT_ID);

    const Command::Pointer command = commandService->GetCommand(commandId);
    const CommandCategory::Pointer category = commandService->GetCategory(categoryId);
    if (!category->IsDefined())
    {
      AddWarning(warningsToLog, "Commands should really have a category",
                 configurationElement, commandId, "categoryId", categoryId);
    }

    ParameterType::Pointer returnType;
    if (!returnTypeId.isNull())
    {
      returnType = commandService->GetParameterType(returnTypeId);
    }

    command->Define(name, description, category, parameters, returnType, helpContextId);
    ReadState(configurationElement, warningsToLog, command);
  }

  // If there were any warnings, then log them now.
  LogWarnings(warningsToLog,
              "Warnings while parsing the commands from the 'org.blueberry.ui.commands' and 'org.blueberry.ui.actionDefinitions' extension points.");
}

QList<SmartPointer<IParameter> > CommandPersistence::ReadParameters(
    const SmartPointer<IConfigurationElement>& configurationElement,
    QList<SmartPointer<IStatus> >& warningsToLog,
    ICommandService* const commandService)
{
  const QList<IConfigurationElement::Pointer> parameterElements = configurationElement
      ->GetChildren(TAG_COMMAND_PARAMETER);
  if (parameterElements.isEmpty())
  {
    return QList<IParameter::Pointer>();
  }

  QList<IParameter::Pointer> parameters;
  foreach (const IConfigurationElement::Pointer& parameterElement, parameterElements)
  {
    // Read out the id
    const QString id = ReadRequired(parameterElement, ATT_ID,
                                    warningsToLog, "Parameters need an id");
    if (id.isNull())
    {
      continue;
    }

    // Read out the name.
    const QString name = ReadRequired(parameterElement, ATT_NAME,
                                      warningsToLog, "Parameters need a name");
    if (name.isNull())
    {
      continue;
    }

    /*
     * The IParameterValues will be initialized lazily as an
     * IExecutableExtension.
     */

    // Read out the typeId attribute, if present.
    const QString typeId = ReadOptional(parameterElement, ATT_TYPE_ID);

    // Read out the optional attribute, if present.
    const bool optional = ReadBoolean(parameterElement, ATT_OPTIONAL, true);

    ParameterType::Pointer type;
    if (!typeId.isNull())
    {
      type = commandService->GetParameterType(typeId);
    }

    const IParameter::Pointer parameter(new CommandParameter(id, name,
                                                             parameterElement, type, optional));
    parameters.push_back(parameter);
  }

  return parameters;
}

void CommandPersistence::ReadParameterTypesFromRegistry(
    const QList<SmartPointer<IConfigurationElement> >& configurationElements,
    ICommandService* const commandService)
{

  // Undefine all the previous handle objects.
  const QList<ParameterType::Pointer> handleObjects = commandService->GetDefinedParameterTypes();
  foreach (const ParameterType::Pointer& handleObject, handleObjects)
  {
    handleObject->Undefine();
  }

  QList<IStatus::Pointer> warningsToLog;

  foreach (const IConfigurationElement::Pointer configurationElement, configurationElements)
  {
    // Read out the commandParameterType identifier.
    const QString parameterTypeId = ReadRequired(configurationElement,
                                                 ATT_ID, warningsToLog, "Command parameter types need an id");
    if (parameterTypeId.isNull())
    {
      continue;
    }

    // Read out the type.
    const QString type = ReadOptional(configurationElement, ATT_TYPE);

    // Read out the converter.
    const QString converter = ReadOptional(configurationElement, ATT_CONVERTER);

    /*
     * if the converter attribute was given, create a proxy
     * AbstractParameterValueConverter for the ParameterType, otherwise
     * null indicates there is no converter
     */
    const QSharedPointer<IParameterValueConverter> parameterValueConverter(
        (converter.isNull()) ? nullptr : new ParameterValueConverterProxy(configurationElement));

    const ParameterType::Pointer parameterType = commandService->GetParameterType(parameterTypeId);
    parameterType->Define(type, parameterValueConverter);
  }

  // If there were any warnings, then log them now.
  LogWarnings(warningsToLog,
              "Warnings while parsing the commandParameterTypes from the 'org.blueberry.ui.commands' extension point.");

}

void CommandPersistence::ReadState(
    const SmartPointer<IConfigurationElement>& configurationElement,
    QList<SmartPointer<IStatus> >& warningsToLog,
    const SmartPointer<Command>& command)
{
  const QList<IConfigurationElement::Pointer> stateElements = configurationElement
      ->GetChildren(TAG_STATE);
  if (stateElements.isEmpty())
  {
    return;
  }

  foreach (const IConfigurationElement::Pointer stateElement, stateElements)
  {
    const QString id = ReadRequired(stateElement, ATT_ID, warningsToLog, "State needs an id");
    if (id.isNull())
    {
      continue;
    }

    if (CheckClass(stateElement, warningsToLog,
                   "State must have an associated class", id))
    {
      const State::Pointer state(
            new CommandStateProxy(stateElement, ATT_CLASS,
                                  WorkbenchPlugin::GetDefault()->GetPreferences(),
                                  CommandService::CreatePreferenceKey(command, id))
            );
      command->AddState(id, state);
    }
  }
}

bool CommandPersistence::IsChangeImportant(const QList<SmartPointer<IExtension> >& /*extensions*/,
                                           RegistryChangeType /*changeType*/)
{
  return false;
}

bool CommandPersistence::IsChangeImportant(const QList<SmartPointer<IExtensionPoint> >& /*extensionPoints*/,
                                           RegistryChangeType /*changeType*/)
{
  return false;
}

void CommandPersistence::Read()
{
  RegistryPersistence::Read();
  ReRead();
}

CommandPersistence::CommandPersistence(ICommandService* const commandService)
  : commandService(commandService)
{
  if (commandService == nullptr)
  {
    throw ctkInvalidArgumentException("The command service cannot be null");
  }
}

bool CommandPersistence::CommandsNeedUpdating(const QList<SmartPointer<IExtension> >& extensions)
{
  foreach(const IExtension::Pointer& extension, extensions)
  {
    if (extension->GetExtensionPointUniqueIdentifier() == EXTENSION_COMMANDS ||
        extension->GetExtensionPointUniqueIdentifier() == EXTENSION_ACTION_DEFINITIONS)
    {
      return true;
    }
  }
  return false;
}

void CommandPersistence::ReRead()
{
  // Create the extension registry mementos.
  IExtensionRegistry* const registry = Platform::GetExtensionRegistry();
  QList<QList<IConfigurationElement::Pointer> > indexedConfigurationElements;
  for (int i = 0; i < INDEX_SIZE; ++i)
  {
    indexedConfigurationElements.push_back(QList<IConfigurationElement::Pointer>());
  }

  // Sort the commands extension point based on element name.
  QList<IConfigurationElement::Pointer> commandsExtensionPoint = registry
      ->GetConfigurationElementsFor(EXTENSION_COMMANDS);
  foreach (const IConfigurationElement::Pointer& configurationElement, commandsExtensionPoint)
  {
    const QString name = configurationElement->GetName();

    // Check if it is a binding definition.
    if (TAG_COMMAND == name)
    {
      indexedConfigurationElements[INDEX_COMMAND_DEFINITIONS].push_back(configurationElement);
    }
    else if (TAG_CATEGORY == name)
    {
      indexedConfigurationElements[INDEX_CATEGORY_DEFINITIONS].push_back(configurationElement);
    }
    else if (TAG_COMMAND_PARAMETER_TYPE == name)
    {
      indexedConfigurationElements[INDEX_PARAMETER_TYPE_DEFINITIONS].push_back(configurationElement);
    }
  }

  const QList<IConfigurationElement::Pointer> actionDefinitionsExtensionPoint = registry
      ->GetConfigurationElementsFor(EXTENSION_ACTION_DEFINITIONS);
  foreach (const IConfigurationElement::Pointer& configurationElement, actionDefinitionsExtensionPoint)
  {
    const QString name = configurationElement->GetName();

    if (TAG_ACTION_DEFINITION == name)
    {
      indexedConfigurationElements[INDEX_COMMAND_DEFINITIONS].push_back(configurationElement);
    }
  }

  ReadCategoriesFromRegistry(indexedConfigurationElements[INDEX_CATEGORY_DEFINITIONS],
                             commandService);
  ReadCommandsFromRegistry(indexedConfigurationElements[INDEX_COMMAND_DEFINITIONS],
                           commandService);
  ReadParameterTypesFromRegistry(indexedConfigurationElements[INDEX_PARAMETER_TYPE_DEFINITIONS],
                                 commandService);
}

}
