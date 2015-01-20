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
#include "mitkSimulationIO.h"
#include <mitkCustomMimeType.h>
#include <mitkStringProperty.h>
#include <sofa/helper/system/SetDirectory.h>
#include <algorithm>

#include <mitkIOMimeTypes.h>

static mitk::CustomMimeType CreateSimulationMimeType()
{
  mitk::CustomMimeType mimeType(mitk::IOMimeTypes::DEFAULT_BASE_NAME() + ".simulation.sofa.scene");
  mimeType.SetCategory("Simulation");
  mimeType.AddExtension("scn");
  mimeType.AddExtension("xml");
  mimeType.SetComment("SOFA Scene File");

  return mimeType;
}

mitk::SimulationIO::SimulationIO()
  : AbstractFileIO(Simulation::GetStaticNameOfClass(), CreateSimulationMimeType(), "SOFA Simulation Scene")
{
  this->RegisterService();
}

std::vector<mitk::BaseData::Pointer> mitk::SimulationIO::Read()
{
  Simulation::Pointer simulation = Simulation::New();
  sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSOFASimulation();

  ISimulationService* simulationService = GetSimulationService();
  Simulation::Pointer lastActiveSimulation = simulationService->GetActiveSimulation();

  simulationService->SetActiveSimulation(simulation);

  std::ifstream scnFile(this->GetLocalFileName().c_str());
  std::string content = std::string((std::istreambuf_iterator<char>(scnFile)), std::istreambuf_iterator<char>());
  scnFile.close();

  std::istringstream stream(content);
  std::string firstLine;

  if (!std::getline(stream, firstLine).good())
    mitkThrow() << "Could not load '" << this->GetLocalFileName() << "'!";

  std::string originalPath;

  if (firstLine.size() > 21 && firstLine.substr(0, 21) == "<!-- ORIGINAL_PATH = ")
  {
    originalPath = firstLine.substr(21);
    sofa::helper::system::DataRepository.addFirstPath(originalPath);
  }

  std::string path = sofa::helper::system::SetDirectory::GetParentDir(this->GetLocalFileName().c_str());
  sofa::helper::system::DataRepository.addFirstPath(path);

  sofa::simulation::Node::SPtr rootNode = sofa::core::objectmodel::SPtr_dynamic_cast<sofa::simulation::Node>(sofaSimulation->load(this->GetLocalFileName().c_str()));

  if (!rootNode)
  {
    sofa::helper::system::DataRepository.removePath(path);
    mitkThrow() << "Could not load '" << this->GetLocalFileName() << "'!";
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

  std::vector<BaseData::Pointer> returnValue;
  returnValue.push_back(simulation.GetPointer());

  return returnValue;
}

mitk::IFileIO::ConfidenceLevel mitk::SimulationIO::GetReaderConfidenceLevel() const
{
  if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported)
    return Unsupported;

  std::string inputLocation = this->GetInputLocation();
  std::string::size_type length = inputLocation.length();

  if (length < 5)
    return Unsupported;

  std::string ext = inputLocation.substr(length - 4);
  std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

  return ext != ".scn" && ext != ".xml"
    ? Unsupported
    : Supported;
}

void mitk::SimulationIO::Write()
{
  this->ValidateOutputLocation();

  const Simulation* simulation = dynamic_cast<const Simulation*>(this->GetInput());

  if (simulation == NULL)
    mitkThrow() << "Invalid input!";

  std::string originalContent = simulation->GetProperty("Scene File")->GetValueAsString();

  LocalFile localFile(this);
  std::ofstream scnFile(localFile.GetFileName().c_str());

  if (originalContent.size() < 22 || originalContent.substr(0, 21) != "<!-- ORIGINAL_PATH = ")
  {
    std::string originalPath = "<!-- ORIGINAL_PATH = ";
    originalPath += simulation->GetProperty("Path")->GetValueAsString();
    originalPath += "\n     This comment is parsed by MITK. -->\n";

    scnFile << originalPath;
  }

  scnFile << originalContent;
}

mitk::IFileIO::ConfidenceLevel mitk::SimulationIO::GetWriterConfidenceLevel() const
{
  if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported)
    return Unsupported;

  return dynamic_cast<const Simulation*>(this->GetInput()) == NULL
    ? Unsupported
    : Supported;
}

mitk::SimulationIO* mitk::SimulationIO::IOClone() const
{
  return new SimulationIO(*this);
}
