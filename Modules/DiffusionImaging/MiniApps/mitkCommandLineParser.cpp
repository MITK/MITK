/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
/*=========================================================================

  Library:   CTK

  Copyright (c) Kitware Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=========================================================================*/

// STL includes
#include <stdexcept>
#include <iostream>


// MITK includes
#include "mitkCommandLineParser.h"


using namespace std;

namespace
{
// --------------------------------------------------------------------------
class CommandLineParserArgumentDescription
{
public:


    CommandLineParserArgumentDescription(
            const string& longArg, const string& longArgPrefix,
            const string& shortArg, const string& shortArgPrefix,
            mitkCommandLineParser::Type type, const string& argHelp, const string& argLabel,
            const us::Any& defaultValue, bool ignoreRest,
            bool deprecated, bool optional, string& argGroup, string& groupDescription)
        : LongArg(longArg), LongArgPrefix(longArgPrefix),
          ShortArg(shortArg), ShortArgPrefix(shortArgPrefix),
          ArgHelp(argHelp), ArgLabel(argLabel), IgnoreRest(ignoreRest), NumberOfParametersToProcess(0),
          Deprecated(deprecated), DefaultValue(defaultValue), Value(type), ValueType(type), Optional(optional), ArgGroup(argGroup), ArgGroupDescription(groupDescription)
    {
        Value = defaultValue;

        switch (type)
        {
        case mitkCommandLineParser::String:
        {
            NumberOfParametersToProcess = 1;
        }
            break;
        case mitkCommandLineParser::Bool:
        {
            NumberOfParametersToProcess = 0;
        }
            break;
        case mitkCommandLineParser::StringList:
        {
            NumberOfParametersToProcess = -1;
        }
            break;
        case mitkCommandLineParser::Int:
        {
            NumberOfParametersToProcess = 1;
        }
            break;
        case mitkCommandLineParser::Float:
        {
            NumberOfParametersToProcess = 1;
        }
            break;

        case mitkCommandLineParser::OutputDirectory:
        case mitkCommandLineParser::InputDirectory:
        {
            NumberOfParametersToProcess = 1;
        }
            break;

        case mitkCommandLineParser::OutputFile:
        case mitkCommandLineParser::InputFile:
        {
            NumberOfParametersToProcess = 1;
        }
            break;
        case mitkCommandLineParser::InputImage:
        {
            NumberOfParametersToProcess = 1;
        }
          break;

        default:
            std::cout << "Type not supported: " << static_cast<int>(type);
        }

    }

    ~CommandLineParserArgumentDescription(){}

    bool addParameter(const string& value);

    string helpText();

    string  LongArg;
    string  LongArgPrefix;
    string  ShortArg;
    string  ShortArgPrefix;
    string  ArgHelp;
    string  ArgLabel;
    string  ArgGroup;
    string  ArgGroupDescription;
    bool    IgnoreRest;
    int     NumberOfParametersToProcess;
    bool    Deprecated;
    bool    Optional;

    us::Any                   DefaultValue;
    us::Any                   Value;
    mitkCommandLineParser::Type  ValueType;
};

// --------------------------------------------------------------------------
bool CommandLineParserArgumentDescription::addParameter(const string &value)
{
    switch (ValueType)
    {
    case mitkCommandLineParser::String:
    {
        Value = value;
    }
        break;
    case mitkCommandLineParser::Bool:
    {
        if (value.compare("true")==0)
            Value = true;
        else
            Value = false;
    }
        break;
    case mitkCommandLineParser::StringList:
    {
        try
        {
            mitkCommandLineParser::StringContainerType list = us::any_cast<mitkCommandLineParser::StringContainerType>(Value);
            list.push_back(value);
            Value = list;
        }
        catch(...)
        {
            mitkCommandLineParser::StringContainerType list;
            list.push_back(value);
            Value = list;
        }
    }
        break;
    case mitkCommandLineParser::Int:
    {
        stringstream ss(value);
        int i;
        ss >> i;
        Value = i;
    }
        break;
    case mitkCommandLineParser::Float:
    {
        stringstream ss(value);
        float f;
        ss >> f;
        Value = f;
    }
        break;

    case mitkCommandLineParser::InputDirectory:
    case mitkCommandLineParser::OutputDirectory:
    {
      Value = value;
    }
      break;

    case mitkCommandLineParser::InputFile:
    case mitkCommandLineParser::InputImage:
    case mitkCommandLineParser::OutputFile:
    {
      Value = value;
    }
        break;

    default:
        return false;
    }

    return true;
}

// --------------------------------------------------------------------------
string CommandLineParserArgumentDescription::helpText()
{
    string text;

    string shortAndLongArg;
    if (!this->ShortArg.empty())
    {
        shortAndLongArg = "  ";
        shortAndLongArg += this->ShortArgPrefix;
        shortAndLongArg += this->ShortArg;
    }

    if (!this->LongArg.empty())
    {
        if (this->ShortArg.empty())
            shortAndLongArg.append("  ");
        else
            shortAndLongArg.append(", ");

        shortAndLongArg += this->LongArgPrefix;
        shortAndLongArg += this->LongArg;
    }

    text = text + shortAndLongArg + ", " + this->ArgHelp;

    if (this->Optional)
        text += " (optional)";

    if (!this->DefaultValue.Empty())
    {
        text = text + ", (default: " + this->DefaultValue.ToString() + ")";
    }
    text += "\n";
    return text;
}

}

// --------------------------------------------------------------------------
// ctkCommandLineParser::ctkInternal class

// --------------------------------------------------------------------------
class mitkCommandLineParser::ctkInternal
{
public:
    ctkInternal()
        : Debug(false), FieldWidth(0), StrictMode(false)
    {}

    ~ctkInternal() {  }

    CommandLineParserArgumentDescription* argumentDescription(const string& argument);

    vector<CommandLineParserArgumentDescription*>                 ArgumentDescriptionList;
    map<string, CommandLineParserArgumentDescription*>        ArgNameToArgumentDescriptionMap;
    map<string, vector<CommandLineParserArgumentDescription*> > GroupToArgumentDescriptionListMap;

    StringContainerType UnparsedArguments;
    StringContainerType ProcessedArguments;
    string     ErrorString;
    bool        Debug;
    int         FieldWidth;
    string     LongPrefix;
    string     ShortPrefix;
    string     CurrentGroup;
    string     DisableQSettingsLongArg;
    string     DisableQSettingsShortArg;
    bool        StrictMode;
};

// --------------------------------------------------------------------------
// ctkCommandLineParser::ctkInternal methods

// --------------------------------------------------------------------------
CommandLineParserArgumentDescription*
mitkCommandLineParser::ctkInternal::argumentDescription(const string& argument)
{
    string unprefixedArg = argument;

    if (!LongPrefix.empty() && argument.compare(0, LongPrefix.size(), LongPrefix)==0)
    {
        // Case when (ShortPrefix + UnPrefixedArgument) matches LongPrefix
        if (argument == LongPrefix && !ShortPrefix.empty() && argument.compare(0, ShortPrefix.size(), ShortPrefix)==0)
        {
            unprefixedArg = argument.substr(ShortPrefix.size(),argument.size());
        }
        else
        {
            unprefixedArg = argument.substr(LongPrefix.size(),argument.size());
        }
    }
    else if (!ShortPrefix.empty() && argument.compare(0, ShortPrefix.size(), ShortPrefix)==0)
    {
        unprefixedArg = argument.substr(ShortPrefix.size(),argument.size());
    }
    else if (!LongPrefix.empty() && !ShortPrefix.empty())
    {
        return 0;
    }

    if (ArgNameToArgumentDescriptionMap.count(unprefixedArg))
    {
        return this->ArgNameToArgumentDescriptionMap[unprefixedArg];
    }
    return 0;
}

// --------------------------------------------------------------------------
// ctkCommandLineParser methods

// --------------------------------------------------------------------------
mitkCommandLineParser::mitkCommandLineParser()
{
    this->Internal = new ctkInternal();
    this->Category = string();
    this->Title = string();
    this->Contributor = string();
    this->Description = string();
    this->ParameterGroupName = "Parameters";
    this->ParameterGroupDescription = "Groupbox containing parameters.";
}

// --------------------------------------------------------------------------
mitkCommandLineParser::~mitkCommandLineParser()
{
    delete this->Internal;
}

// --------------------------------------------------------------------------
map<string, us::Any> mitkCommandLineParser::parseArguments(const StringContainerType& arguments,
                                                              bool* ok)
{
    // Reset
    this->Internal->UnparsedArguments.clear();
    this->Internal->ProcessedArguments.clear();
    this->Internal->ErrorString.clear();
    //    foreach (CommandLineParserArgumentDescription* desc, this->Internal->ArgumentDescriptionList)
    for (unsigned int i=0; i<Internal->ArgumentDescriptionList.size(); i++)
    {
        CommandLineParserArgumentDescription* desc = Internal->ArgumentDescriptionList.at(i);
        desc->Value = us::Any(desc->ValueType);
        if (!desc->DefaultValue.Empty())
        {
            desc->Value = desc->DefaultValue;
        }
    }
    bool error = false;
    bool ignoreRest = false;
    CommandLineParserArgumentDescription * currentArgDesc = 0;
    vector<CommandLineParserArgumentDescription*> parsedArgDescriptions;
    for(unsigned int i = 1; i < arguments.size(); ++i)
    {
        string argument = arguments.at(i);

        if (this->Internal->Debug) { std::cout << "Processing" << argument; }
        if (!argument.compare("--xml") || !argument.compare("-xml") || !argument.compare("--XML") || !argument.compare("-XML"))
        {
          this->generateXmlOutput();
          return map<string, us::Any>();
        }

        // should argument be ignored ?
        if (ignoreRest)
        {
            if (this->Internal->Debug)
            {
                std::cout << "  Skipping: IgnoreRest flag was been set";
            }
            this->Internal->UnparsedArguments.push_back(argument);
            continue;
        }

        // Skip if the argument does not start with the defined prefix
        if (!(argument.compare(0, Internal->LongPrefix.size(), Internal->LongPrefix)==0
              || argument.compare(0, Internal->ShortPrefix.size(), Internal->ShortPrefix)==0))
        {
            if (this->Internal->StrictMode)
            {
                this->Internal->ErrorString = "Unknown argument ";
                this->Internal->ErrorString += argument;
                error = true;
                break;
            }
            if (this->Internal->Debug)
            {
                std::cout << "  Skipping: It does not start with the defined prefix";
            }
            this->Internal->UnparsedArguments.push_back(argument);
            continue;
        }

        // Skip if argument has already been parsed ...
        bool alreadyProcessed = false;
        for (unsigned int i=0; i<Internal->ProcessedArguments.size(); i++)
            if (argument.compare(Internal->ProcessedArguments.at(i))==0)
            {
                alreadyProcessed = true;
                break;
            }

        if (alreadyProcessed)
        {
            if (this->Internal->StrictMode)
            {
                this->Internal->ErrorString = "Argument ";
                this->Internal->ErrorString += argument;
                this->Internal->ErrorString += " already processed !";
                error = true;
                break;
            }
            if (this->Internal->Debug)
            {
                std::cout << "  Skipping: Already processed !";
            }
            continue;
        }

        // Retrieve corresponding argument description
        currentArgDesc = this->Internal->argumentDescription(argument);

        // Is there a corresponding argument description ?
        if (currentArgDesc)
        {
            // If the argument is deprecated, print the help text but continue processing
            if (currentArgDesc->Deprecated)
            {
                std::cout << "Deprecated argument " << argument << ": "  << currentArgDesc->ArgHelp;
            }
            else
            {
                parsedArgDescriptions.push_back(currentArgDesc);
            }

            this->Internal->ProcessedArguments.push_back(currentArgDesc->ShortArg);
            this->Internal->ProcessedArguments.push_back(currentArgDesc->LongArg);
            int numberOfParametersToProcess = currentArgDesc->NumberOfParametersToProcess;
            ignoreRest = currentArgDesc->IgnoreRest;
            if (this->Internal->Debug && ignoreRest)
            {
                std::cout << "  IgnoreRest flag is True";
            }

            // Is the number of parameters associated with the argument being processed known ?
            if (numberOfParametersToProcess == 0)
            {
                currentArgDesc->addParameter("true");
            }
            else if (numberOfParametersToProcess > 0)
            {
                string missingParameterError =
                        "Argument %1 has %2 value(s) associated whereas exacly %3 are expected.";
                for(int j=1; j <= numberOfParametersToProcess; ++j)
                {
                    if (i + j >= arguments.size())
                    {
//                        this->Internal->ErrorString =
//                                missingParameterError.arg(argument).arg(j-1).arg(numberOfParametersToProcess);
//                        if (this->Internal->Debug) { std::cout << this->Internal->ErrorString; }
                        if (ok) { *ok = false; }
                        return map<string, us::Any>();
                    }
                    string parameter = arguments.at(i + j);
                    if (this->Internal->Debug)
                    {
                        std::cout << "  Processing parameter" << j << ", value:" << parameter;
                    }
                    if (this->argumentAdded(parameter))
                    {
//                        this->Internal->ErrorString =
//                                missingParameterError.arg(argument).arg(j-1).arg(numberOfParametersToProcess);
//                        if (this->Internal->Debug) { std::cout << this->Internal->ErrorString; }
                        if (ok) { *ok = false; }
                        return map<string, us::Any>();
                    }
                    if (!currentArgDesc->addParameter(parameter))
                    {
//                        this->Internal->ErrorString = string(
//                                    "Value(s) associated with argument %1 are incorrect. %2").
//                                arg(argument).arg(currentArgDesc->ExactMatchFailedMessage);
//                        if (this->Internal->Debug) { std::cout << this->Internal->ErrorString; }
                        if (ok) { *ok = false; }
                        return map<string, us::Any>();
                    }
                }
                // Update main loop increment
                i = i + numberOfParametersToProcess;
            }
            else if (numberOfParametersToProcess == -1)
            {
                if (this->Internal->Debug)
                {
                    std::cout << "  Proccessing StringList ...";
                }
                int j = 1;
                while(j + i < arguments.size())
                {
                    if (this->argumentAdded(arguments.at(j + i)))
                    {
                        if (this->Internal->Debug)
                        {
                            std::cout << "  No more parameter for" << argument;
                        }
                        break;
                    }
                    string parameter = arguments.at(j + i);

                    if (parameter.compare(0, Internal->LongPrefix.size(), Internal->LongPrefix)==0
                        || parameter.compare(0, Internal->ShortPrefix.size(), Internal->ShortPrefix)==0)
                    {
                        j--;
                        break;
                    }

                    if (this->Internal->Debug)
                    {
                        std::cout << "  Processing parameter" << j << ", value:" << parameter;
                    }
                    if (!currentArgDesc->addParameter(parameter))
                    {
//                        this->Internal->ErrorString = string(
//                                    "Value(s) associated with argument %1 are incorrect. %2").
//                                arg(argument).arg(currentArgDesc->ExactMatchFailedMessage);
//                        if (this->Internal->Debug) { std::cout << this->Internal->ErrorString; }
                        if (ok) { *ok = false; }
                        return map<string, us::Any>();
                    }
                    j++;
                }
                // Update main loop increment
                i = i + j;
            }
        }
        else
        {
            if (this->Internal->StrictMode)
            {
                this->Internal->ErrorString = "Unknown argument ";
                this->Internal->ErrorString += argument;
                error = true;
                break;
            }
            if (this->Internal->Debug)
            {
                std::cout << "  Skipping: Unknown argument";
            }
            this->Internal->UnparsedArguments.push_back(argument);
        }
    }

    if (ok)
    {
        *ok = !error;
    }

    map<string, us::Any> parsedArguments;

    int obligatoryArgs = 0;
    vector<CommandLineParserArgumentDescription*>::iterator it;
    for(it = Internal->ArgumentDescriptionList.begin(); it != Internal->ArgumentDescriptionList.end();++it)
    {
        CommandLineParserArgumentDescription* desc = *it;

        if(!desc->Optional)
            obligatoryArgs++;
    }

    int parsedObligatoryArgs = 0;
    for(it = parsedArgDescriptions.begin(); it != parsedArgDescriptions.end();++it)
    {
        CommandLineParserArgumentDescription* desc = *it;

        string key;
        if (!desc->LongArg.empty())
        {
            key = desc->LongArg;
        }
        else
        {
            key = desc->ShortArg;
        }

        if(!desc->Optional)
            parsedObligatoryArgs++;

        std::pair<string, us::Any> elem; elem.first = key; elem.second = desc->Value;
        parsedArguments.insert(elem);
    }

    if (obligatoryArgs>parsedObligatoryArgs)
    {
        parsedArguments.clear();
        cout << helpText();
    }

    return parsedArguments;
}

// -------------------------------------------------------------------------
map<string, us::Any> mitkCommandLineParser::parseArguments(int argc, char** argv, bool* ok)
{
    StringContainerType arguments;

    // Create a StringContainerType of arguments
    for(int i = 0; i < argc; ++i)
        arguments.push_back(argv[i]);

    return this->parseArguments(arguments, ok);
}

// -------------------------------------------------------------------------
string mitkCommandLineParser::errorString() const
{
    return this->Internal->ErrorString;
}

// -------------------------------------------------------------------------
const mitkCommandLineParser::StringContainerType& mitkCommandLineParser::unparsedArguments() const
{
    return this->Internal->UnparsedArguments;
}

// --------------------------------------------------------------------------
void mitkCommandLineParser::addArgument(const string& longarg, const string& shortarg,
                                       Type type, const string& argLabel, const string& argHelp,
                                       const us::Any& defaultValue, bool optional, bool ignoreRest,
                                       bool deprecated)
{
    if (longarg.empty() && shortarg.empty()) { return; }

    /* Make sure it's not already added */
    bool added = this->Internal->ArgNameToArgumentDescriptionMap.count(longarg);
    if (added) { return; }

    added = this->Internal->ArgNameToArgumentDescriptionMap.count(shortarg);
    if (added) { return; }

    CommandLineParserArgumentDescription* argDesc =
            new CommandLineParserArgumentDescription(longarg, this->Internal->LongPrefix,
                                                     shortarg, this->Internal->ShortPrefix, type,
                                                     argHelp, argLabel, defaultValue, ignoreRest, deprecated, optional, ParameterGroupName, ParameterGroupDescription);

    int argWidth = 0;
    if (!longarg.empty())
    {
        this->Internal->ArgNameToArgumentDescriptionMap[longarg] = argDesc;
        argWidth += longarg.size() + this->Internal->LongPrefix.size();
    }
    if (!shortarg.empty())
    {
        this->Internal->ArgNameToArgumentDescriptionMap[shortarg] = argDesc;
        argWidth += shortarg.size() + this->Internal->ShortPrefix.size() + 2;
    }
    argWidth += 5;

    // Set the field width for the arguments
    if (argWidth > this->Internal->FieldWidth)
    {
        this->Internal->FieldWidth = argWidth;
    }

    this->Internal->ArgumentDescriptionList.push_back(argDesc);
    this->Internal->GroupToArgumentDescriptionListMap[this->Internal->CurrentGroup].push_back(argDesc);
}

// --------------------------------------------------------------------------
void mitkCommandLineParser::addDeprecatedArgument(
        const string& longarg, const string& shortarg, const string& argLabel, const string& argHelp)
{
    addArgument(longarg, shortarg, StringList, argLabel, argHelp, us::Any(), false, true, false);
}

// --------------------------------------------------------------------------
int mitkCommandLineParser::fieldWidth() const
{
    return this->Internal->FieldWidth;
}

// --------------------------------------------------------------------------
void mitkCommandLineParser::beginGroup(const string& description)
{
    this->Internal->CurrentGroup = description;
}

// --------------------------------------------------------------------------
void mitkCommandLineParser::endGroup()
{
    this->Internal->CurrentGroup.clear();
}

// --------------------------------------------------------------------------
string mitkCommandLineParser::helpText() const
{
    string text;
    vector<CommandLineParserArgumentDescription*> deprecatedArgs;

    // Loop over grouped argument descriptions
    map<string, vector<CommandLineParserArgumentDescription*> >::iterator it;
    for(it = Internal->GroupToArgumentDescriptionListMap.begin(); it != Internal->GroupToArgumentDescriptionListMap.end();++it)
    {
        if (!(*it).first.empty())
        {
            text = text + "\n" + (*it).first + "\n";
        }

        vector<CommandLineParserArgumentDescription*>::iterator it2;
        for(it2 = (*it).second.begin(); it2 != (*it).second.end(); ++it2)
        {
            CommandLineParserArgumentDescription* argDesc = *it2;
            if (argDesc->Deprecated)
            {
                deprecatedArgs.push_back(argDesc);
            }
            else
            {
                text += argDesc->helpText();
            }
        }
    }

    if (!deprecatedArgs.empty())
    {
        text += "\nDeprecated arguments:\n";
        vector<CommandLineParserArgumentDescription*>::iterator it2;
        for(it2 = deprecatedArgs.begin(); it2 != deprecatedArgs.end(); ++it2)
        {
            CommandLineParserArgumentDescription* argDesc = *it2;
            text += argDesc->helpText();
        }
    }

    return text;
}

// --------------------------------------------------------------------------
bool mitkCommandLineParser::argumentAdded(const string& argument) const
{
    return this->Internal->ArgNameToArgumentDescriptionMap.count(argument);
}

// --------------------------------------------------------------------------
bool mitkCommandLineParser::argumentParsed(const string& argument) const
{
    for (unsigned int i=0; i<Internal->ProcessedArguments.size(); i++)
        if (argument.compare(Internal->ProcessedArguments.at(i))==0)
            return true;
    return false;
}

// --------------------------------------------------------------------------
void mitkCommandLineParser::setArgumentPrefix(const string& longPrefix, const string& shortPrefix)
{
    this->Internal->LongPrefix = longPrefix;
    this->Internal->ShortPrefix = shortPrefix;
}

// --------------------------------------------------------------------------
void mitkCommandLineParser::setStrictModeEnabled(bool strictMode)
{
    this->Internal->StrictMode = strictMode;
}

void mitkCommandLineParser::generateXmlOutput()
{
  std::stringstream xml;

  xml << "<executable>" << endl;
  xml << "<category>" << Category << "</category>" << endl;
  xml << "<title>" << Title <<"</title>" << endl;
  xml << "<description>" << Description << "</description>" << endl;
  xml << "<contributor>" << Contributor << "</contributor>" << endl;
  xml << "<parameters>" << endl;

  std::vector<CommandLineParserArgumentDescription*>::iterator it;

  std::string lastParameterGroup = "";
  for (it = this->Internal->ArgumentDescriptionList.begin(); it != this->Internal->ArgumentDescriptionList.end(); it++)
  {
    std::string type;
    switch ((*it)->ValueType)
    {
      case mitkCommandLineParser::String:
        type = "string";
        break;

      case mitkCommandLineParser::Bool:
        type = "boolean";
        break;

      case mitkCommandLineParser::StringList:
        type = "string-vector";
        break;

      case mitkCommandLineParser::Int:
        type = "integer";
        break;

      case mitkCommandLineParser::Float:
        type = "float";
        break;

      case mitkCommandLineParser::OutputDirectory:
      case mitkCommandLineParser::InputDirectory:
        type = "directory";
        break;

      case mitkCommandLineParser::InputImage:
        type = "image";
        break;

      case mitkCommandLineParser::OutputFile:
      case mitkCommandLineParser::InputFile:
        type = "file";
        break;
    }

    if (lastParameterGroup.compare((*it)->ArgGroup))
    {
      if (it != this->Internal->ArgumentDescriptionList.begin())
      {
        xml << "</parameters>" << endl;
        xml << "<parameters>" << endl;
      }
      xml << "<label>" << (*it)->ArgGroup << "</label>" << endl;
      xml << "<description>" << (*it)->ArgGroupDescription << "</description>" << endl;
      lastParameterGroup = (*it)->ArgGroup;
    }

    xml << "<" << type << ">" << endl;
    xml << "<name>" << (*it)->LongArg << "</name>" << endl;
    xml << "<label>" << (*it)->ArgLabel << "</label>" << endl;
    xml << "<description>" << (*it)->ArgHelp << "</description>" << endl;
    xml << "<longflag>" << (*it)->LongArg << "</longflag>" << endl;
    xml << "<flag>" << (*it)->ShortArg << "</flag>" << endl;

    if ((*it)->ValueType == mitkCommandLineParser::InputDirectory || (*it)->ValueType == mitkCommandLineParser::InputFile || (*it)->ValueType == mitkCommandLineParser::InputImage)
    {
      xml << "<channel>input</channel>" << endl;
    }
    else if ((*it)->ValueType == mitkCommandLineParser::OutputDirectory || (*it)->ValueType == mitkCommandLineParser::OutputFile)
    {
      xml << "<channel>output</channel>" << endl;
    }
    xml << "</" << type << ">" << endl;
  }

  xml << "</parameters>" << endl;
  xml << "</executable>" << endl;

  cout << xml.str();
}

void mitkCommandLineParser::setTitle(string title)
{
  Title = title;
}
void mitkCommandLineParser::setContributor(string contributor)
{
  Contributor = contributor;
}

void mitkCommandLineParser::setCategory(string category)
{
  Category = category;
}

void mitkCommandLineParser::setDescription(string description)
{
  Description = description;
}

void mitkCommandLineParser::changeParameterGroup(string name, string tooltip)
{
  ParameterGroupName = name;
  ParameterGroupDescription = tooltip;
}
