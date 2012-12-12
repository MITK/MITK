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
#include "mitkSimulationMapper3D.h"
#include "mitkSimulationObjectFactory.h"
#include "mitkSimulationLight.h"
#include "mitkSimulationLightManager.h"
#include "mitkSimulationModel.h"
#include <mitkCoreObjectFactory.h>
#include <sofa/component/init.h>
#include <sofa/core/ObjectFactory.h>
#include <sofa/simulation/common/xml/initXml.h>

mitk::SimulationObjectFactory::SimulationObjectFactory()
  : m_SimulationIOFactory(SimulationIOFactory::New())
{
  itk::ObjectFactoryBase::RegisterFactory(m_SimulationIOFactory);

  const std::string description = "SOFA Scene Files";
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.scn", description));
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.xml", description));

  sofa::component::init();
  sofa::simulation::xml::initXml();

  int SimulationLightClass = sofa::core::RegisterObject("").add<SimulationLight>();
  sofa::core::ObjectFactory::AddAlias("DirectionalLight", "SimulationLight", true);
  sofa::core::ObjectFactory::AddAlias("PositionalLight", "SimulationLight", true);
  sofa::core::ObjectFactory::AddAlias("SpotLight", "SimulationLight", true);

  int SimulationLightManagerClass = sofa::core::RegisterObject("").add<SimulationLightManager>();
  sofa::core::ObjectFactory::AddAlias("LightManager", "SimulationLightManager", true);

  int SimulationModelClass = sofa::core::RegisterObject("").add<SimulationModel>();
  sofa::core::ObjectFactory::AddAlias("OglModel", "SimulationModel", true);
}

mitk::SimulationObjectFactory::~SimulationObjectFactory()
{
  itk::ObjectFactoryBase::UnRegisterFactory(m_SimulationIOFactory);
}

mitk::Mapper::Pointer mitk::SimulationObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId slotId)
{
  mitk::Mapper::Pointer mapper;

  if (dynamic_cast<Simulation*>(node->GetData()) != NULL)
  {
    if (slotId == mitk::BaseRenderer::Standard3D)
      mapper = mitk::SimulationMapper3D::New();

    if (mapper.IsNotNull())
      mapper->SetDataNode(node);
  }

  return mapper;
}

const char* mitk::SimulationObjectFactory::GetDescription() const
{
  return "mitk::SimulationObjectFactory";
}

const char* mitk::SimulationObjectFactory::GetFileExtensions()
{
  std::string fileExtensions;
  this->CreateFileExtensions(m_FileExtensionsMap, fileExtensions);
  return fileExtensions.c_str();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::SimulationObjectFactory::GetFileExtensionsMap()
{
  return m_FileExtensionsMap;
}

const char* mitk::SimulationObjectFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* mitk::SimulationObjectFactory::GetSaveFileExtensions()
{
  std::string saveFileExtensions;
  this->CreateFileExtensions(m_FileExtensionsMap, saveFileExtensions);
  return saveFileExtensions.c_str();
}

mitk::CoreObjectFactoryBase::MultimapType mitk::SimulationObjectFactory::GetSaveFileExtensionsMap()
{
  return m_SaveFileExtensionsMap;
}

void mitk::SimulationObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  if (node != NULL)
  {
    if (dynamic_cast<Simulation*>(node->GetData()) != NULL)
      SimulationMapper3D::SetDefaultProperties(node);
  }
}

class RegisterSimulationObjectFactory
{
public:
  RegisterSimulationObjectFactory()
    : m_Factory(mitk::SimulationObjectFactory::New())
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(m_Factory);
  }

  ~RegisterSimulationObjectFactory()
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory(m_Factory);
  }

private:
  RegisterSimulationObjectFactory(const RegisterSimulationObjectFactory&);
  RegisterSimulationObjectFactory& operator=(const RegisterSimulationObjectFactory&);

  mitk::SimulationObjectFactory::Pointer m_Factory;
};

static RegisterSimulationObjectFactory registerSimulationObjectFactory;
