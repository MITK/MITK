/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryParameterizedCommand.h"

#include "berryIParameter.h"
#include "berryIParameterValues.h"
#include "berryCommand.h"
#include "berryParameterization.h"
#include "berryExecutionEvent.h"
#include "berryCommandManager.h"
#include "berryCommandCategory.h"
#include "berryState.h"
#include "berryIHandler.h"

#include "internal/berryCommandUtils.h"

#include <berryObjectString.h>


namespace berry
{

const uint ParameterizedCommand::HASH_CODE_NOT_COMPUTED = 0;
const uint ParameterizedCommand::HASH_FACTOR = 89;
const uint ParameterizedCommand::HASH_INITIAL = qHash("berry::ParameterizedCommand");

ParameterizedCommand::ParameterizedCommand(const SmartPointer<Command>& command,
                                           const QList<Parameterization>& params)
  : command(command), hashCode(HASH_CODE_NOT_COMPUTED)
{
  if (!command)
  {
    throw Poco::NullPointerException(
        "A parameterized command cannot have a null command");
  }

  QList<IParameter::Pointer> parameters;
  try
  {
    parameters = command->GetParameters();
  } catch (const NotDefinedException* /*e*/)
  {
    // This should not happen.
  }
  if (!params.empty() && !parameters.empty())
  {
    for (int j = 0; j < parameters.size(); j++)
    {
      for (int i = 0; i < params.size(); i++)
      {
        if (parameters[j] == params[i].GetParameter())
        {
          this->parameterizations.push_back(params[i]);
        }
      }
    }
  }
}

bool ParameterizedCommand::operator<(const Object* object) const
{
  const ParameterizedCommand* command = dynamic_cast<const ParameterizedCommand*>(object);
  const bool thisDefined = this->command->IsDefined();
  const bool otherDefined = command->command->IsDefined();
  if (!thisDefined || !otherDefined)
  {
    return CommandUtils::Compare(thisDefined, otherDefined) < 0;
  }

  try
  {
    const int compareTo = this->GetName().compare(command->GetName());
    if (compareTo == 0)
    {
      return (this->GetId() < command->GetId());
    }
    return compareTo < 0;
  }
  catch (const NotDefinedException* /*e*/)
  {
    throw CommandException(
        "Concurrent modification of a command's defined state");
  }
}

bool ParameterizedCommand::operator==(const Object* object) const
{
  if (this == object)
  {
    return true;
  }

  if (const ParameterizedCommand* command = dynamic_cast<const ParameterizedCommand*>(object))
  {
    if (!(this->command == command->command))
    {
      return false;
    }

    return CommandUtils::Equals(this->parameterizations, command->parameterizations);
  }

  return false;
}

Object::Pointer ParameterizedCommand::ExecuteWithChecks(const Object::ConstPointer& trigger,
    const Object::Pointer& applicationContext)
{
  ExecutionEvent::Pointer excEvent(new ExecutionEvent(command,
          this->GetParameterMap(), trigger, applicationContext));
  return command->ExecuteWithChecks(excEvent);
}

SmartPointer<Command> ParameterizedCommand::GetCommand() const
{
  return command;
}

QString ParameterizedCommand::GetId() const
{
  return command->GetId();
}

QString ParameterizedCommand::GetName() const
{
  if (name.isEmpty())
  {
    QTextStream nameBuffer(&name);
    nameBuffer << command->GetName() << " (";
    const unsigned int parameterizationCount = (unsigned int) parameterizations.size();
    for (unsigned int i = 0; i < parameterizationCount; i++)
    {
      const Parameterization& parameterization = parameterizations[i];
      nameBuffer << parameterization.GetParameter()->GetName() << ": ";
      try
      {
        nameBuffer << parameterization.GetValueName();
      }
      catch (const ParameterValuesException& /*e*/)
      {
        /*
         * Just let it go for now. If someone complains we can
         * add more info later.
         */
      }

      // If there is another item, append a separator.
      if (i + 1 < parameterizationCount)
      {
        nameBuffer << ", ";
      }

      nameBuffer << ")";
    }
  }
  return name;
}

QHash<QString, QString> ParameterizedCommand::GetParameterMap() const
{
  QHash<QString, QString> parameterMap;
  for (int i = 0; i < parameterizations.size(); i++)
  {
    const Parameterization& parameterization = parameterizations[i];
    parameterMap.insert(parameterization.GetParameter()->GetId(),
                        parameterization.GetValue());
  }
  return parameterMap;
}

uint ParameterizedCommand::HashCode() const
{
  if (hashCode == HASH_CODE_NOT_COMPUTED)
  {
    hashCode = HASH_INITIAL * HASH_FACTOR + (command ? command->HashCode() : 0);
    hashCode = hashCode * HASH_FACTOR;
    for (int i = 0; i < parameterizations.size(); i++)
    {
      hashCode += parameterizations[i].HashCode();
    }

    if (hashCode == HASH_CODE_NOT_COMPUTED)
    {
      hashCode++;
    }
  }
  return hashCode;
}

QString ParameterizedCommand::Serialize()
{
  const QString escapedId(this->Escape(this->GetId()));

  if (parameterizations.empty())
  {
    return escapedId;
  }

  QString str;
  QTextStream buffer(&str);
  buffer << CommandManager::PARAMETER_START_CHAR;

  for (int i = 0; i < parameterizations.size(); i++)
  {

    if (i> 0)
    {
      // insert separator between parameters
      buffer << CommandManager::PARAMETER_SEPARATOR_CHAR;
    }

    const Parameterization& parameterization = parameterizations[i];
    const QString parameterId(parameterization.GetParameter()->GetId());
    const QString escapedParameterId(this->Escape(parameterId));

    buffer << escapedParameterId;

    const QString parameterValue(parameterization.GetValue());
    if (!parameterValue.isEmpty())
    {
      const QString escapedParameterValue(this->Escape(parameterValue));
      buffer << CommandManager::ID_VALUE_CHAR
             << escapedParameterValue;
    }
  }

  buffer << CommandManager::PARAMETER_END_CHAR;

  return str;
}

QString ParameterizedCommand::ToString() const
{
  QString str;
  QTextStream buffer(&str);
  buffer << "ParameterizedCommand(" << command->ToString() << ","
         << CommandUtils::ToString(parameterizations) << ")";
  return str;
}

QList<ParameterizedCommand::Pointer>
ParameterizedCommand::GenerateCombinations(const SmartPointer<Command> command)
{
  QList<IParameter::Pointer> parameters(command->GetParameters());

  typedef QList<QList<Parameterization> > ExpandedParamsType;
  const ExpandedParamsType expansion(ExpandParameters(0, parameters));
  QList<ParameterizedCommand::Pointer> combinations;

  for (ExpandedParamsType::const_iterator expansionItr = expansion.begin();
      expansionItr != expansion.end(); ++expansionItr)
  {
    QList<Parameterization> combination(*expansionItr);

    QList<Parameterization> parameterizations(combination);
    ParameterizedCommand::Pointer pCmd(new ParameterizedCommand(command,
            parameterizations));
    combinations.push_back(pCmd);
  }

  return combinations;
}

ParameterizedCommand::Pointer ParameterizedCommand::GenerateCommand(const SmartPointer<Command> command,
                                                                    const QHash<QString, Object::Pointer>& parameters)
{
  // no parameters
  if (parameters.empty())
  {
    ParameterizedCommand::Pointer pCmd(new ParameterizedCommand(command, QList<Parameterization>()));
    return pCmd;
  }

  try
  {
    QList<Parameterization> parms;

    // iterate over given parameters
    for (QHash<QString, Object::Pointer>::const_iterator i = parameters.begin();
        i != parameters.end(); ++i)
    {
      QString key(i.key());

      // get the parameter from the command
      IParameter::Pointer parameter(command->GetParameter(key));

      // if the parameter is defined add it to the parameter list
      if (!parameter)
      {
        return ParameterizedCommand::Pointer(nullptr);
      }
      ParameterType::Pointer parameterType(command->GetParameterType(key));
      if (!parameterType)
      {
        QString val = i.value()->ToString();
        parms.push_back(Parameterization(parameter, val));
      }
      else
      {
        IParameterValueConverter* valueConverter(parameterType->GetValueConverter());
        if (valueConverter)
        {
          QString val(valueConverter->ConvertToString(i.value()));
          parms.push_back(Parameterization(parameter, val));
        }
        else
        {
          QString val = i.value()->ToString();
          parms.push_back(Parameterization(parameter, val));
        }
      }
    }

    // convert the parameters to an Parameterization array and create
    // the command
    ParameterizedCommand::Pointer pCmd(new ParameterizedCommand(command, parms));
    return pCmd;
  }
  catch (const NotDefinedException* /*e*/)
  {
  }
  catch (const ParameterValueConversionException* /*e*/)
  {
  }
  return ParameterizedCommand::Pointer(nullptr);
}

QString ParameterizedCommand::Escape(const QString& rawText)
{

  QString buffer;

  for (QString::const_iterator i = rawText.begin();
      i != rawText.end(); ++i)
  {

    QString::value_type c = *i;
    if (c == CommandManager::PARAMETER_START_CHAR ||
        c == CommandManager::PARAMETER_END_CHAR ||
        c == CommandManager::ID_VALUE_CHAR ||
        c == CommandManager::PARAMETER_SEPARATOR_CHAR ||
        c == CommandManager::ESCAPE_CHAR)
    {
      buffer += CommandManager::ESCAPE_CHAR;
    }

    buffer += c;
  }

  if (buffer.isEmpty())
  {
    return rawText;
  }
  return buffer;
}

QList<QList<Parameterization> > ParameterizedCommand::ExpandParameters(
    unsigned int startIndex, const QList<IParameter::Pointer>& parameters)
{
  typedef QList<QList<Parameterization> > ReturnType;

  const int nextIndex = startIndex + 1;
  const bool noMoreParameters = (nextIndex >= parameters.size());

  const IParameter::Pointer parameter(parameters[startIndex]);
  ReturnType parameterizations;

  if (parameter->IsOptional())
  {
    parameterizations.push_back(QList<Parameterization>());
  }

  IParameterValues* values = nullptr;
  try
  {
    values = parameter->GetValues();
  }
  catch (const ParameterValuesException& /*e*/)
  {
    if (noMoreParameters)
    {
      return parameterizations;
    }

    // Make recursive call
    return ExpandParameters(nextIndex, parameters);
  }

  const QHash<QString,QString> parameterValues = values->GetParameterValues();
  for (IParameter::ParameterValues::const_iterator parameterValueItr =
      parameterValues.begin(); parameterValueItr != parameterValues.end(); ++parameterValueItr)
  {
    QList<Parameterization> combination;
    combination.push_back(
        Parameterization(parameter, parameterValueItr.value()));
    parameterizations.push_back(combination);
  }

  // Check if another iteration will produce any more names.
  if (noMoreParameters)
  {
    // This is it, so just return the current parameterizations.
    return parameterizations;
  }

  // Make recursive call
  ReturnType suffixes(ExpandParameters(nextIndex, parameters));
  if (suffixes.empty())
  {
    // This is it, so just return the current parameterizations.
    return parameterizations;
  }

  ReturnType returnValue;
  for (ReturnType::iterator suffixItr = suffixes.begin(); suffixItr
      != suffixes.end(); ++suffixItr)
  {
    for (ReturnType::iterator combinationItr = parameterizations.begin(); combinationItr
        != parameterizations.end(); ++combinationItr)
    {
      QList<Parameterization> newCombination(*combinationItr);
      newCombination.append(*suffixItr);
      returnValue.push_back(newCombination);
    }
  }

  return returnValue;
}

}
