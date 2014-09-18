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

#include "mitkGetSimulationService.h"
#include "mitkISimulationService.h"
#include "mitkSimulation.h"
#include "mitkSimulationReader.h"
#include <mitkStringProperty.h>
#include <sofa/helper/system/SetDirectory.h>
#include <algorithm>

bool mitk::SimulationReader::CanReadFile(const std::string& filename, const std::string&, const std::string&)
{
  std::string::size_type length = filename.length();

  if (length < 5)
    return false;

  std::string ext = filename.substr(length - 4);
  std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

  if (ext == ".scn" || ext == ".xml")
    return true;

  return false;
}

mitk::SimulationReader::SimulationReader()
{
  mitk::Simulation::Pointer output = mitk::Simulation::New();

  this->SetNumberOfRequiredOutputs(1);
  this->SetNthOutput(0, output.GetPointer());
}

mitk::SimulationReader::~SimulationReader()
{
}

void mitk::SimulationReader::GenerateData()
{
  Simulation::Pointer simulation = static_cast<mitk::Simulation*>(this->GetOutput());
  sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSOFASimulation();

  ISimulationService* simulationService = GetSimulationService();
  Simulation::Pointer lastActiveSimulation = simulationService->GetActiveSimulation();

  simulationService->SetActiveSimulation(simulation);

  std::ifstream scnFile(m_FileName.c_str());
  std::string content = std::string((std::istreambuf_iterator<char>(scnFile)), std::istreambuf_iterator<char>());
  scnFile.close();

  std::istringstream stream(content);
  std::string firstLine;

  if (!std::getline(stream, firstLine).good())
    mitkThrow() << "Could not load '" << m_FileName << "'!";

  std::string originalPath;

  if (firstLine.size() > 21 && firstLine.substr(0, 21) == "<!-- ORIGINAL_PATH = ")
  {
    originalPath = firstLine.substr(21);
    sofa::helper::system::DataRepository.addFirstPath(originalPath);
  }

  std::string path = sofa::helper::system::SetDirectory::GetParentDir(m_FileName.c_str());
  sofa::helper::system::DataRepository.addFirstPath(path);

  sofa::simulation::Node::SPtr rootNode = sofa::core::objectmodel::SPtr_dynamic_cast<sofa::simulation::Node>(sofaSimulation->load(m_FileName.c_str()));

  if (!rootNode)
  {
    sofa::helper::system::DataRepository.removePath(path);
    mitkThrow() << "Could not load '" << m_FileName << "'!";
  }

  simulation->SetRootNode(rootNode);

  sofaSimulation->init(rootNode.get());
  sofaSimulation->reset(rootNode.get());
  simulation->UpdateOutputInformation();

  sofa::helper::system::DataRepository.removePath(path);

  if (!originalPath.empty())
  {
    sofa::helper::system::DataRepository.removePath(originalPath);
    simulation->SetProperty("Path", StringProperty::New(originalPath));
  }
  else
  {
    simulation->SetProperty("Path", StringProperty::New(path));
  }

  simulation->SetProperty("Scene File", StringProperty::New(content));

  simulationService->SetActiveSimulation(lastActiveSimulation);
}

void mitk::SimulationReader::GenerateOutputInformation()
{
}

const char* mitk::SimulationReader::GetFileName() const
{
  return m_FileName.c_str();
}

void mitk::SimulationReader::SetFileName(const char* aFileName)
{
  m_FileName = aFileName;
}

const char* mitk::SimulationReader::GetFilePattern() const
{
  return m_FilePattern.c_str();
}

void mitk::SimulationReader::SetFilePattern(const char* aFilePattern)
{
  m_FilePattern = aFilePattern;
}

const char* mitk::SimulationReader::GetFilePrefix() const
{
  return m_FilePrefix.c_str();
}

void mitk::SimulationReader::SetFilePrefix(const char* aFilePrefix)
{
  m_FilePrefix = aFilePrefix;
}

mitk::BaseDataSource::DataObjectPointer mitk::SimulationReader::MakeOutput(DataObjectPointerArraySizeType)
{
  return mitk::Simulation::New().GetPointer();
}

mitk::BaseDataSource::DataObjectPointer mitk::SimulationReader::MakeOutput(const DataObjectIdentifierType& name)
{
  return this->IsIndexedOutputName(name)
    ? this->MakeOutput(this->MakeIndexFromOutputName(name))
    : mitk::Simulation::New().GetPointer();
}
