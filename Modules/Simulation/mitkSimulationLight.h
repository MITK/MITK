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

#ifndef mitkSimulationLight_h
#define mitkSimulationLight_h

#include <SimulationExports.h>
#include <sofa/core/visual/VisualModel.h>

namespace mitk
{
  class Simulation_EXPORT SimulationLight : public sofa::core::visual::VisualModel
  {
  public:
    SOFA_CLASS(SimulationLight, sofa::core::visual::VisualModel);

  private:
    SimulationLight();
    ~SimulationLight();

    SimulationLight(const MyType&);
    MyType& operator=(const MyType&);
  };
}

#endif
