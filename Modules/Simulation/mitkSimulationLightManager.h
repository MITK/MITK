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

#ifndef mitkSimulationLightManager_h
#define mitkSimulationLightManager_h

#include <SimulationExports.h>
#include <sofa/core/visual/VisualManager.h>

namespace mitk
{
  class Simulation_EXPORT SimulationLightManager : public sofa::core::visual::VisualManager
  {
  public:
    SOFA_CLASS(SimulationLightManager, sofa::core::visual::VisualManager);

  private:
    SimulationLightManager();
    ~SimulationLightManager();

    SimulationLightManager(const MyType&);
    MyType& operator=(const MyType&);
  };
}

#endif
