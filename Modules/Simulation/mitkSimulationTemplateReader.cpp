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
#include "mitkSimulationTemplateReader.h"
#include <algorithm>
#include <fstream>
#include <string>

static std::string ReadFile(const std::string& fileName)
{
  std::ifstream file(fileName.c_str());

  if (!file.is_open())
    mitkThrow() << "Could not load '" << fileName << "'!";

  std::string contents;

  file.seekg(0, std::ios::end);
  contents.resize(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(&contents[0], contents.size());

  file.close();

  if (contents.empty())
    mitkThrow() << fileName << " is empty!";

  return contents;
}

bool mitk::SimulationTemplateReader::CanReadFile(const std::string& filename, const std::string&, const std::string&)
{
  std::string::size_type length = filename.length();

  if (length < 14)
    return false;

  std::string ext = filename.substr(length - 13);
  std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

  if (ext == ".scn.template" || ext == ".xml.template")
    return true;

  return false;
}

mitk::SimulationTemplateReader::SimulationTemplateReader()
{
  mitk::SimulationTemplate::Pointer output = mitk::SimulationTemplate::New();

  this->SetNumberOfRequiredOutputs(1);
  this->SetNthOutput(0, output.GetPointer());
}

mitk::SimulationTemplateReader::~SimulationTemplateReader()
{
}

void mitk::SimulationTemplateReader::GenerateData()
{
  SimulationTemplate::Pointer simulationTemplate = dynamic_cast<mitk::SimulationTemplate*>(this->GetOutput(0));

  std::string contents = ReadFile(m_FileName);
  simulationTemplate->Parse(contents);
}

void mitk::SimulationTemplateReader::GenerateOutputInformation()
{
}

const char* mitk::SimulationTemplateReader::GetFileName() const
{
  return m_FileName.c_str();
}

void mitk::SimulationTemplateReader::SetFileName(const char* aFileName)
{
  m_FileName = aFileName;
}

const char* mitk::SimulationTemplateReader::GetFilePattern() const
{
  return m_FilePattern.c_str();
}

void mitk::SimulationTemplateReader::SetFilePattern(const char* aFilePattern)
{
  m_FilePattern = aFilePattern;
}

const char* mitk::SimulationTemplateReader::GetFilePrefix() const
{
  return m_FilePrefix.c_str();
}

void mitk::SimulationTemplateReader::SetFilePrefix(const char* aFilePrefix)
{
  m_FilePrefix = aFilePrefix;
}

mitk::BaseDataSource::DataObjectPointer mitk::SimulationTemplateReader::MakeOutput(DataObjectPointerArraySizeType)
{
  return mitk::SimulationTemplate::New().GetPointer();
}

mitk::BaseDataSource::DataObjectPointer mitk::SimulationTemplateReader::MakeOutput(const DataObjectIdentifierType& name)
{
  return this->IsIndexedOutputName(name)
    ? this->MakeOutput(this->MakeIndexFromOutputName(name))
    : mitk::SimulationTemplate::New().GetPointer();
}
