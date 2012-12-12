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

#include "mitkSimulation.h"
#include "mitkSimulationReader.h"
#include <algorithm>
#include <sofa/helper/system/SetDirectory.h>

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
  Simulation::Pointer simulation = dynamic_cast<mitk::Simulation*>(this->GetOutput(0));
  sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSimulation();

  sofa::simulation::Simulation::SPtr currentSofaSimulation = sofa::simulation::getSimulation();
  sofa::core::visual::VisualParams* visualParams = sofa::core::visual::VisualParams::defaultInstance();
  sofa::core::visual::DrawTool* currentDrawTool = visualParams->drawTool();

  simulation->SetAsActiveSimulation();

  std::string path = sofa::helper::system::SetDirectory::GetParentDir(m_FileName.c_str());
  sofa::helper::system::DataRepository.addFirstPath(path);
  sofa::simulation::Node::SPtr rootNode = sofa::core::objectmodel::SPtr_dynamic_cast<sofa::simulation::Node>(sofaSimulation->loadFromFile(m_FileName.c_str()));

  if (rootNode == NULL)
  {
    sofa::helper::system::DataRepository.removePath(path);
    mitkThrow() << "Could not load '" << m_FileName << "'!";
  }

  simulation->SetRootNode(rootNode.get());
  simulation->SetDefaultDT(rootNode->getDt());
  sofaSimulation->init(rootNode.get());

  sofa::helper::system::DataRepository.removePath(path);

  sofa::simulation::setSimulation(currentSofaSimulation.get());
  visualParams->drawTool() = currentDrawTool;
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
