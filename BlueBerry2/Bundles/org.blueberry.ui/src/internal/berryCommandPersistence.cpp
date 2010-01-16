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

#include "berryCommandPersistence.h"
#include <berryCommandCategory.h>
#include <berryCommand.h>

#include <berryPlatform.h>
#include <berryIExtensionPointService.h>

#include "berryParameter.h"
#include "berryParameterValueConverterProxy.h"

namespace berry {

 const int CommandPersistence::INDEX_CATEGORY_DEFINITIONS = 0;
const int CommandPersistence::INDEX_COMMAND_DEFINITIONS = 1;
 const int CommandPersistence::INDEX_PARAMETER_TYPE_DEFINITIONS = 2;

   void CommandPersistence::ReadCategoriesFromRegistry(
      const std::vector<IConfigurationElement::Pointer>& configurationElements,
      const ICommandService::Pointer commandService) {
    // Undefine all the previous handle objects.
    std::vector<CommandCategory::Pointer> handleObjects(commandService
        ->GetDefinedCategories());

      for (unsigned int i = 0; i < handleObjects.size(); i++) {
        handleObjects[i]->Undefine();
      }

    // Define the uncategorized category.
    commandService
        ->DefineUncategorizedCategory(
            "Uncategorized",
            "Commands that were either auto-generated or have no category");

    std::vector<IStatus::Pointer> warningsToLog;

    for (unsigned int i = 0; i < configurationElements.size(); i++) {
      const IConfigurationElement::Pointer configurationElement(configurationElements[i]);

      // Read out the category identifier.
      std::string categoryId;
      bool result = ReadRequired(configurationElement,
          ATT_ID, categoryId, warningsToLog, "Categories need an id"); //$NON-NLS-1$
      if (!result) {
        continue;
      }

      // Read out the name.
      std::string name;
      result = ReadRequired(configurationElement, ATT_NAME, name,
          warningsToLog, "Categories need a name", categoryId);
      if (!result) {
        continue;
      }

      // Read out the description.
      std::string description;
      ReadOptional(configurationElement, ATT_DESCRIPTION, description);

      const CommandCategory::Pointer category(commandService->GetCategory(categoryId));
      category->Define(name, description);
    }

    // If there were any warnings, then log them now.
    LogWarnings(
        warningsToLog,
        "Warnings while parsing the commands from the 'org.eclipse.ui.commands' and 'org.eclipse.ui.actionDefinitions' extension points."); //$NON-NLS-1$
  }

  void CommandPersistence::ReadCommandsFromRegistry(
      const std::vector<IConfigurationElement::Pointer>& configurationElements,
      const ICommandService::Pointer commandService) {
    // Undefine all the previous handle objects.
    std::vector<Command::Pointer> handleObjects(commandService->GetDefinedCommands());
    for (unsigned int i = 0; i < handleObjects.size(); i++) {
        handleObjects[i]->Undefine();
      }

    std::vector<IStatus::Pointer> warningsToLog;

    for (unsigned int i = 0; i < configurationElements.size(); i++) {
      const IConfigurationElement::Pointer configurationElement(configurationElements[i]);

      // Read out the command identifier.
      std::string commandId;
      bool result = ReadRequired(configurationElement, ATT_ID, commandId,
          warningsToLog, "Commands need an id");
      if (!result) {
        continue;
      }

      // Read out the name.
      std::string name;
      result = ReadRequired(configurationElement, ATT_NAME, name,
          warningsToLog, "Commands need a name");
      if (!result) {
        continue;
      }

      // Read out the description.
      std::string description;
      ReadOptional(configurationElement, ATT_DESCRIPTION, description);

      // Read out the category id.
      std::string categoryId;
      configurationElement->GetAttribute(ATT_CATEGORY_ID, categoryId);
      if (categoryId.empty()) {
        configurationElement->GetAttribute(ATT_CATEGORY, categoryId);
      }

      // Read out the parameters.
      std::vector<IParameter::Pointer> parameters(ReadParameters(configurationElement,
          warningsToLog, commandService));

      // Read out the returnTypeId.
      std::string returnTypeId;
      ReadOptional(configurationElement, ATT_RETURN_TYPE_ID, returnTypeId);

      // Read out the help context identifier.
      std::string helpContextId;
      ReadOptional(configurationElement, ATT_HELP_CONTEXT_ID, helpContextId);

      const Command::Pointer command(commandService->GetCommand(commandId));
      const CommandCategory::Pointer category(commandService->GetCategory(categoryId));
      if (!category->IsDefined()) {
        AddWarning(
            warningsToLog,
            "Commands should really have a category", //$NON-NLS-1$
            configurationElement, commandId,
            "categoryId", categoryId); //$NON-NLS-1$
      }

      const ParameterType::Pointer returnType(commandService->GetParameterType(returnTypeId));

      command->Define(name, description, category, parameters, returnType, helpContextId);
      ReadState(configurationElement, warningsToLog, command);
    }

    // If there were any warnings, then log them now.
    LogWarnings(
        warningsToLog,
        "Warnings while parsing the commands from the 'org.blueberry.ui.commands' and 'org.blueberry.ui.actionDefinitions' extension points.");
  }

   std::vector<IParameter::Pointer> CommandPersistence::ReadParameters(
      const IConfigurationElement::Pointer configurationElement,
      std::vector<IStatus::Pointer>& warningsToLog,
      const ICommandService::Pointer commandService) {
    std::vector<IConfigurationElement::Pointer> parameterElements(configurationElement
        ->GetChildren(TAG_COMMAND_PARAMETER));
    if (parameterElements.empty()) {
      return std::vector<IParameter::Pointer>();
    }

    std::vector<IParameter::Pointer> parameters;
    for (unsigned int i = 0; i < parameterElements.size(); i++) {
      const IConfigurationElement::Pointer parameterElement(parameterElements[i]);
      // Read out the id
      std::string id;
      bool result = ReadRequired(parameterElement, ATT_ID, id,
          warningsToLog, "Parameters need an id");
      if (!result) {
        continue;
      }

      // Read out the name.
      std::string name;
      result = ReadRequired(parameterElement, ATT_NAME, name,
          warningsToLog, "Parameters need a name");
      if (!result) {
        continue;
      }

      /*
       * The IParameterValues will be initialized lazily as an
       * IExecutableExtension.
       */

      // Read out the typeId attribute, if present.
      std::string typeId;
      bool typeIdResult = ReadOptional(parameterElement, ATT_TYPE_ID, typeId);

      // Read out the optional attribute, if present.
      const bool optional(ReadBoolean(parameterElement,
          ATT_OPTIONAL, true));

      ParameterType::Pointer type;
      if (typeIdResult) {
        type = commandService->GetParameterType(typeId);
      }

      const IParameter::Pointer parameter(new Parameter(id, name,
          parameterElement, type, optional));
      parameters.push_back(parameter);
    }

    return parameters;
  }

  void CommandPersistence::ReadParameterTypesFromRegistry(
      const std::vector<IConfigurationElement::Pointer>& configurationElements,
      const ICommandService::Pointer commandService) {

    // Undefine all the previous handle objects.
    const std::vector<ParameterType::Pointer> handleObjects(commandService
        ->GetDefinedParameterTypes());
      for (unsigned int i = 0; i < handleObjects.size(); i++) {
        handleObjects[i]->Undefine();
      }

    std::vector<IStatus::Pointer> warningsToLog;

    for (unsigned int i = 0; i < configurationElements.size(); i++) {
      const IConfigurationElement::Pointer configurationElement(configurationElements[i]);

      // Read out the commandParameterType identifier.
      std::string parameterTypeId;
      bool result = ReadRequired(configurationElement,
          ATT_ID, parameterTypeId, warningsToLog, "Command parameter types need an id");
      if (!result) {
        continue;
      }

      // Read out the type.
      std::string type;
      ReadOptional(configurationElement, ATT_TYPE, type);

      // Read out the converter.
      std::string converter;
      bool hasConverter = ReadOptional(configurationElement, ATT_CONVERTER, converter);

      /*
       * if the converter attribute was given, create a proxy
       * AbstractParameterValueConverter for the ParameterType, otherwise
       * null indicates there is no converter
       */
      IParameterValueConverter::Pointer parameterValueConverter(hasConverter ? 0 : new ParameterValueConverterProxy(configurationElement));

      const ParameterType::Pointer parameterType(commandService
          ->GetParameterType(parameterTypeId));
      parameterType->Define(type, parameterValueConverter);
    }

    // If there were any warnings, then log them now.
    LogWarnings(
        warningsToLog,
        "Warnings while parsing the commandParameterTypes from the 'org.blueberry.ui.commands' extension point.");

  }

   void CommandPersistence::ReadState(
      const IConfigurationElement::Pointer configurationElement,
      std::vector<IStatus::Pointer>& warningsToLog,
      const Command::Pointer command) {
    std::vector<IConfigurationElement::Pointer> stateElements(configurationElement
        ->GetChildren(TAG_STATE));
    if (stateElements.empty()) {
      return;
    }

    for (unsigned int i = 0; i < stateElements.size(); i++) {
      const IConfigurationElement::Pointer stateElement(stateElements[i]);

      std::string id;
      bool result = ReadRequired(stateElement, ATT_ID, id, warningsToLog,
          "State needs an id"); //$NON-NLS-1$
      if (!result) {
        continue;
      }

      if (CheckClass(stateElement, warningsToLog,
          "State must have an associated class", id)) {
        State::Pointer state(new CommandStateProxy(stateElement,
            ATT_CLASS, PrefUtil.getInternalPreferenceStore(),
            CommandService.createPreferenceKey(command, id)));
        command->AddState(id, state);
      }
    }
  }


  void CommandPersistence::Read() {
    RegistryPersistence::Read();
    this->ReRead();
  }


  CommandPersistence::CommandPersistence(const ICommandService::Pointer commandService)
  : commandService(commandService)
  {
    if (!commandService) {
      throw Poco::NullPointerException("The command service cannot be null");
    }
  }

  void CommandPersistence::ReRead() {
    // Create the extension registry mementos.
    const IExtensionPointService::Pointer registry(Platform::GetExtensionPointService());

    std::vector<std::vector<IConfigurationElement::Pointer> > indexedConfigurationElements;
    indexedConfigurationElements.push_back(std::vector<IConfigurationElement::Pointer>());
    indexedConfigurationElements.push_back(std::vector<IConfigurationElement::Pointer>());
    indexedConfigurationElements.push_back(std::vector<IConfigurationElement::Pointer>());


    // Sort the commands extension point based on element name.
    std::vector<IConfigurationElement::Pointer> commandsExtensionPoint(registry
        ->GetConfigurationElementsFor(EXTENSION_COMMANDS));
    for (unsigned int i = 0; i < commandsExtensionPoint.size(); i++)
    {
      const IConfigurationElement::Pointer configurationElement(commandsExtensionPoint[i]);
      const std::string name(configurationElement->GetName());

      // Check if it is a binding definition.
      if (TAG_COMMAND == name) {
        indexedConfigurationElements[INDEX_COMMAND_DEFINITIONS].push_back(configurationElement);
      } else if (TAG_CATEGORY == name) {
        indexedConfigurationElements[INDEX_CATEGORY_DEFINITIONS].push_back(configurationElement);
      } else if (TAG_COMMAND_PARAMETER_TYPE == name) {
        indexedConfigurationElements[INDEX_PARAMETER_TYPE_DEFINITIONS].push_back(configurationElement);
      }
    }

    std::vector<IConfigurationElement::Pointer> actionDefinitionsExtensionPoint(registry
        ->GetConfigurationElementsFor(EXTENSION_ACTION_DEFINITIONS));
    for (unsigned int i = 0; i < actionDefinitionsExtensionPoint.size(); i++) {
      const IConfigurationElement::Pointer configurationElement(actionDefinitionsExtensionPoint[i]);
      const std::string name(configurationElement->GetName());

      if (TAG_ACTION_DEFINITION == name) {
        indexedConfigurationElements[INDEX_COMMAND_DEFINITIONS].push_back(configurationElement);
      }
    }

    ReadCategoriesFromRegistry(
        indexedConfigurationElements[INDEX_CATEGORY_DEFINITIONS], commandService);
    ReadCommandsFromRegistry(
        indexedConfigurationElements[INDEX_COMMAND_DEFINITIONS], commandService);
    ReadParameterTypesFromRegistry(
        indexedConfigurationElements[INDEX_PARAMETER_TYPE_DEFINITIONS], commandService);
  }

}
