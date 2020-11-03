/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryHandlerPersistence.h"

#include "berryIExtension.h"
#include "berryIExtensionRegistry.h"
#include "berryIConfigurationElement.h"
#include "berryIHandler.h"
#include "berryIHandlerService.h"
#include "berryIHandlerActivation.h"

#include "berryHandlerProxy.h"
#include "berryPlatform.h"
#include "berryWorkbenchPlugin.h"
#include "berryExpression.h"

namespace berry {

const int HandlerPersistence::INDEX_COMMAND_DEFINITIONS = 0;
const int HandlerPersistence::INDEX_HANDLER_DEFINITIONS = 1;
//const int HandlerPersistence::INDEX_HANDLER_SUBMISSIONS = 2;
const int HandlerPersistence::INDEX_SIZE = 2;


HandlerPersistence::HandlerPersistence(IHandlerService* handlerService,
                                       IEvaluationService* evaluationService)
  : handlerService(handlerService)
  , evaluationService(evaluationService)
{
}

HandlerPersistence::~HandlerPersistence()
{
  ClearActivations(handlerService);
}

bool HandlerPersistence::HandlersNeedUpdating(const QList<SmartPointer<IExtension> >& extensions)
{
  /*
   * Handlers will need to be re-read (i.e., re-verified) if any of the
   * handler extensions change (i.e., handlers, commands), or if any of
   * the command extensions change (i.e., action definitions).
   */
  foreach(const IExtension::Pointer& extension, extensions)
  {
    QString xpId = extension->GetExtensionPointUniqueIdentifier();
    if (xpId == EXTENSION_HANDLERS ||
        xpId == EXTENSION_COMMANDS ||
        xpId == EXTENSION_ACTION_DEFINITIONS)
    {
      return true;
    }
  }
  return false;
}

void HandlerPersistence::ReRead()
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

    /*// Check if it is a handler submission or a command definition.
    if (TAG_HANDLER_SUBMISSION == name)
    {
      indexedConfigurationElements[INDEX_HANDLER_SUBMISSIONS].push_back(configurationElement);
    }
    else*/
    if (TAG_COMMAND == name)
    {
      indexedConfigurationElements[INDEX_COMMAND_DEFINITIONS].push_back(configurationElement);
    }
  }

  // Sort the handler extension point based on element name.
  QList<IConfigurationElement::Pointer> handlersExtensionPoint = registry
      ->GetConfigurationElementsFor(EXTENSION_HANDLERS);
  foreach (const IConfigurationElement::Pointer& configurationElement, handlersExtensionPoint)
  {
    const QString name = configurationElement->GetName();

    // Check if it is a handler submission or a command definition.
    if (TAG_HANDLER == name)
    {
      indexedConfigurationElements[INDEX_HANDLER_DEFINITIONS].push_back(configurationElement);
    }
  }

  ClearActivations(handlerService);
  ReadDefaultHandlersFromRegistry(indexedConfigurationElements[INDEX_COMMAND_DEFINITIONS]);
  //ReadHandlerSubmissionsFromRegistry(indexedConfigurationElements[INDEX_HANDLER_SUBMISSIONS]);
  ReadHandlersFromRegistry(indexedConfigurationElements[INDEX_HANDLER_DEFINITIONS]);
}

bool HandlerPersistence::IsChangeImportant(const QList<SmartPointer<IExtension> >& /*extensions*/,
                                           RegistryChangeType /*changeType*/)
{
  return false;
}

bool HandlerPersistence::IsChangeImportant(const QList<SmartPointer<IExtensionPoint> >& /*extensionPoints*/,
                                           RegistryChangeType /*changeType*/)
{
  return false;
}

void HandlerPersistence::Read()
{
  RegistryPersistence::Read();
  ReRead();
}

void HandlerPersistence::ClearActivations(IHandlerService* handlerService)
{
  handlerService->DeactivateHandlers(handlerActivations);
  foreach (const IHandlerActivation::Pointer& activation, handlerActivations)
  {
    if (activation->GetHandler().IsNotNull())
    {
      try
      {
        activation->GetHandler()->Dispose();
      }
      catch (const ctkException& e)
      {
        WorkbenchPlugin::Log("Failed to dispose handler for " + activation->GetCommandId(), e);
      }
      catch (...)
      {
        WorkbenchPlugin::Log("Failed to dispose handler for " + activation->GetCommandId());
      }
    }
  }
  handlerActivations.clear();
}

void HandlerPersistence::ReadDefaultHandlersFromRegistry(
    const QList<SmartPointer<IConfigurationElement> >& configurationElements)
{
  foreach (const IConfigurationElement::Pointer& configurationElement, configurationElements)
  {
    /*
     * Read out the command identifier. This was already checked by
     * <code>CommandPersistence</code>, so we'll just ignore any
     * problems here.
     */
    const QString commandId = ReadOptional(configurationElement, ATT_ID);
    if (commandId.isNull())
    {
      continue;
    }

    // Check to see if we have a default handler of any kind.
    if (configurationElement->GetAttribute(ATT_DEFAULT_HANDLER).isNull() &&
        configurationElement->GetChildren(TAG_DEFAULT_HANDLER).isEmpty())
    {
      continue;
    }

    IHandler::Pointer handler(new HandlerProxy(commandId, configurationElement, ATT_DEFAULT_HANDLER));
    handlerActivations.push_back(handlerService->ActivateHandler(commandId, handler));
  }
}

void HandlerPersistence::ReadHandlersFromRegistry(
    const QList<SmartPointer<IConfigurationElement> >& configurationElements)
{
  QList<IStatus::Pointer> warningsToLog;

  foreach (const IConfigurationElement::Pointer& configurationElement, configurationElements)
  {
    // Read out the command identifier.
    const QString commandId = ReadRequired(configurationElement,
                                           ATT_COMMAND_ID, warningsToLog, "Handlers need a command id");
    if (commandId.isNull())
    {
      continue;
    }

    // Check to see if we have a handler class.
    if (!CheckClass(configurationElement, warningsToLog,
                    "Handlers need a class", commandId))
    {
      continue;
    }

    // Get the activeWhen and enabledWhen expressions.
    const Expression::Pointer activeWhenExpression = ReadWhenElement(
          configurationElement, TAG_ACTIVE_WHEN, commandId, warningsToLog);
    if (activeWhenExpression == ERROR_EXPRESSION)
    {
      continue;
    }
    const Expression::Pointer enabledWhenExpression = ReadWhenElement(
          configurationElement, TAG_ENABLED_WHEN, commandId, warningsToLog);
    if (enabledWhenExpression == ERROR_EXPRESSION)
    {
      continue;
    }

    const IHandler::Pointer proxy(new HandlerProxy(commandId, configurationElement,
                                                   ATT_CLASS, enabledWhenExpression, evaluationService));
    handlerActivations.push_back(handlerService->ActivateHandler(commandId,
                                                                 proxy, activeWhenExpression));

    // Read out the help context identifier.
    const QString helpContextId = ReadOptional(configurationElement, ATT_HELP_CONTEXT_ID);
    handlerService->SetHelpContextId(proxy, helpContextId);
  }

  LogWarnings(warningsToLog,
              "Warnings while parsing the handlers from the 'org.blueberry.ui.handlers' extension point.");
}

}
