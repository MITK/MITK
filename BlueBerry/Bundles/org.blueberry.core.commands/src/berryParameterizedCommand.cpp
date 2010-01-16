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

#include "berryParameterizedCommand.h"

#include "berryIParameter.h"
#include "berryCommand.h"
#include "berryParameterization.h"
#include "berryExecutionEvent.h"
#include "berryCommandManager.h"
#include "berryCommandCategory.h"
#include "berryIHandler.h"

#include "internal/berryCommandUtils.h"

#include <berryObjectString.h>

#include <sstream>
#include <Poco/Hash.h>

namespace berry
{

const int INDEX_PARAMETER_ID = 0;
const int INDEX_PARAMETER_NAME = 1;
const int INDEX_PARAMETER_VALUE_NAME = 2;
const int INDEX_PARAMETER_VALUE_VALUE = 3;

const std::size_t ParameterizedCommand::HASH_CODE_NOT_COMPUTED = 0;
const std::size_t ParameterizedCommand::HASH_FACTOR = 89;
const std::size_t ParameterizedCommand::HASH_INITIAL = Poco::hash(
    "berry::ParameterizedCommand");

ParameterizedCommand::ParameterizedCommand(const SmartPointer<Command> command,
    const std::vector<Parameterization>& params) :
  command(command), hashCode(HASH_CODE_NOT_COMPUTED)
{
  if (!command)
  {
    throw Poco::NullPointerException(
        "A parameterized command cannot have a null command");
  }

  std::vector<IParameter::Pointer> parameters;
  try
  {
    parameters = command->GetParameters();
  } catch (const NotDefinedException* /*e*/)
  {
    // This should not happen.
  }
  if (!params.empty() && !parameters.empty())
  {
    for (unsigned int j = 0; j < parameters.size(); j++)
    {
      for (unsigned int i = 0; i < params.size(); i++)
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

Object::Pointer ParameterizedCommand::ExecuteWithChecks(const Object::ConstPointer trigger,
    const Object::ConstPointer applicationContext) throw(ExecutionException,
    NotDefinedException, NotEnabledException, NotHandledException)
{
  ExecutionEvent::Pointer excEvent(new ExecutionEvent(command,
          this->GetParameterMap(), trigger, applicationContext));
  return command->ExecuteWithChecks(excEvent);
}

SmartPointer<Command> ParameterizedCommand::GetCommand() const
{
  return command;
}

std::string ParameterizedCommand::GetId() const
{
  return command->GetId();
}

std::string ParameterizedCommand::GetName() const throw(NotDefinedException)
{
  if (name.empty())
  {
    std::stringstream nameBuffer;
    nameBuffer << command->GetName() << " (";
    const unsigned int parameterizationCount = parameterizations.size();
    for (unsigned int i = 0; i < parameterizationCount; i++)
    {
      const Parameterization& parameterization = parameterizations[i];
      nameBuffer << parameterization.GetParameter()->GetName() << ": ";
      try
      {
        nameBuffer << parameterization.GetValueName();
      }
      catch (const ParameterValuesException* /*e*/)
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
    name = nameBuffer.str();
  }
  return name;
}

std::map<std::string, std::string> ParameterizedCommand::GetParameterMap() const
{
  std::map<std::string, std::string> parameterMap;
  for (unsigned int i = 0; i < parameterizations.size(); i++)
  {
    const Parameterization& parameterization = parameterizations[i];
    parameterMap.insert(std::make_pair(parameterization.GetParameter()->GetId(),
            parameterization.GetValue()));
  }
  return parameterMap;
}

std::size_t ParameterizedCommand::HashCode() const
{
  if (hashCode == HASH_CODE_NOT_COMPUTED)
  {
    hashCode = HASH_INITIAL * HASH_FACTOR + (command ? command->HashCode() : 0);
    hashCode = hashCode * HASH_FACTOR;
    for (unsigned int i = 0; i < parameterizations.size(); i++)
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

std::string ParameterizedCommand::Serialize()
{
  const std::string escapedId(this->Escape(this->GetId()));

  if (parameterizations.empty())
  {
    return escapedId;
  }

  std::stringstream buffer;
  buffer << CommandManager::PARAMETER_START_CHAR;

  for (unsigned int i = 0; i < parameterizations.size(); i++)
  {

    if (i> 0)
    {
      // insert separator between parameters
      buffer << CommandManager::PARAMETER_SEPARATOR_CHAR;
    }

    const Parameterization& parameterization = parameterizations[i];
    const std::string parameterId(parameterization.GetParameter()->GetId());
    const std::string escapedParameterId(this->Escape(parameterId));

    buffer << escapedParameterId;

    const std::string parameterValue(parameterization.GetValue());
    if (!parameterValue.empty())
    {
      const std::string escapedParameterValue(this->Escape(parameterValue));
      buffer << CommandManager::ID_VALUE_CHAR
      << escapedParameterValue;
    }
  }

  buffer << CommandManager::PARAMETER_END_CHAR;

  return buffer.str();
}

std::string ParameterizedCommand::ToString() const
{
  std::stringstream buffer;
  buffer << "ParameterizedCommand(" << command->ToString() << ","
  << CommandUtils::ToString(parameterizations) << ")";
  return buffer.str();
}

std::vector<ParameterizedCommand::Pointer>
ParameterizedCommand::GenerateCombinations(const SmartPointer<Command> command)
throw(NotDefinedException)
{
  std::vector<IParameter::Pointer> parameters(command->GetParameters());

  typedef std::vector<std::list<Parameterization> > ExpandedParamsType;
  const ExpandedParamsType expansion(ExpandParameters(0, parameters));
  std::vector<ParameterizedCommand::Pointer> combinations;

  for (ExpandedParamsType::const_iterator expansionItr = expansion.begin();
      expansionItr != expansion.end(); ++expansionItr)
  {
    std::list<Parameterization> combination(*expansionItr);

    std::vector<Parameterization> parameterizations(combination.begin(), combination.end());
    ParameterizedCommand::Pointer pCmd(new ParameterizedCommand(command,
            parameterizations));
    combinations.push_back(pCmd);
  }

  return combinations;
}

ParameterizedCommand::Pointer ParameterizedCommand::GenerateCommand(const SmartPointer<Command> command,
    const std::map<std::string, Object::Pointer>& parameters)
{
  // no parameters
  if (parameters.empty())
  {
    ParameterizedCommand::Pointer pCmd(new ParameterizedCommand(command, std::vector<Parameterization>()));
    return pCmd;
  }

  try
  {
    std::vector<Parameterization> parms;

    // iterate over given parameters
    for (std::map<std::string, Object::Pointer>::const_iterator i = parameters.begin();
        i != parameters.end(); ++i)
    {
      std::string key(i->first);

      // get the parameter from the command
      IParameter::Pointer parameter(command->GetParameter(key));

      // if the parameter is defined add it to the parameter list
      if (!parameter)
      {
        return ParameterizedCommand::Pointer(0);
      }
      ParameterType::Pointer parameterType(command->GetParameterType(key));
      if (!parameterType)
      {
        std::string val(*(i->second.Cast<ObjectString>()));
        parms.push_back(Parameterization(parameter, val));
      }
      else
      {
        IParameterValueConverter::Pointer valueConverter(parameterType
            ->GetValueConverter());
        if (valueConverter)
        {
          std::string val(valueConverter->ConvertToString(i->second));
          parms.push_back(Parameterization(parameter, val));
        }
        else
        {
          std::string val(*(i->second.Cast<ObjectString>()));
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
  return ParameterizedCommand::Pointer(0);
}

std::string ParameterizedCommand::Escape(const std::string& rawText)
{

  std::string buffer;

  for (std::string::const_iterator i = rawText.begin();
      i != rawText.end(); ++i)
  {

    std::string::value_type c = *i;
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

  if (buffer.empty())
  {
    return rawText;
  }
  return buffer;
}

std::vector<std::list<Parameterization> > ParameterizedCommand::ExpandParameters(
    unsigned int startIndex, const std::vector<IParameter::Pointer>& parameters)
{
  typedef std::vector<std::list<Parameterization> > ReturnType;

  const unsigned int nextIndex = startIndex + 1;
  const bool noMoreParameters = (nextIndex >= parameters.size());

  const IParameter::Pointer parameter(parameters[startIndex]);
  ReturnType parameterizations;

  if (parameter->IsOptional())
  {
    parameterizations.push_back(std::list<Parameterization>());
  }

  IParameter::ParameterValues parameterValues(parameter->GetValues());
  for (IParameter::ParameterValues::iterator parameterValueItr =
      parameterValues.begin(); parameterValueItr != parameterValues.end(); ++parameterValueItr)
  {
    std::list<Parameterization> combination;
    combination.push_back(
        Parameterization(parameter, parameterValueItr->second));
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
      std::list<Parameterization> newCombination(*combinationItr);
      newCombination.insert(newCombination.end(), suffixItr->begin(),
          suffixItr->end());
      returnValue.push_back(newCombination);
    }
  }

  return returnValue;
}

}
