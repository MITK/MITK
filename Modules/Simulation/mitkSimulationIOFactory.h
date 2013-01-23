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

#ifndef mitkSimulationIOFactory_h
#define mitkSimulationIOFactory_h

#include <itkObjectFactoryBase.h>
#include <mitkCommon.h>
#include <SimulationExports.h>

namespace mitk
{
  class Simulation_EXPORT SimulationIOFactory : public itk::ObjectFactoryBase
  {
  public:
    mitkClassMacro(SimulationIOFactory, itk::ObjectFactoryBase);
    itkFactorylessNewMacro(Self);

    const char* GetDescription() const;
    const char* GetITKSourceVersion() const;

  private:
    SimulationIOFactory();
    ~SimulationIOFactory();

    SimulationIOFactory(const Self&);
    Self& operator=(const Self&);
  };
};

#endif
