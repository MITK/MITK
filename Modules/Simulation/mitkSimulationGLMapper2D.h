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

#ifndef mitkSimulationGLMapper2D_h
#define mitkSimulationGLMapper2D_h

#include <mitkGLMapper.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class ISimulationService;

  class MitkSimulation_EXPORT SimulationGLMapper2D : public GLMapper
  {
  public:
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false);

    mitkClassMacro(SimulationGLMapper2D, GLMapper);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void ApplyColorAndOpacityProperties(BaseRenderer* renderer, vtkActor* actor = NULL);
    void Paint(BaseRenderer* renderer);

  private:
    SimulationGLMapper2D();
    ~SimulationGLMapper2D();

    SimulationGLMapper2D(const Self&);
    Self& operator=(const Self&);

    ISimulationService* m_SimulationService;
  };
}

#endif
