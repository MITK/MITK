/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryHandlerProxy.h"

#include "berryIConfigurationElement.h"
#include "berryIEvaluationContext.h"
#include "berryIEvaluationService.h"
#include "berryICommandService.h"
#include "berryIContributor.h"
#include "berryIEvaluationReference.h"

#include "berryEvaluationResult.h"
#include "berryExpression.h"
#include "berryCoreException.h"
#include "berryCommandExceptions.h"
#include "berryUtil.h"
#include "berryShell.h"
#include "berryObjects.h"
#include "berryObjectString.h"
#include "berryUIElement.h"
#include "berryRadioState.h"
#include "berryRegistryToggleState.h"
#include "berryHandlerEvent.h"
#include "berryPlatformUI.h"
#include "berryBundleUtility.h"
#include "berryWorkbenchRegistryConstants.h"
#include "berryStatus.h"
#include "berryWorkbenchPlugin.h"

#include <QMessageBox>

namespace berry {

QHash<SmartPointer<IConfigurationElement>, HandlerProxy*> HandlerProxy::CEToProxyMap =
    QHash<IConfigurationElement::Pointer, HandlerProxy*>();
const QString HandlerProxy::PROP_ENABLED = "enabled";


HandlerProxy::HandlerProxy(const QString commandId,
                           const SmartPointer<IConfigurationElement>& configurationElement,
                           const QString handlerAttributeName,
                           const SmartPointer<Expression> enabledWhenExpression,
                           IEvaluationService* evaluationService)
  : configurationElement(configurationElement)
  , enabledWhenExpression(enabledWhenExpression)
  , handlerAttributeName(handlerAttributeName)
  , evaluationService(evaluationService)
  , proxyEnabled(false)
  , commandId(commandId)
  , loadException(nullptr)
{
  this->Register();
  if (configurationElement.IsNull())
  {
    throw ctkInvalidArgumentException(
        "The configuration element backing a handler proxy cannot be null");
  }

  if (handlerAttributeName.isNull())
  {
    throw ctkInvalidArgumentException(
        "The attribute containing the handler class must be known");
  }

  if (enabledWhenExpression.IsNotNull() && evaluationService == nullptr)
  {
    throw ctkInvalidArgumentException(
        "We must have a handler service and evaluation service to support the enabledWhen expression");
  }

  if (enabledWhenExpression.IsNotNull())
  {
    SetProxyEnabled(false);
    RegisterEnablement();
  }
  else
  {
    SetProxyEnabled(true);
  }

  CEToProxyMap.insert(configurationElement, this);
  this->UnRegister(false);
}

void HandlerProxy::UpdateStaleCEs(const QList<SmartPointer<IConfigurationElement> >& replacements)
{
  foreach (const IConfigurationElement::Pointer& ce, replacements)
  {
    QHash<IConfigurationElement::Pointer, HandlerProxy*>::const_iterator i = CEToProxyMap.find(ce);
    if (i != CEToProxyMap.end())
    {
      (*i)->configurationElement = ce;
    }
  }
}

void HandlerProxy::SetEnabled(const Object::Pointer& evaluationContext)
{
  IEvaluationContext* const context = dynamic_cast<IEvaluationContext*>(evaluationContext.GetPointer());
  if (context == nullptr)
  {
    return;
  }
  if (enabledWhenExpression.IsNotNull())
  {
    try
    {
      SetProxyEnabled(enabledWhenExpression->Evaluate(context) == EvaluationResult::TRUE_EVAL);
    }
    catch (const CoreException& /*e*/)
    {
      // TODO should we log this exception, or just treat it as
      // a failure
    }
  }
  if (IsOkToLoad() && LoadHandler())
  {
    handler->SetEnabled(evaluationContext);
  }
}

void HandlerProxy::Dispose()
{
  if (handler.IsNotNull())
  {
    handler->RemoveHandlerListener(GetHandlerListener());
    handler->Dispose();
    handler = nullptr;
  }
  if (evaluationService && enablementRef)
  {
    evaluationService->RemoveEvaluationListener(enablementRef);
  }
  enablementRef = nullptr;
  delete loadException;
  loadException = nullptr;
}

Object::Pointer HandlerProxy::Execute(const SmartPointer<const ExecutionEvent>& event)
{
  if (LoadHandler())
  {
    if (!IsEnabled())
    {
      Shell::Pointer shell = Util::GetShellToParentOn();
      QWidget* parent = nullptr;
      if (shell.IsNotNull()) parent = shell->GetControl();
      QMessageBox::information(parent, "Information", "The chosen operation is not enabled.");
      return Object::Pointer(nullptr);
    }
    return handler->Execute(event);
  }

  if(loadException != nullptr)
    throw ExecutionException("Exception occured when loading the handler", *loadException);

  return Object::Pointer(nullptr);
}

bool HandlerProxy::IsEnabled() const
{
  if (enabledWhenExpression.IsNotNull())
  {
    // proxyEnabled reflects the enabledWhen clause
    if (!GetProxyEnabled())
    {
      return false;
    }
    if (IsOkToLoad() && LoadHandler())
    {
      return handler->IsEnabled();
    }

    return true;
  }

  /*
   * There is no enabled when expression, so we just need to consult the
   * handler.
   */
  if (IsOkToLoad() && LoadHandler())
  {
    return handler->IsEnabled();
  }
  return true;
}

bool HandlerProxy::IsHandled() const
{
  if (configurationElement.IsNotNull() && handler.IsNull())
  {
    return true;
  }

  if (IsOkToLoad() && LoadHandler())
  {
    return handler->IsHandled();
  }

  return false;
}

QString HandlerProxy::ToString() const
{
  if (handler.IsNull())
  {
    if (configurationElement.IsNotNull())
    {
      const QString configurationElementAttribute = GetConfigurationElementAttribute();
      if (!configurationElementAttribute.isEmpty())
      {
        return configurationElementAttribute;
      }
    }
    return "HandlerProxy()";
  }

  return handler->ToString();
}

void HandlerProxy::UpdateElement(UIElement* element, const QHash<QString, Object::Pointer>& parameters)
{
  if (checkedState.IsNotNull())
  {
    ObjectBool::Pointer value = checkedState->GetValue().Cast<ObjectBool>();
    element->SetChecked(value->GetValue());
  }
  else if (radioState.IsNotNull())
  {
    ObjectString::Pointer value = radioState->GetValue().Cast<ObjectString>();
    Object::Pointer parameter = parameters[RadioState::PARAMETER_ID];
    element->SetChecked(value.IsNotNull() && parameter == value);
  }
  if (handler.IsNotNull())
  {
    if (IElementUpdater::Pointer updater = handler.Cast<IElementUpdater>())
    {
      updater->UpdateElement(element, parameters);
    }
  }
}

void HandlerProxy::HandleStateChange(const SmartPointer<State>& state, const Object::Pointer& oldValue)
{
  if (state->GetId() == RegistryToggleState::STATE_ID)
  {
    checkedState = state;
    RefreshElements();
  }
  else if (state->GetId() == RadioState::STATE_ID)
  {
    radioState = state;
    RefreshElements();
  }
  if (IStateListener* stateListener = dynamic_cast<IStateListener*>(handler.GetPointer()))
  {
    stateListener->HandleStateChange(state, oldValue);
  }
}

SmartPointer<IConfigurationElement> HandlerProxy::GetConfigurationElement() const
{
  return configurationElement;
}

QString HandlerProxy::GetAttributeName() const
{
  return handlerAttributeName;
}

SmartPointer<IHandler> HandlerProxy::GetHandler() const
{
  return handler;
}

void HandlerProxy::RegisterEnablement()
{
  enablementRef = evaluationService->AddEvaluationListener(
        enabledWhenExpression, GetEnablementListener(), PROP_ENABLED);
}

void HandlerProxy::SetProxyEnabled(bool enabled)
{
  proxyEnabled = enabled;
}

bool HandlerProxy::GetProxyEnabled() const
{
  return proxyEnabled;
}

IPropertyChangeListener* HandlerProxy::GetEnablementListener() const
{
  return const_cast<HandlerProxy*>(this);
}

void HandlerProxy::PropertyChange(const SmartPointer<PropertyChangeEvent>& event)
{
  if (event->GetProperty() == PROP_ENABLED)
  {
    SetProxyEnabled(event->GetNewValue().IsNull() ?
                      false : event->GetNewValue().Cast<ObjectBool>()->GetValue());
    HandlerEvent::Pointer event(new HandlerEvent(IHandler::Pointer(this), true, false));
    FireHandlerChanged(event);
  }
}

bool HandlerProxy::LoadHandler() const
{
  if (handler.IsNull())
  {
    HandlerProxy* self = const_cast<HandlerProxy*>(this);
    // Load the handler.
    try
    {
      if (configurationElement.IsNotNull())
      {
        handler = configurationElement->CreateExecutableExtension<IHandler>(handlerAttributeName);
        if (handler.IsNotNull())
        {
          handler->AddHandlerListener(GetHandlerListener());
          self->SetEnabled(evaluationService == nullptr ? IEvaluationContext::Pointer(nullptr) : evaluationService->GetCurrentState());
          self->RefreshElements();
          return true;
        }
        else
        {
          const QString message = "The proxied handler was the wrong class";
          IStatus::Pointer status(
                new Status(IStatus::ERROR_TYPE, PlatformUI::PLUGIN_ID(), 0, message, BERRY_STATUS_LOC));
          WorkbenchPlugin::Log(message, status);
          configurationElement = nullptr;
          loadException = new ctkException("Class cast exception");
        }
      }
    }
    catch (const CoreException& e)
    {
      const QString message = "The proxied handler for '" + configurationElement->GetAttribute(handlerAttributeName)
          + "' could not be loaded";
      IStatus::Pointer status(
            new Status(IStatus::ERROR_TYPE, PlatformUI::PLUGIN_ID(), 0, message, e, BERRY_STATUS_LOC));
      WorkbenchPlugin::Log(message, status);
      configurationElement = nullptr;
      loadException = e.clone();
    }
    return false;
  }

  return true;
}

IHandlerListener* HandlerProxy::GetHandlerListener() const
{
  return const_cast<HandlerProxy*>(this);
}

void HandlerProxy::HandlerChanged(const SmartPointer<HandlerEvent>& handlerEvent)
{
  HandlerEvent::Pointer event(new HandlerEvent(IHandler::Pointer(this),
                                               handlerEvent->IsEnabledChanged(),
                                               handlerEvent->IsHandledChanged()));
  FireHandlerChanged(event);
}

QString HandlerProxy::GetConfigurationElementAttribute() const
{
  const QString attribute = configurationElement->GetAttribute(handlerAttributeName);
  if (attribute.isNull())
  {
    QList<IConfigurationElement::Pointer> children = configurationElement->GetChildren(handlerAttributeName);
    foreach (const IConfigurationElement::Pointer& child, children)
    {
      const QString childAttribute = child->GetAttribute(WorkbenchRegistryConstants::ATT_CLASS);
      if (!childAttribute.isNull())
      {
        return childAttribute;
      }
    }
  }
  return attribute;
}

bool HandlerProxy::IsOkToLoad() const
{
  if (configurationElement.IsNotNull() && handler.IsNull())
  {
    const QString bundleId = configurationElement->GetContributor()->GetName();
    return BundleUtility::IsActive(bundleId);
  }
  return true;
}

void HandlerProxy::RefreshElements()
{
  if (commandId.isNull() || (!(handler.Cast<IElementUpdater>())
      && (checkedState.IsNull() && radioState.IsNull())))
  {
    return;
  }
  ICommandService* cs = PlatformUI::GetWorkbench()->GetService<ICommandService>();
  cs->RefreshElements(commandId, QHash<QString, Object::Pointer>());
}

}
