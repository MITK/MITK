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


// CTK includes
#include "ctkCommandLineParser.h"

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
            ctkCommandLineParser::Type type, const string& argHelp,
            const us::Any& defaultValue, bool ignoreRest,
            bool deprecated, bool optional)
        : LongArg(longArg), LongArgPrefix(longArgPrefix),
          ShortArg(shortArg), ShortArgPrefix(shortArgPrefix),
          ArgHelp(argHelp), IgnoreRest(ignoreRest), NumberOfParametersToProcess(0),
          Deprecated(deprecated), DefaultValue(defaultValue), Value(type), ValueType(type), Optional(optional)
    {
        Value = defaultValue;

        switch (type)
        {
        case ctkCommandLineParser::String:
        {
            NumberOfParametersToProcess = 1;
        }
            break;
        case ctkCommandLineParser::Bool:
        {
            NumberOfParametersToProcess = 0;
        }
            break;
        case ctkCommandLineParser::StringList:
        {
            NumberOfParametersToProcess = -1;
        }
            break;
        case ctkCommandLineParser::Int:
        {
            NumberOfParametersToProcess = 1;
        }
            break;
        case ctkCommandLineParser::Float:
        {
            NumberOfParametersToProcess = 1;
        }
            break;
        default:
            MITK_INFO << "Type not supported: " << static_cast<int>(type);
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
    bool    IgnoreRest;
    int     NumberOfParametersToProcess;
    bool    Deprecated;
    bool    Optional;

    us::Any                   DefaultValue;
    us::Any                   Value;
    ctkCommandLineParser::Type  ValueType;
};

// --------------------------------------------------------------------------
bool CommandLineParserArgumentDescription::addParameter(const string &value)
{
    switch (ValueType)
    {
    case ctkCommandLineParser::String:
    {
        Value = value;
    }
        break;
    case ctkCommandLineParser::Bool:
    {
        if (value.compare("true")==0)
            Value = true;
        else
            Value = false;
    }
        break;
    case ctkCommandLineParser::StringList:
    {
        try
        {
            ctkCommandLineParser::StringContainerType list = us::any_cast<ctkCommandLineParser::StringContainerType>(Value);
            list.push_back(value);
            Value = list;
        }
        catch(...)
        {
            ctkCommandLineParser::StringContainerType list;
            list.push_back(value);
            Value = list;
        }
    }
        break;
    case ctkCommandLineParser::Int:
    {
        stringstream ss(value);
        int i;
        ss >> i;
        Value = i;
    }
        break;
    case ctkCommandLineParser::Float:
    {
        stringstream ss(value);
        float f;
        ss >> f;
        Value = f;
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
class ctkCommandLineParser::ctkInternal
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
ctkCommandLineParser::ctkInternal::argumentDescription(const string& argument)
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
ctkCommandLineParser::ctkCommandLineParser()
{
    this->Internal = new ctkInternal();
}

// --------------------------------------------------------------------------
ctkCommandLineParser::~ctkCommandLineParser()
{
    delete this->Internal;
}

// --------------------------------------------------------------------------
map<string, us::Any> ctkCommandLineParser::parseArguments(const StringContainerType& arguments,
                                                              bool* ok)
{
    // Reset
    this->Internal->UnparsedArguments.clear();
    this->Internal->ProcessedArguments.clear();
    this->Internal->ErrorString.clear();
    //    foreach (CommandLineParserArgumentDescription* desc, this->Internal->ArgumentDescriptionList)
    for (int i=0; i<Internal->ArgumentDescriptionList.size(); i++)
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
    for(int i = 1; i < arguments.size(); ++i)
    {
        string argument = arguments.at(i);
        if (this->Internal->Debug) { MITK_DEBUG << "Processing" << argument; }

        // should argument be ignored ?
        if (ignoreRest)
        {
            if (this->Internal->Debug)
            {
                MITK_DEBUG << "  Skipping: IgnoreRest flag was been set";
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
                MITK_DEBUG << "  Skipping: It does not start with the defined prefix";
            }
            this->Internal->UnparsedArguments.push_back(argument);
            continue;
        }

        // Skip if argument has already been parsed ...
        bool alreadyProcessed = false;
        for (int i=0; i<Internal->ProcessedArguments.size(); i++)
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
                MITK_DEBUG << "  Skipping: Already processed !";
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
                MITK_WARN << "Deprecated argument " << argument << ": "  << currentArgDesc->ArgHelp;
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
                MITK_DEBUG << "  IgnoreRest flag is True";
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
//                        if (this->Internal->Debug) { MITK_DEBUG << this->Internal->ErrorString; }
                        if (ok) { *ok = false; }
                        return map<string, us::Any>();
                    }
                    string parameter = arguments.at(i + j);
                    if (this->Internal->Debug)
                    {
                        MITK_DEBUG << "  Processing parameter" << j << ", value:" << parameter;
                    }
                    if (this->argumentAdded(parameter))
                    {
//                        this->Internal->ErrorString =
//                                missingParameterError.arg(argument).arg(j-1).arg(numberOfParametersToProcess);
//                        if (this->Internal->Debug) { MITK_DEBUG << this->Internal->ErrorString; }
                        if (ok) { *ok = false; }
                        return map<string, us::Any>();
                    }
                    if (!currentArgDesc->addParameter(parameter))
                    {
//                        this->Internal->ErrorString = string(
//                                    "Value(s) associated with argument %1 are incorrect. %2").
//                                arg(argument).arg(currentArgDesc->ExactMatchFailedMessage);
//                        if (this->Internal->Debug) { MITK_DEBUG << this->Internal->ErrorString; }
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
                    MITK_DEBUG << "  Proccessing StringList ...";
                }
                int j = 1;
                while(j + i < arguments.size())
                {
                    if (this->argumentAdded(arguments.at(j + i)))
                    {
                        if (this->Internal->Debug)
                        {
                            MITK_DEBUG << "  No more parameter for" << argument;
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
                        MITK_DEBUG << "  Processing parameter" << j << ", value:" << parameter;
                    }
                    if (!currentArgDesc->addParameter(parameter))
                    {
//                        this->Internal->ErrorString = string(
//                                    "Value(s) associated with argument %1 are incorrect. %2").
//                                arg(argument).arg(currentArgDesc->ExactMatchFailedMessage);
//                        if (this->Internal->Debug) { MITK_DEBUG << this->Internal->ErrorString; }
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
                MITK_DEBUG << "  Skipping: Unknown argument";
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
map<string, us::Any> ctkCommandLineParser::parseArguments(int argc, char** argv, bool* ok)
{
    StringContainerType arguments;

    // Create a StringContainerType of arguments
    for(int i = 0; i < argc; ++i)
        arguments.push_back(argv[i]);

    return this->parseArguments(arguments, ok);
}

// -------------------------------------------------------------------------
string ctkCommandLineParser::errorString() const
{
    return this->Internal->ErrorString;
}

// -------------------------------------------------------------------------
const ctkCommandLineParser::StringContainerType& ctkCommandLineParser::unparsedArguments() const
{
    return this->Internal->UnparsedArguments;
}

// --------------------------------------------------------------------------
void ctkCommandLineParser::addArgument(const string& longarg, const string& shortarg,
                                       Type type, const string& argHelp,
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
                                                     argHelp, defaultValue, ignoreRest, deprecated, optional);

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
void ctkCommandLineParser::addDeprecatedArgument(
        const string& longarg, const string& shortarg, const string& argHelp)
{
    addArgument(longarg, shortarg, StringList, argHelp, us::Any(), false, true);
}

// --------------------------------------------------------------------------
int ctkCommandLineParser::fieldWidth() const
{
    return this->Internal->FieldWidth;
}

// --------------------------------------------------------------------------
void ctkCommandLineParser::beginGroup(const string& description)
{
    this->Internal->CurrentGroup = description;
}

// --------------------------------------------------------------------------
void ctkCommandLineParser::endGroup()
{
    this->Internal->CurrentGroup.clear();
}

// --------------------------------------------------------------------------
string ctkCommandLineParser::helpText() const
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
bool ctkCommandLineParser::argumentAdded(const string& argument) const
{
    return this->Internal->ArgNameToArgumentDescriptionMap.count(argument);
}

// --------------------------------------------------------------------------
bool ctkCommandLineParser::argumentParsed(const string& argument) const
{
    for (int i=0; i<Internal->ProcessedArguments.size(); i++)
        if (argument.compare(Internal->ProcessedArguments.at(i))==0)
            return true;
    return false;
}

// --------------------------------------------------------------------------
void ctkCommandLineParser::setArgumentPrefix(const string& longPrefix, const string& shortPrefix)
{
    this->Internal->LongPrefix = longPrefix;
    this->Internal->ShortPrefix = shortPrefix;
}

// --------------------------------------------------------------------------
void ctkCommandLineParser::setStrictModeEnabled(bool strictMode)
{
    this->Internal->StrictMode = strictMode;
}

