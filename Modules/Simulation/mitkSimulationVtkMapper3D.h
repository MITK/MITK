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

#ifndef mitkSimulationVtkMapper3D_h
#define mitkSimulationVtkMapper3D_h

#include <mitkVtkMapper.h>
#include <vtkSmartPointer.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class vtkSimulationPolyDataMapper;

  class MitkSimulation_EXPORT SimulationVtkMapper3D : public VtkMapper
  {
    class LocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage();

      vtkSmartPointer<vtkSimulationPolyDataMapper> m_Mapper;
      vtkSmartPointer<vtkActor> m_Actor;

    private:
      LocalStorage(const LocalStorage&);
      LocalStorage& operator=(const LocalStorage&);
    };

  public:
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false);

    mitkClassMacro(SimulationVtkMapper3D, VtkMapper);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void ApplyColorAndOpacityProperties(vtkActor*, BaseRenderer*);
    void ApplySimulationProperties(BaseRenderer* renderer);
    vtkProp* GetVtkProp(BaseRenderer* renderer);

  protected:
    void GenerateDataForRenderer(BaseRenderer* renderer);

  private:
    SimulationVtkMapper3D();
    ~SimulationVtkMapper3D();

    SimulationVtkMapper3D(const Self&);
    Self& operator=(const Self&);

    LocalStorageHandler<LocalStorage> m_LocalStorageHandler;
  };
}

#endif
