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
#include <mitkSurface.h>
#include <SimulationExports.h>
#include <sofa/simulation/common/Simulation.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

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

    void AppendSnapshot(Surface::Pointer surface) const;
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
    Surface::Pointer TakeSnapshot() const;
    void UpdateOutputInformation();
    bool VerifyRequestedRegion();

  private:
    Simulation();
    ~Simulation();

    Simulation(Self&);
    Self& operator=(const Self&);

    vtkSmartPointer<vtkPolyData> CreateSnapshot() const;

    sofa::simulation::Simulation::SPtr m_Simulation;
    sofa::simulation::Node::SPtr m_RootNode;
    SimulationDrawTool m_DrawTool;
    double m_DefaultDT;
  };
}

#endif
