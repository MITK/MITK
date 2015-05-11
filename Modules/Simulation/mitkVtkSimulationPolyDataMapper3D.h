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

#ifndef mitkVtkSimulationPolyDataMapper3D_h
#define mitkVtkSimulationPolyDataMapper3D_h

#include <mitkSimulation.h>
#include <vtkPolyDataMapper.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class ISimulationService;

  class MITKSIMULATION_EXPORT vtkSimulationPolyDataMapper3D : public vtkPolyDataMapper
  {
  public:
    static vtkSimulationPolyDataMapper3D* New();
    vtkTypeMacro(vtkSimulationPolyDataMapper3D, vtkPolyDataMapper);

    using vtkPolyDataMapper::GetBounds;
    double* GetBounds() override;

    void Render(vtkRenderer* renderer, vtkActor* actor) override;
    void RenderPiece(vtkRenderer*, vtkActor*) override;
    void SetSimulation(mitk::Simulation::Pointer simulation);

  private:
    vtkSimulationPolyDataMapper3D();
    ~vtkSimulationPolyDataMapper3D();

    Simulation::Pointer m_Simulation;
    ISimulationService* m_SimulationService;
  };
}

#endif
