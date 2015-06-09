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

#ifndef mitkVtkSimulationPolyDataMapper2D_h
#define mitkVtkSimulationPolyDataMapper2D_h

#include <mitkSimulation.h>
#include <vtkPolyDataMapper.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class ISimulationService;

  class MITKSIMULATION_EXPORT vtkSimulationPolyDataMapper2D : public vtkPolyDataMapper
  {
  public:
    static vtkSimulationPolyDataMapper2D* New();
    vtkTypeMacro(vtkSimulationPolyDataMapper2D, vtkPolyDataMapper);

    using vtkPolyDataMapper::GetBounds;
    double* GetBounds() override;
    void Render(vtkRenderer* renderer, vtkActor* actor) override;
    void RenderPiece(vtkRenderer*, vtkActor*) override;
    void SetSimulation(mitk::Simulation::Pointer simulation);

  private:
    vtkSimulationPolyDataMapper2D();
    ~vtkSimulationPolyDataMapper2D();

    Simulation::Pointer m_Simulation;
    ISimulationService* m_SimulationService;
  };
}

#endif
