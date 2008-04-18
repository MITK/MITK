/*=========================================================================
 
Program:   openCherry Platform
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

#include "cherryHandlerUtil.h"

#include <org.opencherry.core.expressions/cherryIEvaluationContext.h>
#include <org.opencherry.core.commands/common/cherryCommandExceptions.h>

#include "../cherryISources.h"

namespace cherry
{

void HandlerUtil::NoVariableFound(ExecutionEvent::Pointer event,
    const std::string& name)
{
  throw ExecutionException("No " + name //$NON-NLS-1$
      + " found while executing " + event->GetCommand()->GetId()); //$NON-NLS-1$
}

void HandlerUtil::IncorrectTypeFound(ExecutionEvent::Pointer event,
    const std::string& name, const std::string& expectedType,
    const std::string& wrongType)
{
  throw ExecutionException("Incorrect type for " //$NON-NLS-1$
      + name
      + " found while executing " //$NON-NLS-1$
      + event->GetCommand()->GetId()
      + ", expected " + expectedType //$NON-NLS-1$
      + " found " + wrongType); //$NON-NLS-1$
}

Object::Pointer HandlerUtil::GetVariable(
    ExecutionEvent::Pointer event, const std::string& name)
{
  return event->GetApplicationContext().Cast<IEvaluationContext>()->GetVariable(name);
}

Object::Pointer HandlerUtil::GetVariableChecked(
    ExecutionEvent::Pointer event, const std::string& name)
{
  Object::Pointer o(HandlerUtil::GetVariable(event, name));
  if (o.IsNull())
  {
    HandlerUtil::NoVariableFound(event, name);
  }
  return o;
}

Object::Pointer HandlerUtil::GetVariable(
    Object::Pointer context, const std::string& name)
{
  IEvaluationContext::Pointer eval(context.Cast<IEvaluationContext>());
  if (eval.IsNotNull())
  {
    return eval->GetVariable(name);
  }
  return Object::Pointer(0);
}

ObjectVector::Pointer HandlerUtil::GetActiveContexts(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o(HandlerUtil::GetVariable(event,
      ISources::ACTIVE_CONTEXT_NAME));
  return o.Cast<ObjectVector>();
}

ObjectVector::Pointer HandlerUtil::GetActiveContextsChecked(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o(HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_CONTEXT_NAME));
  if (o.Cast<ObjectVector>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event, ISources::ACTIVE_CONTEXT_NAME,
        "ObjectVector", o->GetNameOfClass());
  }
  return o.Cast<ObjectVector>();
}

//IEditorPart::Pointer HandlerUtil::GetActiveEditor(ExecutionEvent::Pointer event)
//{
//  Object::Pointer o(HandlerUtil::GetVariable(event,
//      ISources::ACTIVE_EDITOR_NAME));
//  return o.Cast<IEditorPart>();
//}

//IEditorPart::Pointer HandlerUtil::GetActiveEditorChecked(
//    ExecutionEvent::Pointer event)
//{
//  Object::Pointer o = HandlerUtil::GetVariableChecked(event,
//      ISources::ACTIVE_EDITOR_NAME);
//  if (o.Cast<IEditorPart>().IsNull())
//  {
//    HandlerUtil::IncorrectTypeFound(event, ISources::ACTIVE_EDITOR_NAME,
//        "IEditorPart", o->GetNameOfClass());
//  }
//  return (IEditorPart) o;
//}

ObjectString::Pointer HandlerUtil::GetActiveEditorId(ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariable(event,
      ISources::ACTIVE_EDITOR_ID_NAME);
  return o.Cast<ObjectString>();
}

ObjectString::Pointer HandlerUtil::GetActiveEditorIdChecked(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_EDITOR_ID_NAME);
  if (o.Cast<ObjectString>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event, ISources::ACTIVE_EDITOR_ID_NAME,
        "std::string", o->GetNameOfClass());
  }
  return o.Cast<ObjectString>();
}

IWorkbenchPart::Pointer HandlerUtil::GetActivePart(ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariable(event,
      ISources::ACTIVE_PART_NAME);
  return o.Cast<IWorkbenchPart>();
}

IWorkbenchPart::Pointer HandlerUtil::GetActivePartChecked(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_PART_NAME);
  if (o.Cast<IWorkbenchPart>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event, ISources::ACTIVE_PART_NAME,
        "IWorkbenchPart", o->GetNameOfClass());
  }
  return o.Cast<IWorkbenchPart>();
}

ObjectString::Pointer HandlerUtil::GetActivePartId(ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariable(event,
      ISources::ACTIVE_PART_ID_NAME);
  return o.Cast<ObjectString>();
}

ObjectString::Pointer HandlerUtil::GetActivePartIdChecked(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_PART_ID_NAME);
  if (o.Cast<ObjectString>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event, ISources::ACTIVE_PART_ID_NAME,
        "std::string", o->GetNameOfClass());
  }
  return o.Cast<ObjectString>();
}

IWorkbenchPartSite::Pointer HandlerUtil::GetActiveSite(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariable(event,
      ISources::ACTIVE_SITE_NAME);
  return o.Cast<IWorkbenchPartSite>();
}

IWorkbenchPartSite::Pointer HandlerUtil::GetActiveSiteChecked(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_SITE_NAME);
  if (o.Cast<IWorkbenchPartSite>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event, ISources::ACTIVE_SITE_NAME,
        "IWorkbenchSitePart", o->GetNameOfClass());
  }
  return o.Cast<IWorkbenchPartSite>();
}

ISelection::Pointer HandlerUtil::GetCurrentSelection(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariable(event,
      ISources::ACTIVE_CURRENT_SELECTION_NAME);
  return o.Cast<ISelection>();
}

ISelection::Pointer HandlerUtil::GetCurrentSelectionChecked(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_CURRENT_SELECTION_NAME);
  if (o.Cast<ISelection>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event,
        ISources::ACTIVE_CURRENT_SELECTION_NAME, "ISelection",
        o->GetNameOfClass());
  }
  return o.Cast<ISelection>();
}

ObjectVector::Pointer HandlerUtil::GetActiveMenus(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariable(event,
      ISources::ACTIVE_MENU_NAME);
  return o.Cast<ObjectVector>();
}

ObjectVector::Pointer HandlerUtil::GetActiveMenusChecked(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_MENU_NAME);
  if (o.Cast<ObjectVector>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event, ISources::ACTIVE_MENU_NAME,
        "ObjectVector", o->GetNameOfClass());
  }
  return o.Cast<ObjectVector>();
}

ISelection::Pointer HandlerUtil::GetActiveMenuSelection(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariable(event,
      ISources::ACTIVE_MENU_SELECTION_NAME);
  return o.Cast<ISelection>();
}

ISelection::Pointer HandlerUtil::GetActiveMenuSelectionChecked(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_MENU_SELECTION_NAME);
  if (o.Cast<ISelection>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event,
        ISources::ACTIVE_MENU_SELECTION_NAME, "ISelection", o->GetNameOfClass());
  }
  return o.Cast<ISelection>();
}

IWorkbenchWindow::Pointer HandlerUtil::GetActiveWorkbenchWindow(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariable(event,
      ISources::ACTIVE_WORKBENCH_WINDOW_NAME);
  return o.Cast<IWorkbenchWindow>();
}

IWorkbenchWindow::Pointer HandlerUtil::GetActiveWorkbenchWindowChecked(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_WORKBENCH_WINDOW_NAME);
  if (o.Cast<IWorkbenchWindow>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event,
        ISources::ACTIVE_WORKBENCH_WINDOW_NAME, "IWorkbenchWindow",
        o->GetNameOfClass());
  }
  return o.Cast<IWorkbenchWindow>();
}

ISelection::Pointer HandlerUtil::GetActiveMenuEditorInput(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariable(event,
      ISources::ACTIVE_MENU_EDITOR_INPUT_NAME);
  return o.Cast<ISelection>();
}

ISelection::Pointer HandlerUtil::GetActiveMenuEditorInputChecked(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariableChecked(event,
      ISources::ACTIVE_MENU_EDITOR_INPUT_NAME);
  if (o.Cast<ISelection>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event,
        ISources::ACTIVE_MENU_EDITOR_INPUT_NAME, "ISelection",
        o->GetNameOfClass());
  }
  return o.Cast<ISelection>();
}

ISelection::Pointer HandlerUtil::GetShowInSelection(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariable(event,
      ISources::SHOW_IN_SELECTION);
  return o.Cast<ISelection>();
}

ISelection::Pointer HandlerUtil::GetShowInSelectionChecked(
    ExecutionEvent::Pointer event)
{
  Object::Pointer o = HandlerUtil::GetVariableChecked(event,
      ISources::SHOW_IN_SELECTION);
  if (o.Cast<ISelection>().IsNull())
  {
    HandlerUtil::IncorrectTypeFound(event, ISources::SHOW_IN_SELECTION,
        "ISelection", o->GetNameOfClass());
  }
  return o.Cast<ISelection>();
}

Object::Pointer HandlerUtil::GetShowInInput(
    ExecutionEvent::Pointer event)
{
  Object::Pointer var = HandlerUtil::GetVariable(event,
      ISources::SHOW_IN_INPUT);
  //    if (var == IEvaluationContext.UNDEFINED_VARIABLE) {
  //      return null;
  //    }
  return var;
}

Object::Pointer HandlerUtil::GetShowInInputChecked(
    ExecutionEvent::Pointer event)
{
  Object::Pointer var = HandlerUtil::GetVariableChecked(event,
      ISources::SHOW_IN_INPUT);
  //    if (var == IEvaluationContext.UNDEFINED_VARIABLE) {
  //      HandlerUtil::IncorrectTypeFound(event, ISources::SHOW_IN_INPUT, Object.class, var
  //          .getClass());
  //    }
  return var;
}

}
