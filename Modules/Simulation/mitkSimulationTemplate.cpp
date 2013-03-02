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

  if (templateIndex.empty())
    mitkThrow() << "No templates found!";

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

static std::pair<std::string, mitk::BaseProperty::Pointer> ParseTemplate(const std::string& templ)
{
  std::string name = ParseValue(templ, "name");

  if (!name.empty())
  {
    std::string type = ParseValue(templ, "type");

    if (!type.empty())
    {
      name = "Template." + name;

      mitk::BaseProperty::Pointer property;
      std::string defaultValue = ParseValue(templ, "default");

      if (type == "double" || type == "float")
      {
        float value = !defaultValue.empty()
          ? FromString<float>(defaultValue)
          : 0.0;

        property = mitk::FloatProperty::New(value);
      }
      else if (type == "int" || type == "unsigned int")
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
        return std::make_pair(name, property);
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

mitk::SimulationTemplate::SimulationTemplate()
  : m_IsInitialized(false)
{
}

mitk::SimulationTemplate::~SimulationTemplate()
{
}

bool mitk::SimulationTemplate::Parse(const std::string& contents)
{
  if (m_IsInitialized)
    return false;

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

void mitk::SimulationTemplate::SetProperties(mitk::DataNode::Pointer dataNode) const
{
  if (dataNode.IsNull())
    return;

  if (!m_IsInitialized)
  {
    MITK_ERROR << "Simulation template is not initialized!";
    return;
  }

  if (dynamic_cast<SimulationTemplate*>(dataNode->GetData()) != this)
  {
    MITK_ERROR << "Data node does not own this simulation template!";
    return;
  }

  for(VariableContents::const_iterator it = m_VariableContents.begin(); it != m_VariableContents.end(); ++it)
    dataNode->SetProperty(it->first.c_str(), it->second.GetPointer());
}

void mitk::SimulationTemplate::SetRequestedRegion(itk::DataObject*)
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
