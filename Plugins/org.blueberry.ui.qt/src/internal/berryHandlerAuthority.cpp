/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryHandlerAuthority.h"

#include "berryISources.h"
#include "berryIServiceLocator.h"
#include "berryIEvaluationService.h"
#include "berryIEvaluationContext.h"
#include "berryIEvaluationReference.h"
#include "berryICommandService.h"
#include "berryIHandler.h"
#include "berryISourceProvider.h"
#include "services/berryISourceProviderService.h"

#include "berryObjects.h"
#include "berryHandlerActivation.h"
#include "berryMultiStatus.h"
#include "berryPlatformUI.h"
#include "berryWorkbenchPlugin.h"
#include "berryCommandTracing.h"
#include "berryCommand.h"
#include "berryEvaluationContext.h"
#include "berryExpression.h"

#include <QTime>

namespace berry {

bool HandlerAuthority::DEBUG = false; // = Policy.DEBUG_HANDLERS;
bool HandlerAuthority::DEBUG_PERFORMANCE = false; // = Policy.DEBUG_HANDLERS_PERFORMANCE;
bool HandlerAuthority::DEBUG_VERBOSE = false; // = Policy.DEBUG_HANDLERS && Policy.DEBUG_HANDLERS_VERBOSE;
QString HandlerAuthority::DEBUG_VERBOSE_COMMAND_ID; // = Policy.DEBUG_HANDLERS_VERBOSE_COMMAND_ID;
const QString HandlerAuthority::TRACING_COMPONENT = "HANDLERS";

const QList<QString> HandlerAuthority::SELECTION_VARIABLES = QList<QString>()
    << ISources::ACTIVE_CURRENT_SELECTION_NAME()
    << ISources::ACTIVE_FOCUS_CONTROL_ID_NAME()
    << ISources::ACTIVE_FOCUS_CONTROL_NAME()
    << ISources::ACTIVE_MENU_EDITOR_INPUT_NAME()
    << ISources::ACTIVE_MENU_NAME()
    << ISources::ACTIVE_MENU_SELECTION_NAME();

class HandlerAuthority::HandlerPropertyListener : public IPropertyChangeListener
{
private:

  HandlerAuthority* authority;
  HandlerActivation::Pointer handler;

public:

  HandlerPropertyListener(HandlerAuthority* authority, const HandlerActivation::Pointer& activation)
    : authority(authority)
    , handler(activation)
  {
    handler->SetListener(this);
  }

  using IPropertyChangeListener::PropertyChange;

  void PropertyChange(const PropertyChangeEvent::Pointer& event) override
  {
    if (handler->GetCommandId() == event->GetProperty())
    {
      bool val = false;
      if (ObjectBool::Pointer boolObj = event->GetNewValue().Cast<ObjectBool>())
      {
        val = boolObj->GetValue();
      }
      handler->SetResult(val);
      authority->changedCommandIds.insert(handler->GetCommandId());
    }
  }
};

IEvaluationService* HandlerAuthority::GetEvaluationService() const
{
  if (evalService == nullptr)
  {
    evalService = locator->GetService<IEvaluationService>();
    evalService->AddServiceListener(GetServiceListener());
  }
  return evalService;
}

IPropertyChangeListener* HandlerAuthority::GetServiceListener() const
{
  return const_cast<HandlerAuthority*>(this);
}

void HandlerAuthority::PropertyChange(const PropertyChangeEvent::Pointer& event)
{
  if (IEvaluationService::PROP_NOTIFYING == event->GetProperty())
  {
    if (ObjectBool::Pointer boolObj = event->GetNewValue().Cast<ObjectBool>())
    {
      bool startNotifying = boolObj->GetValue();
      if (startNotifying)
      {
        changedCommandIds.clear();
      }
      else
      {
        ProcessChangedCommands();
      }
    }
  }
}

HandlerAuthority::HandlerAuthority(ICommandService* commandService,
                 IServiceLocator* locator)
  : commandService(commandService)
  , locator(locator)
  , evalService(nullptr)
{
  if (commandService == nullptr)
  {
    throw ctkInvalidArgumentException("The handler authority needs a command service");
  }
}

HandlerAuthority::~HandlerAuthority()
{
  GetEvaluationService()->RemoveServiceListener(GetServiceListener());
}

void HandlerAuthority::ActivateHandler(const SmartPointer<IHandlerActivation>& activation)
{
  const HandlerActivation::Pointer handler = activation.Cast<HandlerActivation>();

  // First we update the handlerActivationsByCommandId map.
  const QString commandId = handler->GetCommandId();
  MultiStatus::Pointer conflicts(
        new MultiStatus(PlatformUI::PLUGIN_ID(), 0,
                        "A handler conflict occurred. This may disable some commands.",
                        BERRY_STATUS_LOC)
        );
  IdToHandlerActivationMap::mapped_type& handlerActivations = handlerActivationsByCommandId[commandId];
  handlerActivations.insert(handler, Empty());
  if (handler->GetExpression().IsNotNull())
  {
    auto   l = new HandlerPropertyListener(this, handler);
    handler->SetReference(GetEvaluationService()->AddEvaluationListener(
                            handler->GetExpression(), l,
                            handler->GetCommandId()));
  }
  if (handlerActivations.size() > 1)
  {
    UpdateCommand(commandId, ResolveConflicts(commandId, handlerActivations, conflicts));
  }
  else
  {
    UpdateCommand(commandId, (Evaluate(handler) ? handler : HandlerActivation::Pointer(nullptr)));
  }

  if (conflicts->GetSeverity() != IStatus::OK_TYPE)
  {
    WorkbenchPlugin::Log(conflicts);
  }
}

void HandlerAuthority::DeactivateHandler(const SmartPointer<IHandlerActivation>& activation)
{
  const HandlerActivation::Pointer handler = activation.Cast<HandlerActivation>();

  // First we update the handlerActivationsByCommandId map.
  const QString commandId = handler->GetCommandId();
  MultiStatus::Pointer conflicts(
        new MultiStatus("org.blueberry.ui", 0, "A handler conflict occurred.  This may disable some commands.", BERRY_STATUS_LOC));
  IdToHandlerActivationMap::iterator value = handlerActivationsByCommandId.find(commandId);
  if (value != handlerActivationsByCommandId.end())
  {
    IdToHandlerActivationMap::mapped_type& handlerActivations = value.value();
    if (handlerActivations.remove(handler) > 0)
    {
      if (handler->GetReference().IsNotNull())
      {
        GetEvaluationService()->RemoveEvaluationListener(handler->GetReference());
        handler->SetReference(IEvaluationReference::Pointer(nullptr));
        handler->SetListener(nullptr);
      }
      if (handlerActivations.isEmpty())
      {
        handlerActivationsByCommandId.remove(commandId);
        UpdateCommand(commandId, IHandlerActivation::Pointer(nullptr));
      }
      else if (handlerActivations.size() == 1)
      {
        IHandlerActivation::Pointer remainingActivation = handlerActivations.begin().key();
        UpdateCommand(commandId, (Evaluate(remainingActivation) ? remainingActivation : IHandlerActivation::Pointer(nullptr)));
      }
      else
      {
        UpdateCommand(commandId, ResolveConflicts(commandId, handlerActivations, conflicts));
      }
    }
  }

  if (conflicts->GetSeverity() != IStatus::OK_TYPE)
  {
    WorkbenchPlugin::Log(conflicts);
  }
}

SmartPointer<IHandlerActivation> HandlerAuthority::ResolveConflicts(
    const QString& commandId,
    const QMap<SmartPointer<HandlerActivation>,Empty>& activations,
    SmartPointer<MultiStatus> conflicts)
{
  // If we don't have any, then there is no match.
  if (activations.isEmpty())
  {
    return IHandlerActivation::Pointer(nullptr);
  }

  // Cycle over the activations, remembered the current best.
  QMapIterator<HandlerActivation::Pointer,Empty> activationItr(activations);
  HandlerActivation::Pointer bestActivation;
  HandlerActivation::Pointer currentActivation;
  bool conflict = false;
  while (activationItr.hasNext())
  {
    currentActivation = activationItr.next().key();
    if (!Evaluate(currentActivation))
    {
      continue; // only consider potentially active handlers
    }

    // Check to see if we haven't found a potentially active handler yet
    if ((DEBUG_VERBOSE) && ((DEBUG_VERBOSE_COMMAND_ID.isNull()) ||
                            (DEBUG_VERBOSE_COMMAND_ID == commandId)))
    {
      CommandTracing::PrintTrace(TRACING_COMPONENT,
                                 "    resolveConflicts: eval: " + currentActivation->ToString());
    }
    if (bestActivation.IsNull())
    {
      bestActivation = currentActivation;
      conflict = false;
      continue;
    }

    // Compare the two handlers.
    int comparison = bestActivation->CompareTo(currentActivation.GetPointer());
    if (comparison < 0)
    {
      bestActivation = currentActivation;
      conflict = false;
    }
    else if (comparison == 0)
    {
      if (currentActivation->GetHandler() != bestActivation->GetHandler())
      {
        conflict = true;
        break;
      }
    }
    else
    {
      break;
    }
  }

  // If we are logging information, now is the time to do it.
  if (DEBUG)
  {
    if (conflict)
    {
      CommandTracing::PrintTrace(TRACING_COMPONENT,
                                 "Unresolved conflict detected for '" + commandId + '\'');
    }
    else if ((bestActivation.IsNotNull()) && (DEBUG_VERBOSE) &&
             ((DEBUG_VERBOSE_COMMAND_ID.isNull()) || (DEBUG_VERBOSE_COMMAND_ID == commandId)))
    {
      CommandTracing::PrintTrace(TRACING_COMPONENT,
                                 "Resolved conflict detected.  The following activation won: ");
      CommandTracing::PrintTrace(TRACING_COMPONENT, "    " + bestActivation->ToString());
    }
  }

  // Return the current best.
  if (conflict)
  {
    if (!previousLogs.contains(commandId))
    {
      previousLogs.insert(commandId);
      QString str;
      QDebug dbg(&str);
      dbg << "Conflict for" << commandId << ":";
      dbg << bestActivation->ToString();
      dbg << currentActivation->ToString();

      IStatus::Pointer s(new Status(IStatus::WARNING_TYPE, "org.blueberry.ui", str, BERRY_STATUS_LOC));
      conflicts->Add(s);
    }
    return IHandlerActivation::Pointer(nullptr);
  }
  return bestActivation;
}

void HandlerAuthority::UpdateCommand(const QString& commandId,
                   const SmartPointer<IHandlerActivation>& activation)
{
  const Command::Pointer command = commandService->GetCommand(commandId);
  if (activation.IsNull())
  {
    command->SetHandler(IHandler::Pointer(nullptr));
  }
  else
  {
    command->SetHandler(activation->GetHandler());
    commandService->RefreshElements(commandId,  QHash<QString, Object::Pointer>());
  }
}

SmartPointer<IHandler> HandlerAuthority::FindHandler(const QString& commandId,
                                                     IEvaluationContext* context)
{
  IdToHandlerActivationMap::mapped_type activations = handlerActivationsByCommandId.value(commandId);

  IHandlerActivation::Pointer lastActivation;
  IHandlerActivation::Pointer currentActivation;
  QMapIterator<HandlerActivation::Pointer,Empty> i(activations);
  while (i.hasNext() && lastActivation.IsNull())
  {
    HandlerActivation::Pointer activation = i.next().key();
    try
    {
      if (Eval(context, activation))
      {
        lastActivation = currentActivation;
        currentActivation = activation;
      }
    }
    catch (const CoreException&)
    {
      // OK, this one is out of the running
    }
  }
  if (currentActivation.IsNotNull())
  {
    if (lastActivation.IsNull())
    {
      return currentActivation->GetHandler();
    }
    if (lastActivation->GetSourcePriority() != currentActivation->GetSourcePriority())
    {
      return lastActivation->GetHandler();
    }
  }

  return IHandler::Pointer(nullptr);
}

IEvaluationContext::Pointer HandlerAuthority::CreateContextSnapshot(bool includeSelection)
{
  IEvaluationContext::Pointer tmpContext = GetCurrentState();

  EvaluationContext::Pointer context;
  if (includeSelection)
  {
    context = new EvaluationContext(nullptr, tmpContext->GetDefaultVariable());
    for (int i = 0; i < SELECTION_VARIABLES.size(); i++)
    {
      CopyVariable(context.GetPointer(), tmpContext.GetPointer(), SELECTION_VARIABLES[i]);
    }
  }
  else
  {
    context = new EvaluationContext(nullptr, Object::Pointer(nullptr));
  }

  ISourceProviderService* sp = locator->GetService<ISourceProviderService>();
  QList<ISourceProvider::Pointer> providers = sp->GetSourceProviders();
  for (int i = 0; i < providers.size(); i++)
  {
    QList<QString> names = providers[i]->GetProvidedSourceNames();
    for (int j = 0; j < names.size(); j++)
    {
      if (!IsSelectionVariable(names[j]))
      {
        CopyVariable(context.GetPointer(), tmpContext.GetPointer(), names[j]);
      }
    }
  }

  return context;
}

bool HandlerAuthority::Eval(IEvaluationContext* context,
                            const SmartPointer<IHandlerActivation>& activation)
{
  Expression::Pointer expression = activation->GetExpression();
  if (expression.IsNull())
  {
    return true;
  }
  return expression->Evaluate(context) == EvaluationResult::TRUE_EVAL;
}

bool HandlerAuthority::IsSelectionVariable(const QString& name)
{
  for (int i = 0; i < SELECTION_VARIABLES.size(); i++)
  {
    if (SELECTION_VARIABLES[i] == name)
    {
      return true;
    }
  }
  return false;
}

void HandlerAuthority::CopyVariable(IEvaluationContext* context,
                  IEvaluationContext* tmpContext, const QString& var)
{
  Object::ConstPointer o = tmpContext->GetVariable(var);
  if (o.IsNotNull())
  {
    context->AddVariable(var, o);
  }
}

void HandlerAuthority::ProcessChangedCommands()
{
  // If tracing, then track how long it takes to process the activations.
  QTime startTime;
  if (DEBUG_PERFORMANCE)
  {
    startTime.start();
  }

  MultiStatus::Pointer conflicts(
        new MultiStatus("org.blueberry.ui", 0,
                        "A handler conflict occurred.  This may disable some commands.", BERRY_STATUS_LOC));

  /*
   * For every command identifier with a changed activation, we resolve
   * conflicts and trigger an update.
   */
  const QSet<QString> changedIds = changedCommandIds;
  changedCommandIds.clear();
  foreach(const QString& commandId, changedIds)
  {
    IdToHandlerActivationMap::mapped_type activations = handlerActivationsByCommandId.value(commandId);
    if (activations.size() == 1)
    {
      const IHandlerActivation::Pointer activation = activations.begin().key();
      UpdateCommand(commandId, (Evaluate(activation) ? activation : IHandlerActivation::Pointer(nullptr)));
    }
    else
    {
      const IHandlerActivation::Pointer activation = ResolveConflicts(
            commandId, activations, conflicts);
      UpdateCommand(commandId, activation);
    }
  }
  if (conflicts->GetSeverity() != IStatus::OK_TYPE)
  {
    WorkbenchPlugin::Log(conflicts);
  }

  // If tracing performance, then print the results.
  if (DEBUG_PERFORMANCE)
  {
    const int elapsedTime = startTime.elapsed();
    const int size = changedCommandIds.size();
    if (size > 0)
    {
      CommandTracing::PrintTrace(TRACING_COMPONENT, QString::number(size) + " command ids changed in " + elapsedTime + "ms");
    }
  }
}

bool HandlerAuthority::Evaluate(const SmartPointer<IEvaluationResultCache>& expression)
{
  IEvaluationContext::Pointer contextWithDefaultVariable = GetCurrentState();
  return expression->Evaluate(contextWithDefaultVariable.GetPointer());
}

SmartPointer<IEvaluationContext> HandlerAuthority::GetCurrentState() const
{
  return GetEvaluationService()->GetCurrentState();
}

//void HandlerAuthority::UpdateShellKludge()
//{
//  ((EvaluationService) getEvaluationService()).updateShellKludge();
//}

//void HandlerAuthority::UpdateShellKludge(const SmartPointer<Shell>& shell)
//{
//  ((EvaluationService) getEvaluationService()).updateShellKludge(shell);
//}

}
