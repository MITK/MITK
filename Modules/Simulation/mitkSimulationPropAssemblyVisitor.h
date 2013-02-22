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

#ifndef mitkSimulationPropAssemblyVisitor_h
#define mitkSimulationPropAssemblyVisitor_h

#include <SimulationExports.h>
#include <sofa/simulation/common/Visitor.h>

class vtkPropAssembly;

namespace mitk
{
  class Simulation_EXPORT SimulationPropAssemblyVisitor : public sofa::simulation::Visitor
  {
  public:
    explicit SimulationPropAssemblyVisitor(vtkPropAssembly* propAssembly, const sofa::core::ExecParams* params = sofa::core::ExecParams::defaultInstance());
    ~SimulationPropAssemblyVisitor();

    Result processNodeTopDown(sofa::simulation::Node* node);

  private:
    SimulationPropAssemblyVisitor(const SimulationPropAssemblyVisitor&);
    SimulationPropAssemblyVisitor& operator=(const SimulationPropAssemblyVisitor&);

    vtkPropAssembly* m_PropAssembly;
  };
}

#endif
