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

#include "cherryExecutionEvent.h"

#include "common/cherryCommandExceptions.h"

#include <cherryObjectString.h>

namespace cherry
{

ExecutionEvent::ExecutionEvent()
{

}

ExecutionEvent::ExecutionEvent(const Command::ConstPointer cmd,
    const ParameterMap& params, const Object::ConstPointer trg,
    const Object::Pointer appContext)
: applicationContext(appContext), command(cmd), parameters(params), trigger(trg)
{

}

const Object::Pointer ExecutionEvent::GetApplicationContext()
{
  return applicationContext;
}

const Command::ConstPointer ExecutionEvent::GetCommand() const
{
  return command;
}

const Object::ConstPointer ExecutionEvent::GetObjectParameterForExecution(
    const std::string& parameterId)
{
  if (command.IsNull())
  {
    throw ExecutionException(
        "No command is associated with this execution event"); //$NON-NLS-1$
  }

  try
  {
//    const ParameterType parameterType = command
//    .getParameterType(parameterId);
//    if (parameterType == null)
//    {
//      throw new ExecutionException(
//          "Command does not have a parameter type for the given parameter"); //$NON-NLS-1$
//    }
//    const AbstractParameterValueConverter valueConverter = parameterType
//    .getValueConverter();
//    if (valueConverter == null)
//    {
//      throw new ExecutionException(
//          "Command does not have a value converter"); //$NON-NLS-1$
//    }
    const std::string stringValue = this->GetParameter(parameterId);
    ObjectString::Pointer objectValue(new ObjectString(stringValue));
//    const Object objectValue = valueConverter
//    .convertToObject(stringValue);
    return objectValue;
  }
  catch (NotDefinedException e)
  {
    throw ExecutionException("Command is not defined", e); //$NON-NLS-1$
  }
//  catch (ParameterValueConversionException e)
//  {
//    throw new ExecutionException(
//        "The parameter string could not be converted to an object", e); //$NON-NLS-1$
//  }
}

std::string ExecutionEvent::GetParameter(const std::string parameterId)
{
  ParameterMap::const_iterator res = parameters.find(parameterId);
  if (res != parameters.end())
    return res->second;
  else return "";
}

const ExecutionEvent::ParameterMap& ExecutionEvent::GetParameters()
{
  return parameters;
}

const Object::ConstPointer ExecutionEvent::GetTrigger()
{
  return trigger;
}

void ExecutionEvent::PrintSelf(std::ostream& os, Indent Indent) const
{

  os << Indent << "ExecutionEvent(" << command << ',' << parameters.size() << ','
      << trigger << ',' << applicationContext << ')';
}

}
