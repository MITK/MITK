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

#include "mitkSimulationTemplate.h"
#include <mitkProperties.h>
#include <sstream>
#include <vector>

typedef std::vector<std::pair<std::string::size_type, std::string::size_type> > TemplateIndex;
typedef std::vector<std::pair<std::string, mitk::BaseProperty::Pointer> > VariableContents;

static TemplateIndex CreateTemplateIndex(const std::string& contents)
{
  TemplateIndex templateIndex;

  std::string::size_type begin = 0;
  begin = contents.find('{', 0);

  while (begin != std::string::npos)
  {
    std::string::size_type end = contents.find('}', begin);

    if (end == std::string::npos)
      mitkThrow() << "Could not create template index: Expected closing brace before end of file!";

    if (contents.substr(begin + 1, end - begin - 1).find('{') != std::string::npos)
      mitkThrow() << "Could not create template index: Expected closing brace before opening brace!";

    templateIndex.push_back(std::make_pair(begin, ++end - begin));

    begin = contents.find('{', end);
  }

  return templateIndex;
}

static std::vector<std::string> ParseStaticContents(const std::string& contents, const TemplateIndex& templateIndex)
{
  std::vector<std::string> staticContents;
  std::string::size_type index = 0;

  for (TemplateIndex::const_iterator it = templateIndex.begin(); it != templateIndex.end(); ++it)
  {
    staticContents.push_back(contents.substr(index, it->first - index));
    index = it->first + it->second;
  }

  staticContents.push_back(contents.substr(index));

  return staticContents;
}

static std::size_t CountSingleQuotationMarks(const std::string& string)
{
  if (string.empty())
    return 0;

  const std::string::size_type length = string.length();
  std::size_t n = string[0] == '\'' ? 1 : 0;

  for (std::string::size_type i = 1; i != length; ++i)
  {
    if (string[i] == '\'' && string[i - 1] != '\\')
      ++n;
  }

  return n;
}

static bool IsEven(std::size_t number)
{
  return number % 2 == 0;
}

static bool ValueExists(const std::string& templ, const std::string& valueName)
{
  std::string::size_type index = templ.find(valueName);
  std::string::size_type definiteIndex = std::string::npos;

  while (index != std::string::npos)
  {
    if (IsEven(CountSingleQuotationMarks(templ.substr(0, index))))
    {
      definiteIndex = index;
      break;
    }

    index = templ.find(valueName, index + 1);
  }

  return definiteIndex != std::string::npos;
}

static std::string UnescapeSingleQuotationMarks(const std::string& escapedString)
{
  std::string::size_type length = escapedString.length();
  std::string::size_type max_i = length - 2;

  std::vector<char> unescapedString;
  unescapedString.reserve(length);

  for (std::string::size_type i = 0; i != length; ++i)
  {
    char c = escapedString[i];

    if (c == '\\' && i <= max_i && escapedString[i + 1] == '\'')
      continue;

    unescapedString.push_back(c);
  }

  return std::string(unescapedString.begin(), unescapedString.end());
}

static std::string ParseValue(const std::string& templ, const std::string& valueName)
{
  const char* spaces = " \t\n";

  std::string::size_type index = templ.find(valueName);
  std::string::size_type definiteIndex = std::string::npos;

  while (index != std::string::npos)
  {
    if (IsEven(CountSingleQuotationMarks(templ.substr(0, index))))
    {
      if (definiteIndex != std::string::npos)
        mitkThrow() << "Could not parse " << templ << ": " << valueName << " is ambiguous!";

      definiteIndex = index;
    }

    index = templ.find(valueName, index + 1);
  }

  if (definiteIndex == std::string::npos)
    mitkThrow() << "Could not parse " << templ << ": " << valueName << " not found!";

  index = templ.find_first_not_of(spaces, definiteIndex + valueName.length());

  if (index == std::string::npos || templ[index] != '=')
    mitkThrow() << "Could not parse " << templ << ": Expected assignment to " << valueName << "!";

  index = templ.find_first_not_of(spaces, index + 1);

  if (index == std::string::npos || templ[index] != '\'')
    mitkThrow() << "Could not parse " << templ << ": Value of " << valueName << " is not enclosed within single quotation marks!";

  std::string::size_type index2 = std::string::npos;
  std::string::size_type length = templ.length();

  for (std::string::size_type i = ++index; i != length; ++i)
  {
    if (templ[i] == '\'' && templ[i - 1] != '\\')
    {
      index2 = i;
      break;
    }
  }

  return UnescapeSingleQuotationMarks(templ.substr(index, index2 - index));
}

template <typename T>
static T lexical_cast(const std::string& string)
{
  std::istringstream sstream(string);
  T value;

  sstream >> value;

  if (sstream.fail())
    mitkThrow() << "Lexical cast failed for '" << string << "'!";

  return value;
}

static std::pair<std::string, mitk::BaseProperty::Pointer> ParseReference(const std::string& ref)
{
  std::string id = "{}";
  std::string string = ref.substr(2, ref.length() - 4);

  if (string.empty())
    mitkThrow() << "Could not parse " << ref << ": Reference is empty!";

  return std::make_pair(id, mitk::StringProperty::New(string).GetPointer());
}

static std::pair<std::string, mitk::BaseProperty::Pointer> ParseTemplate(const std::string& templ)
{
  std::string::size_type length = templ.length();

  if (length < 4) // {''}
    mitkThrow() << "Could not parse " << templ << ": Too short to be reference or template!";

  if(!IsEven(CountSingleQuotationMarks(templ)))
    mitkThrow() << "Could not parse " << templ << ": Number of single quotation marks is odd!";

  if(templ[1] == '\'')
  {
    if (templ[length - 2] != '\'')
      mitkThrow() << "Could not parse " << templ << ": Reference does not end with '}!";

    return ParseReference(templ);
  }

  if (length < 7) // {id=''}
    mitkThrow() << "Could not parse " << templ << ": Too short to be template!";

  std::string id = ParseValue(templ, "id");

  if (id.empty())
    mitkThrow() << "Could not parse " << templ << ": Id is empty!";

  std::string type = ValueExists(templ, "type")
    ? ParseValue(templ, "type")
    : "string";

  std::string defaultValue = ValueExists(templ, "default")
    ? ParseValue(templ, "default")
    : "";

  mitk::BaseProperty::Pointer property;

  if (type == "float")
  {
    float value = !defaultValue.empty()
      ? lexical_cast<float>(defaultValue)
      : 0.0;

    property = mitk::FloatProperty::New(value);
  }
  else if (type == "int")
  {
    int value = !defaultValue.empty()
      ? lexical_cast<int>(defaultValue)
      : 0.0;

    property = mitk::IntProperty::New(value);
  }
  else if (type == "string")
  {
    std::string value = !defaultValue.empty()
      ? defaultValue
      : "";

    property = mitk::StringProperty::New(value);
  }

  if (property.IsNull())
    mitkThrow() << "Could not parse " << templ << ": Unknown type '" << type << "'!";

  return std::make_pair(id, property);
}

static VariableContents ParseVariableContents(const std::string& contents, const TemplateIndex& templateIndex)
{
  VariableContents variableContents;

  for (TemplateIndex::const_iterator it = templateIndex.begin(); it != templateIndex.end(); ++it)
  {
    std::string templ = contents.substr(it->first, it->second);
    variableContents.push_back(ParseTemplate(templ));
  }

  return variableContents;
}

template <typename T1, typename T2>
class FirstEqualTo
{
public:
  FirstEqualTo(const T1& value)
    : m_Value(value)
  {
  }

  bool operator()(const std::pair<T1, T2>& pair) const
  {
    return pair.first == m_Value;
  }

private:
  T1 m_Value;
};

mitk::SimulationTemplate::SimulationTemplate()
  : m_IsInitialized(false)
{
}

mitk::SimulationTemplate::~SimulationTemplate()
{
}

std::string mitk::SimulationTemplate::CreateSimulation() const
{
  if (!m_IsInitialized)
  {
    MITK_DEBUG << "Simulation template is not initialized!";
    return "";
  }

  std::string contents;

  for (VariableContents::size_type i = 0; i < m_VariableContents.size(); ++i)
  {
    contents += m_StaticContents[i];

    if (m_VariableContents[i].first == "{}")
    {
      VariableContents::const_iterator it = std::find_if(m_VariableContents.begin(), m_VariableContents.end(),
        FirstEqualTo<std::string, mitk::BaseProperty::Pointer>(m_VariableContents[i].second->GetValueAsString()));

      if (it == m_VariableContents.end())
      {
        MITK_DEBUG << "Template '" << m_VariableContents[i].second << "' not found!";
        return "";
      }

      contents += it->second->GetValueAsString();
    }
    else
    {
      contents += m_VariableContents[i].second->GetValueAsString();
    }
  }

  contents += m_StaticContents.back();

  return contents;
}

bool mitk::SimulationTemplate::Parse(const std::string& contents)
{
  if (m_IsInitialized)
  {
    MITK_DEBUG << "Simulation template is already initialized!";
    return false;
  }

  TemplateIndex templateIndex = CreateTemplateIndex(contents);
  std::vector<std::string> staticContents = ParseStaticContents(contents, templateIndex);
  VariableContents variableContents = ParseVariableContents(contents, templateIndex);

  std::swap(m_StaticContents, staticContents);
  std::swap(m_VariableContents, variableContents);
  m_IsInitialized = true;

  return true;
}

bool mitk::SimulationTemplate::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

bool mitk::SimulationTemplate::SetProperties(mitk::DataNode::Pointer dataNode) const
{
  if (dataNode.IsNull())
  {
    MITK_DEBUG << "Data node does not exist!";
    return false;
  }

  if (!m_IsInitialized)
  {
    MITK_DEBUG << "Simulation template is not initialized!";
    return false;
  }

  if (dynamic_cast<SimulationTemplate*>(dataNode->GetData()) != this)
  {
    MITK_DEBUG << "Data node does not own this simulation template!";
    return false;
  }

  for(VariableContents::const_iterator it = m_VariableContents.begin(); it != m_VariableContents.end(); ++it)
  {
    if (it->first != "{}")
      dataNode->SetProperty(it->first.c_str(), it->second.GetPointer());
  }

  return true;
}

void mitk::SimulationTemplate::SetRequestedRegion(const itk::DataObject*)
{
}

void mitk::SimulationTemplate::SetRequestedRegionToLargestPossibleRegion()
{
}

void mitk::SimulationTemplate::UpdateOutputInformation()
{
  if (this->GetSource().IsNotNull())
    this->GetSource()->UpdateOutputInformation();
}

bool mitk::SimulationTemplate::VerifyRequestedRegion()
{
  return true;
}
