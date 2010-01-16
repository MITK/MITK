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

#include "berryRegistryPersistence.h"
#include "berryPlatformUI.h"
#include "berryWorkbenchPlugin.h"

#include <berryParameterization.h>
#include <berryElementHandler.h>
#include <berryExpressionConverter.h>
#include <berryMultiStatus.h>

#include <Poco/String.h>

namespace berry {

class ErrorExpression : public Expression
  {
  public:
    EvaluationResult Evaluate(IEvaluationContext::Pointer )
    {
      // return anything
      return EvaluationResult::FALSE_EVAL;
    }
  };

Expression::Pointer RegistryPersistence::ERROR_EXPRESSION(new ErrorExpression());


 void RegistryPersistence::Dispose() {
    //IExtensionRegistry* registry = Platform::GetExtensionRegistry();
    //TODO Registry listener
    //registry.removeRegistryChangeListener(registryChangeListener);
    registryListenerAttached = false;
  }

   void RegistryPersistence::AddWarning(std::vector<IStatus::Pointer>& warningsToLog,
      const std::string& message, const IConfigurationElement::ConstPointer element,
      const std::string& id, const std::string& extraAttributeName,
      const std::string& extraAttributeValue) {
    std::string statusMessage(message);
    if (element) {
      statusMessage += ": plug-in='" + element->GetContributor() + "'";
    }
    if (!id.empty()) {
      if (element) {
        statusMessage += ',';
      } else {
        statusMessage += ':';
      }
      statusMessage += " id='" + id + "'";
    }
    if (!extraAttributeName.empty()) {
      if (element || !id.empty()) {
        statusMessage += ',';
      } else {
        statusMessage += ':';
      }
      statusMessage += " " + extraAttributeName + "='" + extraAttributeValue + "'";
    }

    IStatus::Pointer status(new Status(IStatus::WARNING,
        PlatformUI::PLUGIN_ID, 0, statusMessage));
    warningsToLog.push_back(status);
  }

   bool RegistryPersistence::CheckClass(
      const IConfigurationElement::ConstPointer configurationElement,
      std::vector<IStatus::Pointer>& warningsToLog,
      const std::string& message,
      const std::string& id) {
    // Check to see if we have a handler class.
     std::string dummy;
    if ((configurationElement->GetAttribute(ATT_CLASS, dummy) == false)
        && (configurationElement->GetChildren(TAG_CLASS).empty())) {
      AddWarning(warningsToLog, message, configurationElement, id);
      return false;
    }

    return true;
  }

   bool RegistryPersistence::IsPulldown(
      const IConfigurationElement::ConstPointer element) {
    std::string style;
    ReadOptional(element, ATT_STYLE, style);
    const bool pulldown = ReadBoolean(element, ATT_PULLDOWN, false);
    return (pulldown || STYLE_PULLDOWN == style);
  }

   void RegistryPersistence::LogWarnings(const std::vector<IStatus::Pointer>& warningsToLog,
      const std::string& message) {
    // If there were any warnings, then log them now.
    if (!warningsToLog.empty()) {
      const IStatus::Pointer status(new MultiStatus(
          PlatformUI::PLUGIN_ID, 0, warningsToLog, message));
      WorkbenchPlugin::Log(status);
    }
  }

   bool RegistryPersistence::ReadBoolean(
     const IConfigurationElement::ConstPointer configurationElement,
     const std::string& attribute,
     bool defaultValue) {
     std::string value;
    const bool hasValue(configurationElement->GetAttribute(attribute, value));
    if (!hasValue) {
      return defaultValue;
    }

    if (defaultValue) {
      return Poco::icompare(value, "false") != 0;
    }

    return Poco::icompare(value, "true") == 0; //$NON-NLS-1$
  }

   bool RegistryPersistence::ReadOptional(
      const IConfigurationElement::ConstPointer configurationElement,
      const std::string& attribute, std::string& value) {
    bool hasValue = configurationElement->GetAttribute(attribute,  value);
    if (value.empty()) {
      hasValue = false;
    }

    return hasValue;
  }

   ParameterizedCommand::Pointer RegistryPersistence::ReadParameterizedCommand(
      const IConfigurationElement::Pointer configurationElement,
      ICommandService::Pointer commandService,
      std::vector<IStatus::Pointer>& warningsToLog,
      const std::string& message,
      const std::string& id) {
    std::string commandId;
    if (!ReadRequired(configurationElement,
        ATT_COMMAND_ID, commandId, warningsToLog, message, id)) {
      return ParameterizedCommand::Pointer(0);
    }

    const Command::Pointer command(commandService->GetCommand(commandId));
    const ParameterizedCommand::Pointer parameterizedCommand(ReadParameters(
        configurationElement, warningsToLog, command));

    return parameterizedCommand;
  }

   ParameterizedCommand::Pointer RegistryPersistence::ReadParameters(
      const IConfigurationElement::Pointer configurationElement,
      std::vector<IStatus::Pointer>& warningsToLog,
      Command::Pointer command) {
    const std::vector<IConfigurationElement::Pointer> parameterElements(configurationElement
        ->GetChildren(TAG_PARAMETER));
    if (parameterElements.empty()) {
      ParameterizedCommand::Pointer cmd(new ParameterizedCommand(command, std::vector<Parameterization>()));
      return cmd;
    }

    std::vector<Parameterization> parameters;
    for (unsigned int i = 0; i < parameterElements.size(); i++) {
      const IConfigurationElement::Pointer parameterElement(parameterElements[i]);

      // Read out the id.
      std::string id;
      parameterElement->GetAttribute(ATT_ID, id);
      if (id.empty()) {
        // The name should never be null. This is invalid.
        AddWarning(warningsToLog, "Parameters need a name", //$NON-NLS-1$
            configurationElement);
        continue;
      }

      // Find the parameter on the command.
      IParameter::Pointer parameter;
      try {
        const std::vector<IParameter::Pointer> commandParameters(command->GetParameters());
        if (!parameters.empty()) {
          for (unsigned int j = 0; j < commandParameters.size(); j++) {
            const IParameter::Pointer currentParameter(commandParameters[j]);
            if (currentParameter->GetId() == id) {
              parameter = currentParameter;
              break;
            }
          }

        }
      } catch (const NotDefinedException& e) {
        // This should not happen.
      }
      if (!parameter) {
        // The name should never be null. This is invalid.
        AddWarning(warningsToLog,
            "Could not find a matching parameter", //$NON-NLS-1$
            configurationElement, id);
        continue;
      }

      // Read out the value.
      std::string value;
      parameterElement->GetAttribute(ATT_VALUE, value);
      if (value.empty()) {
        // The name should never be null. This is invalid.
        AddWarning(warningsToLog, "Parameters need a value", //$NON-NLS-1$
            configurationElement, id);
        continue;
      }

      parameters.push_back(Parameterization(parameter, value));
    }

    if (parameters.empty()) {
      ParameterizedCommand::Pointer cmd(new ParameterizedCommand(command, std::vector<Parameterization>()));
      return cmd;
    }

    ParameterizedCommand::Pointer cmd(new ParameterizedCommand(command, parameters));
    return cmd;
  }

   bool RegistryPersistence::ReadRequired(
      const IConfigurationElement::ConstPointer configurationElement,
      const std::string& attribute,
      std::string& value,
      std::vector<IStatus::Pointer>& warningsToLog,
      const std::string& message,
      const std::string& id) {
    bool hasValue = configurationElement->GetAttribute(attribute, value);
    if (value.empty()) {
      AddWarning(warningsToLog, message, configurationElement, id);
      return false;
    }

    return hasValue;
  }

  Expression::Pointer RegistryPersistence::ReadWhenElement(
      const IConfigurationElement::Pointer parentElement,
      const std::string& whenElementName,
      const std::string& id,
      std::vector<IStatus::Pointer>& warningsToLog) {
    // Check to see if we have an when expression.
    const std::vector<IConfigurationElement::Pointer> whenElements(parentElement
        ->GetChildren(whenElementName));
    Expression::Pointer whenExpression;
    if (!whenElements.empty()) {
      // Check if we have too many when elements.
      if (whenElements.size() > 1) {
        // There should only be one when element
        AddWarning(warningsToLog,
            "There should only be one when element", parentElement, //$NON-NLS-1$
            id, "whenElementName", //$NON-NLS-1$
            whenElementName);
        return ERROR_EXPRESSION;
      }

      const IConfigurationElement::Pointer whenElement(whenElements[0]);
      const std::vector<IConfigurationElement::Pointer> expressionElements(whenElement
          ->GetChildren());
      if (!expressionElements.empty()) {
        // Check if we have too many expression elements
        if (expressionElements.size() > 1) {
          // There should only be one expression element
          AddWarning(
              warningsToLog,
              "There should only be one expression element", parentElement, //$NON-NLS-1$
              id, "whenElementName", //$NON-NLS-1$
              whenElementName);
          return ERROR_EXPRESSION;
        }

        // Convert the activeWhen element into an expression.
        const ElementHandler::Pointer elementHandler(ElementHandler::GetDefault());
        ExpressionConverter * const converter = ExpressionConverter::GetDefault();
        const IConfigurationElement::Pointer expressionElement(expressionElements[0]);
        try {
          whenExpression = elementHandler->Create(converter,
              expressionElement);
        } catch (const CoreException& e) {
          // There when expression could not be created.
          AddWarning(
              warningsToLog,
              "Problem creating when element", //$NON-NLS-1$
              parentElement, id,
              "whenElementName", whenElementName); //$NON-NLS-1$
          return ERROR_EXPRESSION;
        }
      }
    }

    return whenExpression;
  }

  RegistryPersistence::RegistryPersistence()
  : registryListenerAttached(false)
  {
    //TODO registry listener
//    registryChangeListener = new IRegistryChangeListener() {
//      public final void registryChanged(final IRegistryChangeEvent event) {
//        if (isChangeImportant(event)) {
//          Display.getDefault().asyncExec(new Runnable() {
//            public final void run() {
//              read();
//            }
//          });
//        }
//      }
//    };
  }

   void RegistryPersistence::Read() {
     //TODO registry listener
//    if (!registryListenerAttached) {
//      final IExtensionRegistry registry = Platform.getExtensionRegistry();
//      registry.addRegistryChangeListener(registryChangeListener);
//      registryListenerAttached = true;
//    }
  }

}
