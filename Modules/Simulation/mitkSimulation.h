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

#ifndef mitkSimulation_h
#define mitkSimulation_h

#include "mitkSimulationDrawTool.h"
#include <mitkBaseData.h>
#include <SimulationExports.h>
#include <sofa/simulation/common/Simulation.h>

namespace mitk
{
  class Simulation_EXPORT Simulation : public BaseData
  {
  public:
    enum SimulationType
    {
      Tree,
      DAG,
      Bgl
    };

    mitkClassMacro(Simulation, BaseData);
    itkNewMacro(Self);

    static void SetActiveSimulation(Self* simulation);

    static const float ScaleFactor;

    double GetDefaultDT() const;
    SimulationDrawTool* GetDrawTool();
    sofa::simulation::Node::SPtr GetRootNode() const;
    sofa::simulation::Simulation::SPtr GetSimulation() const;
    bool RequestedRegionIsOutsideOfTheBufferedRegion();
    void SetAsActiveSimulation();
    void SetDefaultDT(double dt);
    void SetRequestedRegion(itk::DataObject* data);
    void SetRequestedRegionToLargestPossibleRegion();
    void SetRootNode(sofa::simulation::Node* rootNode);
    void UpdateOutputInformation();
    bool VerifyRequestedRegion();

  private:
    Simulation();
    ~Simulation();

    Simulation(Self&);
    Self& operator=(const Self&);

    sofa::simulation::Simulation::SPtr m_Simulation;
    sofa::simulation::Node::SPtr m_RootNode;
    SimulationDrawTool m_DrawTool;
    double m_DefaultDT;
  };
}

#endif
