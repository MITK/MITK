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

#include <mitkBaseData.h>
#include <sofa/core/visual/DrawToolGL.h>
#include <sofa/simulation/common/Simulation.h>
#include <SimulationExports.h>

namespace mitk
{
  class Simulation_EXPORT Simulation : public BaseData
  {
  public:
    mitkClassMacro(Simulation, BaseData);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void Animate();
    sofa::core::visual::DrawTool* GetDrawTool();
    sofa::simulation::Node::SPtr GetRootNode() const;
    sofa::simulation::Simulation::SPtr GetSimulation() const;
    void Reset();
    void SetAnimationFlag(bool animate);
    void SetDt(double dt);
    void SetRootNode(sofa::simulation::Node::SPtr rootNode);

    bool RequestedRegionIsOutsideOfTheBufferedRegion();
    void SetRequestedRegion(const itk::DataObject*);
    void SetRequestedRegionToLargestPossibleRegion();
    void UpdateOutputInformation();
    bool VerifyRequestedRegion();

  private:
    Simulation();
    ~Simulation();

    sofa::simulation::Simulation::SPtr m_Simulation;
    sofa::simulation::Node::SPtr m_RootNode;
    sofa::core::visual::DrawToolGL m_DrawTool;
  };
}

#endif
