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

#include "mitkIndexROI.h"
#include "mitkSimulation.h"
#include "mitkSimulationObjectFactory.h"
#include "mitkSimulationVtkMapper3D.h"
#include "mitkSimulationWriter.h"
#include "mitkVtkModel.h"
#include <mitkCoreObjectFactory.h>
#include <sofa/helper/system/glut.h>
#include <sofa/component/init.h>
#include <sofa/core/ObjectFactory.h>
#include <sofa/core/visual/VisualParams.h>
#include <sofa/simulation/common/xml/initXml.h>

static void InitializeSofa()
{
  int argc = 0;
  glutInit(&argc, NULL);

  sofa::component::init();
  sofa::simulation::xml::initXml();

  sofa::core::visual::VisualParams::defaultInstance()->setSupported(sofa::core::visual::API_OpenGL);
}

static void RegisterSofaClasses()
{
  using sofa::core::ObjectFactory;
  using sofa::core::RegisterObject;

  int IndexROIClass = RegisterObject("").add<mitk::IndexROI>();
  int VtkModelClass = RegisterObject("").add<mitk::VtkModel>();

  ObjectFactory::AddAlias("VisualModel", "VtkModel", true);
  ObjectFactory::AddAlias("OglModel", "VtkModel", true);
}

mitk::SimulationObjectFactory::SimulationObjectFactory()
  : m_SimulationIOFactory(SimulationIOFactory::New()),
    m_SimulationWriterFactory(SimulationWriterFactory::New())
{
  itk::ObjectFactoryBase::RegisterFactory(m_SimulationIOFactory);
  itk::ObjectFactoryBase::RegisterFactory(m_SimulationWriterFactory);

  m_FileWriters.push_back(SimulationWriter::New().GetPointer());

  std::string description = "SOFA Scene Files";
  m_FileExtensionsMap.insert(std::pair<std::string, std::string>("*.scn", description));

  InitializeSofa();
  RegisterSofaClasses();
}

mitk::SimulationObjectFactory::~SimulationObjectFactory()
{
  itk::ObjectFactoryBase::UnRegisterFactory(m_SimulationWriterFactory);
  itk::ObjectFactoryBase::UnRegisterFactory(m_SimulationIOFactory);
}

mitk::Mapper::Pointer mitk::SimulationObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId slotId)
{
  Mapper::Pointer mapper;

  if (dynamic_cast<Simulation*>(node->GetData()) != NULL)
  {
    if (slotId == BaseRenderer::Standard3D)
      mapper = mitk::SimulationVtkMapper3D::New();

    if (mapper.IsNotNull())
      mapper->SetDataNode(node);
  }

  return mapper;
}

const char* mitk::SimulationObjectFactory::GetDescription() const
{
  return "Simulation Object Factory";
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
  if (node == NULL)
    return;

  if (dynamic_cast<Simulation*>(node->GetData()) != NULL)
    SimulationVtkMapper3D::SetDefaultProperties(node);
}

void mitk::RegisterSimulationObjectFactory()
{
  static bool alreadyRegistered = false;

  if (!alreadyRegistered)
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(mitk::SimulationObjectFactory::New());
    alreadyRegistered = true;
  }
}
