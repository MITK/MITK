/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryExecutionEvent.h"

#include "common/berryCommandExceptions.h"
#include "berryIParameterValueConverter.h"
#include "berryCommandCategory.h"
#include "berryState.h"
#include "berryIHandler.h"

#include <berryObjectString.h>

#include <QTextStream>

namespace berry
{

ExecutionEvent::ExecutionEvent()
{

}

ExecutionEvent::ExecutionEvent(const Command::ConstPointer& cmd,
                               const ParameterMap& params, const Object::ConstPointer& trg,
                               const Object::Pointer& appContext)
: applicationContext(appContext), command(cmd), parameters(params), trigger(trg)
{

}

const Object::Pointer ExecutionEvent::GetApplicationContext() const
{
  return applicationContext;
}

const Command::ConstPointer ExecutionEvent::GetCommand() const
{
  return command;
}

const Object::ConstPointer ExecutionEvent::GetObjectParameterForExecution(
    const QString& parameterId) const
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
    const QString stringValue = this->GetParameter(parameterId);
    ObjectString::Pointer objectValue(new ObjectString(stringValue));
//    const Object objectValue = valueConverter
//    .convertToObject(stringValue);
    return objectValue;
  }
  catch (const NotDefinedException& e)
  {
    throw ExecutionException("Command is not defined", e); //$NON-NLS-1$
  }
//  catch (const ParameterValueConversionException e)
//  {
//    throw new ExecutionException(
//        "The parameter string could not be converted to an object", e); //$NON-NLS-1$
//  }
}

QString ExecutionEvent::GetParameter(const QString& parameterId) const
{
  ParameterMap::const_iterator res = parameters.find(parameterId);
  if (res != parameters.end())
    return res.value();
  else return "";
}

const ExecutionEvent::ParameterMap& ExecutionEvent::GetParameters() const
{
  return parameters;
}

const Object::ConstPointer ExecutionEvent::GetTrigger() const
{
  return trigger;
}

QString ExecutionEvent::ToString() const
{
  QString str;
  QTextStream ss(&str);
  ss << "ExecutionEvent(" << command->ToString() << ',' << parameters.size() << ','
     << (trigger ? trigger->ToString() : QString()) << ',' << applicationContext->ToString() << ')';

  return str;
}

}
