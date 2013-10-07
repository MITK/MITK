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

#include "mitkSimulationIOFactory.h"
#include "mitkSimulationReader.h"
#include <mitkIOAdapter.h>
#include <itkVersion.h>

mitk::SimulationIOFactory::SimulationIOFactory()
{
  this->RegisterOverride(
    "mitkIOAdapter",
    "mitkSimulationReader",
    "Simulation IO Factory",
    true,
    itk::CreateObjectFunction<IOAdapter<SimulationReader> >::New());
}

mitk::SimulationIOFactory::~SimulationIOFactory()
{
}

const char* mitk::SimulationIOFactory::GetDescription() const
{
  return "Simulation IO Factory";
}

const char* mitk::SimulationIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}
