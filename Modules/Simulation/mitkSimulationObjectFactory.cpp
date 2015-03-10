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
#include "mitkSimulationVtkMapper2D.h"
#include "mitkSimulationVtkMapper3D.h"
#include "mitkVtkModel.h"
#include <mitkCoreObjectFactory.h>
#include <boost/core/ignore_unused.hpp>
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
  boost::ignore_unused(IndexROIClass, VtkModelClass);

  ObjectFactory::AddAlias("VisualModel", "VtkModel", true);
  ObjectFactory::AddAlias("OglModel", "VtkModel", true);
}

mitk::SimulationObjectFactory::SimulationObjectFactory()
{
  InitializeSofa();
  RegisterSofaClasses();
}

mitk::SimulationObjectFactory::~SimulationObjectFactory()
{
}

mitk::Mapper::Pointer mitk::SimulationObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId slotId)
{
  Mapper::Pointer mapper;

  if (dynamic_cast<Simulation*>(node->GetData()) != NULL)
  {
    if (slotId == BaseRenderer::Standard2D)
    {
      mapper = SimulationVtkMapper2D::New();
    }
    else if (slotId == BaseRenderer::Standard3D)
    {
      mapper = SimulationVtkMapper3D::New();
    }

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
  return NULL;
}

mitk::CoreObjectFactoryBase::MultimapType mitk::SimulationObjectFactory::GetFileExtensionsMap()
{
  return MultimapType();
}

const char* mitk::SimulationObjectFactory::GetSaveFileExtensions()
{
  return NULL;
}

mitk::CoreObjectFactoryBase::MultimapType mitk::SimulationObjectFactory::GetSaveFileExtensionsMap()
{
  return MultimapType();
}

void mitk::SimulationObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  if (node == NULL)
    return;

  if (dynamic_cast<Simulation*>(node->GetData()) != NULL)
  {
    SimulationVtkMapper2D::SetDefaultProperties(node);
    SimulationVtkMapper3D::SetDefaultProperties(node);
  }
}

void mitk::RegisterSimulationObjectFactory()
{
  static bool alreadyRegistered = false;

  if (!alreadyRegistered)
  {
    CoreObjectFactory::GetInstance()->RegisterExtraFactory(SimulationObjectFactory::New());
    alreadyRegistered = true;
  }
}
