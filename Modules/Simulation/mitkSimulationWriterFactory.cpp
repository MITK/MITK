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

#include "mitkSimulationWriterFactory.h"
#include <itkVersion.h>
#include <mitkSimulationWriter.h>

namespace mitk
{
  template <class T>
  class CreateSimulationWriterFunction : public itk::CreateObjectFunctionBase
  {
  public:
    mitkClassMacro(CreateSimulationWriterFunction, itk::CreateObjectFunctionBase);
    itkFactorylessNewMacro(Self);

    itk::LightObject::Pointer CreateObject()
    {
      typename T::Pointer simulationWriter = T::New();
      simulationWriter->Register();
      return simulationWriter.GetPointer();
    }

  protected:
    CreateSimulationWriterFunction()
    {
    }

    ~CreateSimulationWriterFunction()
    {
    }
  };
}

mitk::SimulationWriterFactory::SimulationWriterFactory()
{
  this->RegisterOverride(
    "IOWriter",
    "SimulationWriter",
    "SOFA scene file writer",
    true,
    CreateSimulationWriterFunction<SimulationWriter>::New());
}

mitk::SimulationWriterFactory::~SimulationWriterFactory()
{
}

const char* mitk::SimulationWriterFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* mitk::SimulationWriterFactory::GetDescription() const
{
  return "SimulationWriterFactory";
}
