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

#ifndef mitkSimulationMapper3D_h
#define mitkSimulationMapper3D_h

#include <mitkVtkMapper3D.h>
#include <SimulationExports.h>
#include <vtkSmartPointer.h>
#include <vtkPropAssembly.h>

namespace mitk
{
  class Simulation_EXPORT SimulationMapper3D : public VtkMapper3D
  {
    class LocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage();

      vtkSmartPointer<vtkPropAssembly> m_VtkProp;

    private:
      LocalStorage(const LocalStorage&);
      LocalStorage& operator=(const LocalStorage&);
    };

  public:
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false);

    mitkClassMacro(SimulationMapper3D, VtkMapper3D);
    itkNewMacro(Self);

    void ApplyProperties(vtkActor* actor, BaseRenderer* renderer);
    vtkProp* GetVtkProp(BaseRenderer* renderer);

  protected:
    void GenerateDataForRenderer(BaseRenderer* renderer);

  private:
    SimulationMapper3D();
    ~SimulationMapper3D();

    SimulationMapper3D(const Self&);
    Self& operator=(const Self&);

    mitk::Mapper::LocalStorageHandler<LocalStorage> m_LocalStorageHandler;
  };
}

#endif
