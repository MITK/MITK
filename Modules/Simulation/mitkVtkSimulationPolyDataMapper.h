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

#ifndef mitkVtkSimulationPolyDataMapper_h
#define mitkVtkSimulationPolyDataMapper_h

#include <mitkSimulation.h>
#include <vtkPolyDataMapper.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class ISimulationService;

  class MitkSimulation_EXPORT vtkSimulationPolyDataMapper : public vtkPolyDataMapper
  {
  public:
    static vtkSimulationPolyDataMapper* New();
    vtkTypeMacro(vtkSimulationPolyDataMapper, vtkPolyDataMapper);

    double* GetBounds();
    void Render(vtkRenderer* renderer, vtkActor* actor);
    void RenderPiece(vtkRenderer*, vtkActor*);
    void SetSimulation(mitk::Simulation::Pointer simulation);

  private:
    vtkSimulationPolyDataMapper();
    ~vtkSimulationPolyDataMapper();

    Simulation::Pointer m_Simulation;
    ISimulationService* m_SimulationService;
  };
}

#endif
