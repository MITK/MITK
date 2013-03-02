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

typedef std::vector<std::pair<std::string::size_type, std::string::size_type> > TemplateIndex;

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
  std::vector<std::string> staticContent;
  std::string::size_type index = 0;

  for (TemplateIndex::const_iterator it = templateIndex.begin(); it != templateIndex.end(); ++it)
  {
    staticContent.push_back(contents.substr(index, it->first - index));
    index = it->first + it->second;
  }

  staticContent.push_back(contents.substr(index));

  return staticContent;
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

  std::swap(m_StaticContents, staticContents);
  m_IsInitialized = true;

  return true;
}

bool mitk::SimulationTemplate::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
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
