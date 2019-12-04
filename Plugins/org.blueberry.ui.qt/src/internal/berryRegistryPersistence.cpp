/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryRegistryPersistence.h"

#include "berryExpression.h"
#include "berryIConfigurationElement.h"
#include "berryIContributor.h"
#include "berryStatus.h"
#include "berryMultiStatus.h"
#include "berryParameterizedCommand.h"
#include "berryParameterization.h"
#include "berryICommandService.h"
#include "berryCommand.h"
#include "berryElementHandler.h"
#include "berryExpressionConverter.h"
#include "berryIRegistryEventListener.h"

#include "berryPlatformUI.h"

#include "berryWorkbenchPlugin.h"

namespace {

class ErrorExpression : public berry::Expression
{
public:
  berry::EvaluationResult::ConstPointer Evaluate(berry::IEvaluationContext* /*context*/) const override
  {
    return berry::EvaluationResult::ConstPointer(nullptr);
  }
};

}

namespace berry {

const SmartPointer<Expression> RegistryPersistence::ERROR_EXPRESSION(new ErrorExpression());

class RegistryPersistence::ReadRunnable : public Poco::Runnable
{
public:
  ReadRunnable(berry::RegistryPersistence* registryPersistence)
    : registryPersistence(registryPersistence)
  {}

  void run() override
  {
    registryPersistence->Read();
  }

private:
  berry::RegistryPersistence* registryPersistence;
};

void RegistryPersistence::AddWarning(QList<SmartPointer<IStatus> >& warningsToLog,
                       const QString& message,
                       const SmartPointer<IConfigurationElement>& element)
{
  AddWarning(warningsToLog, message, element, QString(), QString(), QString());
}

void RegistryPersistence::AddWarning(QList<SmartPointer<IStatus> >& warningsToLog,
                       const QString& message,
                       const SmartPointer<IConfigurationElement>& element,
                       const QString& id)
{
  AddWarning(warningsToLog, message, element, id, QString(), QString());
}

void RegistryPersistence::AddWarning(QList<SmartPointer<IStatus> >& warningsToLog,
                       const QString& message,
                       const SmartPointer<IConfigurationElement>& element,
                       const QString& id,
                       const QString& extraAttributeName,
                       const QString& extraAttributeValue)
{
  QString statusMessage = message;
  if (element.IsNotNull())
  {
    statusMessage += ": plug-in='" + element->GetContributor()->GetName() + '\'';
  }
  if (!id.isNull())
  {
    if (element.IsNotNull())
    {
      statusMessage += ',';
    }
    else
    {
      statusMessage += ':';
    }
    statusMessage += " id='" + id + '\'';
  }
  if (!extraAttributeName.isNull())
  {
    if ((element.IsNotNull()) || (!id.isNull()))
    {
      statusMessage += ',';
    }
    else
    {
      statusMessage += ':';
    }
    statusMessage += ' ' + extraAttributeName + "='" + extraAttributeValue + '\'';
  }

  IStatus::Pointer status(new Status(IStatus::WARNING_TYPE, PlatformUI::PLUGIN_ID(), 0,
                                     statusMessage, BERRY_STATUS_LOC));
  warningsToLog.push_back(status);
}

bool RegistryPersistence::CheckClass(const SmartPointer<IConfigurationElement>& configurationElement,
                                     QList<SmartPointer<IStatus> >& warningsToLog,
                                     const QString& message,
                                     const QString& id)
{
  // Check to see if we have a handler class.
  if ((configurationElement->GetAttribute(ATT_CLASS).isNull())
      && (configurationElement->GetChildren(TAG_CLASS).isEmpty()))
  {
    AddWarning(warningsToLog, message, configurationElement, id);
    return false;
  }

  return true;
}

bool RegistryPersistence::IsPulldown(const SmartPointer<IConfigurationElement>& element)
{
  const QString style = ReadOptional(element, ATT_STYLE);
  const bool pulldown = ReadBoolean(element, ATT_PULLDOWN, false);
  return (pulldown || STYLE_PULLDOWN == style);
}

void RegistryPersistence::LogWarnings(QList<SmartPointer<IStatus> >& warningsToLog,
                        const QString& message)
{
  // If there were any warnings, then log them now.
  if (!warningsToLog.isEmpty())
  {
    IStatus::Pointer status(new MultiStatus(PlatformUI::PLUGIN_ID(), 0, warningsToLog,
                                            message, BERRY_STATUS_LOC));
    WorkbenchPlugin::Log(status);
  }
}

bool RegistryPersistence::ReadBoolean(const SmartPointer<IConfigurationElement>& configurationElement,
                        const QString& attribute,
                        const bool defaultValue)
{
  const QString value = configurationElement->GetAttribute(attribute);
  if (value.isNull())
  {
    return defaultValue;
  }

  if (defaultValue)
  {
    return value.compare("false", Qt::CaseInsensitive) != 0;
  }

  return value.compare("true", Qt::CaseInsensitive) == 0;
}

QString RegistryPersistence::ReadOptional(const SmartPointer<IConfigurationElement>& configurationElement,
                            const QString& attribute)
{
  QString value = configurationElement->GetAttribute(attribute);
  if (value.isEmpty())
  {
    value = QString();
  }

  return value;
}

SmartPointer<ParameterizedCommand> RegistryPersistence::ReadParameterizedCommand(
    const SmartPointer<IConfigurationElement>& configurationElement,
    ICommandService* const commandService,
    QList<SmartPointer<IStatus> >& warningsToLog,
    const QString& message, const QString& id)
{
  const QString commandId = ReadRequired(configurationElement,
                                         ATT_COMMAND_ID, warningsToLog, message, id);
  if (commandId.isNull())
  {
    return ParameterizedCommand::Pointer(nullptr);
  }

  const Command::Pointer command = commandService->GetCommand(commandId);
  const ParameterizedCommand::Pointer parameterizedCommand = ReadParameters(
        configurationElement, warningsToLog, command);

  return parameterizedCommand;
}

SmartPointer<ParameterizedCommand> RegistryPersistence::ReadParameters(
    const SmartPointer<IConfigurationElement>& configurationElement,
    QList<SmartPointer<IStatus> >& warningsToLog,
    const SmartPointer<Command>& command)
{
  const QList<IConfigurationElement::Pointer> parameterElements = configurationElement
      ->GetChildren(TAG_PARAMETER);
  if (parameterElements.isEmpty())
  {
    ParameterizedCommand::Pointer result(new ParameterizedCommand(command, QList<Parameterization>()));
    return result;
  }

  QList<Parameterization> parameters;
  foreach (const IConfigurationElement::Pointer& parameterElement, parameterElements)
  {
    // Read out the id.
    const QString id = parameterElement->GetAttribute(ATT_ID);
    if (id.isEmpty())
    {
      // The id should never be null. This is invalid.
      AddWarning(warningsToLog, "Parameters need an id", configurationElement);
      continue;
    }

    // Find the parameter on the command.
    IParameter::Pointer parameter;
    try
    {
      const QList<IParameter::Pointer> commandParameters = command->GetParameters();
      foreach (const IParameter::Pointer& currentParameter, commandParameters)
      {
        if (currentParameter->GetId() == id)
        {
          parameter = currentParameter;
          break;
        }
      }
    }
    catch (const NotDefinedException& /*e*/)
    {
      // This should not happen.
    }
    if (parameter.IsNull())
    {
      // The name should never be null. This is invalid.
      AddWarning(warningsToLog, "Could not find a matching parameter",
                 configurationElement, id);
      continue;
    }

    // Read out the value.
    const QString value = parameterElement->GetAttribute(ATT_VALUE);
    if (value.isEmpty())
    {
      // The name should never be null. This is invalid.
      AddWarning(warningsToLog, "Parameters need a value",
                 configurationElement, id);
      continue;
    }

    parameters.push_back(Parameterization(parameter, value));
  }

  if (parameters.isEmpty())
  {
    ParameterizedCommand::Pointer result(new ParameterizedCommand(command, QList<Parameterization>()));
    return result;
  }

  ParameterizedCommand::Pointer result(new ParameterizedCommand(command, parameters));
  return result;
}

QString RegistryPersistence::ReadRequired(const SmartPointer<IConfigurationElement>& configurationElement,
                            const QString& attribute,
                            QList<SmartPointer<IStatus> >& warningsToLog,
                            const QString& message)
{
  return ReadRequired(configurationElement, attribute, warningsToLog, message, QString());
}

QString RegistryPersistence::ReadRequired(const SmartPointer<IConfigurationElement>& configurationElement,
                            const QString& attribute,
                            QList<SmartPointer<IStatus> >& warningsToLog,
                            const QString& message, const QString& id)
{
  const QString value = configurationElement->GetAttribute(attribute);
  if (value.isEmpty())
  {
    AddWarning(warningsToLog, message, configurationElement, id);
    return QString();
  }

  return value;
}

SmartPointer<Expression> RegistryPersistence::ReadWhenElement(
    const SmartPointer<IConfigurationElement>& parentElement,
    const QString& whenElementName, const QString& id,
    QList<SmartPointer<IStatus> >& warningsToLog)
{
  // Check to see if we have an when expression.
  const QList<IConfigurationElement::Pointer> whenElements = parentElement
      ->GetChildren(whenElementName);
  Expression::Pointer whenExpression;
  if (!whenElements.isEmpty())
  {
    // Check if we have too many when elements.
    if (whenElements.size() > 1)
    {
      // There should only be one when element
      AddWarning(warningsToLog,
                 "There should only be one when element", parentElement,
                 id, "whenElementName", whenElementName);
      return ERROR_EXPRESSION;
    }

    const IConfigurationElement::Pointer whenElement = whenElements.front();
    const QList<IConfigurationElement::Pointer> expressionElements = whenElement->GetChildren();
    if (!expressionElements.isEmpty())
    {
      // Check if we have too many expression elements
      if (expressionElements.size() > 1)
      {
        // There should only be one expression element
        AddWarning(warningsToLog, "There should only be one expression element", parentElement,
                   id, "whenElementName", whenElementName);
        return ERROR_EXPRESSION;
      }

      // Convert the activeWhen element into an expression.
      const ElementHandler::Pointer elementHandler = ElementHandler::GetDefault();
      ExpressionConverter* const converter = ExpressionConverter::GetDefault();
      const IConfigurationElement::Pointer expressionElement = expressionElements.front();
      try
      {
        whenExpression = elementHandler->Create(converter, expressionElement);
      }
      catch (const CoreException& /*e*/)
      {
        // There when expression could not be created.
        AddWarning(warningsToLog, "Problem creating when element",
                   parentElement, id, "whenElementName", whenElementName);
        return ERROR_EXPRESSION;
      }
    }
  }

  return whenExpression;
}

RegistryPersistence::RegistryPersistence()
  : registryListenerAttached(false)
{
}

RegistryPersistence::~RegistryPersistence()
{
  Platform::GetExtensionRegistry()->RemoveListener(this);
  registryListenerAttached = false;
}

void RegistryPersistence::Read()
{
  if (!registryListenerAttached)
  {
    Platform::GetExtensionRegistry()->AddListener(this);
    registryListenerAttached = true;
  }
}

void RegistryPersistence::Added(const QList<SmartPointer<IExtension> >& extensions)
{
  if (this->IsChangeImportant(extensions, OBJECTS_ADDED))
  {
    Display::GetDefault()->AsyncExec(new ReadRunnable(this));
  }
}

void RegistryPersistence::Removed(const QList<SmartPointer<IExtension> >& extensions)
{
  if (this->IsChangeImportant(extensions, OBJECTS_REMOVED))
  {
    Display::GetDefault()->AsyncExec(new ReadRunnable(this));
  }
}

void RegistryPersistence::Added(const QList<SmartPointer<IExtensionPoint> >& extensionPoints)
{
  if (this->IsChangeImportant(extensionPoints, OBJECTS_ADDED))
  {
    Display::GetDefault()->AsyncExec(new ReadRunnable(this));
  }
}

void RegistryPersistence::Removed(const QList<SmartPointer<IExtensionPoint> >& extensionPoints)
{
  if (this->IsChangeImportant(extensionPoints, OBJECTS_REMOVED))
  {
    Display::GetDefault()->AsyncExec(new ReadRunnable(this));
  }
}

}
