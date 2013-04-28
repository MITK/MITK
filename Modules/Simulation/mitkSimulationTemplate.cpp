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

typedef std::vector<std::pair<std::string::size_type, std::string::size_type> > TemplateIndex;
typedef std::vector<std::pair<std::string, mitk::BaseProperty::Pointer> > VariableContents;

static TemplateIndex CreateTemplateIndex(const std::string& contents)
{
  TemplateIndex templateIndex;

  std::string::size_type begin = 0;
  begin = contents.find_first_of('{', 0);

  while (begin != std::string::npos)
  {
    std::string::size_type end = contents.find_first_of('}', begin);

    if (end == std::string::npos)
      mitkThrow() << "Expected closing brace before end of file!";

    templateIndex.push_back(std::make_pair(begin, ++end - begin));

    begin = contents.find_first_of('{', end);
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

static std::string ParseValue(const std::string& templ, const std::string& valueName)
{
  const char* spaces = " \t\n";
  std::string::size_type index = templ.find(valueName);

  if (index != std::string::npos)
  {
    index += valueName.length();
    index = templ.find_first_not_of(spaces, index);

    if (index != std::string::npos && templ[index] == '=')
    {
      ++index;
      index = templ.find_first_not_of(spaces, index);

      if (index != std::string::npos && templ[index] == '\'')
      {
        ++index;
        std::string::size_type length = templ.find_first_of('\'', index);

        if (length != std::string::npos)
        {
          length -= index;
          return templ.substr(index, length);
        }
      }
    }
  }

  return "";
}

template <typename T> T FromString(const std::string& string)
{
  std::istringstream stream(string);

  T value;
  stream >> value;

  return value;
}

static std::pair<std::string, mitk::BaseProperty::Pointer> ParseReference(const std::string& ref)
{
  std::string id = "{ref}";
  mitk::StringProperty::Pointer property = mitk::StringProperty::New(ref.substr(2, ref.length() - 4));

  return std::make_pair(id, property);
}

static std::pair<std::string, mitk::BaseProperty::Pointer> ParseTemplate(const std::string& templ)
{
  if (templ.length() > 4 && templ[1] == '\'')
  {
    return ParseReference(templ);
  }
  else
  {
    std::string id = ParseValue(templ, "id");

    if (!id.empty())
    {
      std::string type = ParseValue(templ, "type");

      if (type.empty())
        type = "string";

      mitk::BaseProperty::Pointer property;
      std::string defaultValue = ParseValue(templ, "default");

      if (type == "float")
      {
        float value = !defaultValue.empty()
          ? FromString<float>(defaultValue)
          : 0.0;

        property = mitk::FloatProperty::New(value);
      }
      else if (type == "int")
      {
        int value = !defaultValue.empty()
          ? FromString<int>(defaultValue)
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

      if (property.IsNotNull())
        return std::make_pair(id, property);
    }
  }

  std::string emptyString;
  mitk::BaseProperty::Pointer nullPointer;

  return std::make_pair(emptyString, nullPointer);
}

static VariableContents ParseVariableContents(const std::string& contents, const TemplateIndex& templateIndex)
{
  VariableContents variableContents;

  for (TemplateIndex::const_iterator it = templateIndex.begin(); it != templateIndex.end(); ++it)
  {
    std::string templ = contents.substr(it->first, it->second);
    std::pair<std::string, mitk::BaseProperty::Pointer> variableContent = ParseTemplate(templ);

    if (variableContent.first.empty() || variableContent.second.IsNull())
      mitkThrow() << "Could not parse " << templ << "!";

    variableContents.push_back(variableContent);
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

    if (m_VariableContents[i].first == "{ref}")
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
    if (it->first != "{ref}")
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
