/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryHandlerUtil.h"

#include <berryIEvaluationContext.h>
#include <berryCommandExceptions.h>

#include "berryISources.h"
#include "berryRegistryToggleState.h"
#include "berryRadioState.h"
#include "berryObjects.h"
#include "berryObjectString.h"

namespace berry
{

void HandlerUtil::NoVariableFound(const ExecutionEvent::ConstPointer& event,
    const QString& name)
{
  throw ExecutionException("No " + name +
                           " found while executing " + event->GetCommand()->GetId());
}

void HandlerUtil::IncorrectTypeFound(const ExecutionEvent::ConstPointer& event,
    const QString& name, const QString& expectedType,
    const QString& wrongType)
{
  throw ExecutionException("Incorrect type for "
      + name
      + " found while executing "
      + event->GetCommand()->GetId()
      + ", expected " + expectedType
      + " found " + wrongType);
}

Object::ConstPointer HandlerUtil::GetVariable(
    const ExecutionEvent::ConstPointer& event, const QString& name)
{
  if (IEvaluationContext::ConstPointer evalContext = event->GetApplicationContext().Cast<const IEvaluationContext>())
  {
    Object::ConstPointer var = evalContext->GetVariable(name);
    return var == IEvaluationContext::UNDEFINED_VARIABLE ? Object::ConstPointer() : var;
  }
  return Object::ConstPointer();
}

Object::ConstPointer HandlerUtil::GetVariableChecked(
    const ExecutionEvent::ConstPointer& event, const QString& name)
{
  Object::ConstPointer o(HandlerUtil::GetVariable(event, name));
  if (o.IsNull())
  {
    HandlerUtil::NoVariableFound(event, name);
  }
  return o;
}

Object::ConstPointer HandlerUtil::GetVariable(
    Object::Pointer context, const QString& name)
{
  IEvaluationContext::Pointer eval(context.Cast<IEvaluationContext>());
  if (eval.IsNotNull())
  {
    Object::ConstPointer var = eval->GetVariable(name);
    return var == IEvaluationContext::UNDEFINED_VARIABLE ? Object::ConstPointer() : var;
  }
  return Object::ConstPointer(nullptr);
}

HandlerUtil::StringVectorType::ConstPointer HandlerUtil::GetActiveContexts(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o(HandlerUtil::GetVariable(event, ISources::ACTIVE_CONTEXT_NAME()));
  return o.Cast<const StringVectorType>();
}

HandlerUtil::StringVectorType::ConstPointer HandlerUtil::GetActiveContextsChecked(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o(HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_CONTEXT_NAME()));
  if (o.Cast<const StringVectorType>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event, ISources::ACTIVE_CONTEXT_NAME(),
        "StringVectorType", o->GetClassName());
  }
  return o.Cast<const StringVectorType>();
}

//IEditorPart::Pointer HandlerUtil::GetActiveEditor(const ExecutionEvent::ConstPointer& event)
//{
//  Object::Pointer o(HandlerUtil::GetVariable(event,
//      ISources::ACTIVE_EDITOR_NAME));
//  return o.Cast<IEditorPart>();
//}

//IEditorPart::Pointer HandlerUtil::GetActiveEditorChecked(
//    const ExecutionEvent::ConstPointer& event)
//{
//  Object::Pointer o = HandlerUtil::GetVariableChecked(event,
//      ISources::ACTIVE_EDITOR_NAME);
//  if (o.Cast<IEditorPart>().IsNull())
//  {
//    HandlerUtil::IncorrectTypeFound(event, ISources::ACTIVE_EDITOR_NAME,
//        "IEditorPart", o->GetClassName());
//  }
//  return (IEditorPart) o;
//}

ObjectString::ConstPointer HandlerUtil::GetActiveEditorId(const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariable(event,
                                                    ISources::ACTIVE_EDITOR_ID_NAME());
  return o.Cast<const ObjectString>();
}

ObjectString::ConstPointer HandlerUtil::GetActiveEditorIdChecked(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariableChecked(event,
                                                           ISources::ACTIVE_EDITOR_ID_NAME());
  if (o.Cast<const ObjectString>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event, ISources::ACTIVE_EDITOR_ID_NAME(),
                                    "QString", o->GetClassName());
  }
  return o.Cast<const ObjectString>();
}

IWorkbenchPart::Pointer HandlerUtil::GetActivePart(const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariable(event,
                                                    ISources::ACTIVE_PART_NAME());
  return IWorkbenchPart::Pointer(const_cast<IWorkbenchPart*>(dynamic_cast<const IWorkbenchPart*>(o.GetPointer())));
}

IWorkbenchPart::Pointer HandlerUtil::GetActivePartChecked(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_PART_NAME());
  if (o.Cast<const IWorkbenchPart>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event, ISources::ACTIVE_PART_NAME(),
                                    "IWorkbenchPart", o->GetClassName());
  }
  return IWorkbenchPart::Pointer(const_cast<IWorkbenchPart*>(dynamic_cast<const IWorkbenchPart*>(o.GetPointer())));
}

ObjectString::ConstPointer HandlerUtil::GetActivePartId(const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariable(event,
                                                    ISources::ACTIVE_PART_ID_NAME());
  return o.Cast<const ObjectString>();
}

ObjectString::ConstPointer HandlerUtil::GetActivePartIdChecked(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariableChecked(event,
                                                           ISources::ACTIVE_PART_ID_NAME());
  if (o.Cast<const ObjectString>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event, ISources::ACTIVE_PART_ID_NAME(),
                                    "QString", o->GetClassName());
  }
  return o.Cast<const ObjectString>();
}

IWorkbenchPartSite::Pointer HandlerUtil::GetActiveSite(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariable(event,
                                                    ISources::ACTIVE_SITE_NAME());
  return IWorkbenchPartSite::Pointer(const_cast<IWorkbenchPartSite*>(dynamic_cast<const IWorkbenchPartSite*>(o.GetPointer())));
}

IWorkbenchPartSite::Pointer HandlerUtil::GetActiveSiteChecked(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_SITE_NAME());
  if (o.Cast<const IWorkbenchPartSite>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event, ISources::ACTIVE_SITE_NAME(),
                                    "IWorkbenchSitePart", o->GetClassName());
  }
  return IWorkbenchPartSite::Pointer(const_cast<IWorkbenchPartSite*>(dynamic_cast<const IWorkbenchPartSite*>(o.GetPointer())));
}

ISelection::ConstPointer HandlerUtil::GetCurrentSelection(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariable(event,
                                                    ISources::ACTIVE_CURRENT_SELECTION_NAME());
  return o.Cast<const ISelection>();
}

ISelection::ConstPointer HandlerUtil::GetCurrentSelectionChecked(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_CURRENT_SELECTION_NAME());
  if (o.Cast<const ISelection>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event,
                                    ISources::ACTIVE_CURRENT_SELECTION_NAME(), "ISelection",
                                    o->GetClassName());
  }
  return o.Cast<const ISelection>();
}

HandlerUtil::StringVectorType::ConstPointer HandlerUtil::GetActiveMenus(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariable(event,
                                                    ISources::ACTIVE_MENU_NAME());
  return o.Cast<const StringVectorType>();
}

HandlerUtil::StringVectorType::ConstPointer HandlerUtil::GetActiveMenusChecked(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariableChecked(event,
                                                           ISources::ACTIVE_MENU_NAME());
  if (o.Cast<const StringVectorType>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event, ISources::ACTIVE_MENU_NAME(),
                                    "StringVectorType", o->GetClassName());
  }
  return o.Cast<const StringVectorType>();
}

ISelection::ConstPointer HandlerUtil::GetActiveMenuSelection(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariable(event,
                                                    ISources::ACTIVE_MENU_SELECTION_NAME());
  return o.Cast<const ISelection>();
}

ISelection::ConstPointer HandlerUtil::GetActiveMenuSelectionChecked(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_MENU_SELECTION_NAME());
  if (o.Cast<const ISelection>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event,
                                    ISources::ACTIVE_MENU_SELECTION_NAME(),
                                    "ISelection", o->GetClassName());
  }
  return o.Cast<const ISelection>();
}

IWorkbenchWindow::Pointer HandlerUtil::GetActiveWorkbenchWindow(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariable(event,
                                                    ISources::ACTIVE_WORKBENCH_WINDOW_NAME());
  return IWorkbenchWindow::Pointer(const_cast<IWorkbenchWindow*>(dynamic_cast<const IWorkbenchWindow*>(o.GetPointer())));
}

IWorkbenchWindow::Pointer HandlerUtil::GetActiveWorkbenchWindowChecked(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariableChecked(event,
                                                           ISources::ACTIVE_WORKBENCH_WINDOW_NAME());
  if (o.Cast<const IWorkbenchWindow>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event,
                                    ISources::ACTIVE_WORKBENCH_WINDOW_NAME(), "IWorkbenchWindow",
                                    o->GetClassName());
  }
  return IWorkbenchWindow::Pointer(const_cast<IWorkbenchWindow*>(dynamic_cast<const IWorkbenchWindow*>(o.GetPointer())));
}

ISelection::ConstPointer HandlerUtil::GetActiveMenuEditorInput(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariable(event,
                                               ISources::ACTIVE_MENU_EDITOR_INPUT_NAME());
  return o.Cast<const ISelection>();
}

ISelection::ConstPointer HandlerUtil::GetActiveMenuEditorInputChecked(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_MENU_EDITOR_INPUT_NAME());
  if (o.Cast<const ISelection>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event,
        ISources::ACTIVE_MENU_EDITOR_INPUT_NAME(), "ISelection",
        o->GetClassName());
  }
  return o.Cast<const ISelection>();
}

ISelection::ConstPointer HandlerUtil::GetShowInSelection(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariable(event,
      ISources::SHOW_IN_SELECTION());
  return o.Cast<const ISelection>();
}

ISelection::ConstPointer HandlerUtil::GetShowInSelectionChecked(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer o = HandlerUtil::GetVariableChecked(event,
      ISources::SHOW_IN_SELECTION());
  if (o.Cast<const ISelection>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event, ISources::SHOW_IN_SELECTION(),
        "ISelection", o->GetClassName());
  }
  return o.Cast<const ISelection>();
}

Object::ConstPointer HandlerUtil::GetShowInInput(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer var = HandlerUtil::GetVariable(event,
      ISources::SHOW_IN_INPUT());
  //    if (var == IEvaluationContext.UNDEFINED_VARIABLE) {
  //      return null;
  //    }
  return var;
}

Object::ConstPointer HandlerUtil::GetShowInInputChecked(
    const ExecutionEvent::ConstPointer& event)
{
  Object::ConstPointer var = HandlerUtil::GetVariableChecked(event,
      ISources::SHOW_IN_INPUT());
  //    if (var == IEvaluationContext.UNDEFINED_VARIABLE) {
  //      HandlerUtil::IncorrectTypeFound(event, ISources::SHOW_IN_INPUT, Object.class, var
  //          .getClass());
  //    }
  return var;
}

bool HandlerUtil::ToggleCommandState(const SmartPointer<Command>& command)
{
  State::Pointer state = command->GetState(RegistryToggleState::STATE_ID);
  if(state.IsNull())
  {
    throw ExecutionException("The command does not have a toggle state");
  }
  if(ObjectBool::Pointer boolObj = state->GetValue().Cast<ObjectBool>())
  {
    bool oldValue = boolObj->GetValue();
    Object::Pointer newValue(new ObjectBool(!oldValue));
    state->SetValue(newValue);
    return oldValue;
  }
  else
  {
   throw ExecutionException("The command's toggle state doesn't contain a boolean value");
  }
}

bool HandlerUtil::MatchesRadioState(const SmartPointer<ExecutionEvent>& event)
{
  QString parameter = event->GetParameter(RadioState::PARAMETER_ID);
  if (parameter.isNull())
  {
    throw ExecutionException("The event does not have the radio state parameter");
  }

  Command::ConstPointer command = event->GetCommand();
  State::Pointer state = command->GetState(RadioState::STATE_ID);
  if (state.IsNull())
  {
    throw ExecutionException("The command does not have a radio state");
  }
  if (ObjectString::Pointer strObj = state->GetValue().Cast<ObjectString>())
  {
    return parameter == *strObj;
  }
  else
  {
    throw ExecutionException("The command's radio state doesn't contain a String value");
  }
}

void HandlerUtil::UpdateRadioState(const SmartPointer<Command>& command, const QString& newState)
{
  State::Pointer state = command->GetState(RadioState::STATE_ID);
  if (state.IsNull())
  {
    throw ExecutionException("The command does not have a radio state");
  }
  Object::Pointer newValue(new ObjectString(newState));
  state->SetValue(newValue);
}

}
