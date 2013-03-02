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

#include "mitkSimulationTemplateIOFactory.h"
#include "mitkSimulationTemplateReader.h"
#include <itkVersion.h>
#include <mitkIOAdapter.h>

mitk::SimulationTemplateIOFactory::SimulationTemplateIOFactory()
{

  this->RegisterOverride(
    "mitkIOAdapter",
    "mitkSimulationTemplateReader",
    "Simulation Template IO",
    true,
    itk::CreateObjectFunction<IOAdapter<SimulationTemplateReader> >::New());
}

mitk::SimulationTemplateIOFactory::~SimulationTemplateIOFactory()
{
}

const char* mitk::SimulationTemplateIOFactory::GetDescription() const
{
  return "mitk::SimulationTemplateIOFactory";
}

const char* mitk::SimulationTemplateIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}
