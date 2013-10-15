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

#ifndef mitkPlanarFigureVtkMapper3D_h
#define mitkPlanarFigureVtkMapper3D_h

#include <mitkVtkMapper.h>
#include <vtkSmartPointer.h>
#include <PlanarFigureExports.h>

namespace mitk
{
  class PlanarFigure_EXPORT PlanarFigureVtkMapper3D : public VtkMapper
  {
    class LocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage();

      vtkSmartPointer<vtkActor> m_Actor;
      unsigned long m_LastMTime;

    private:
      LocalStorage(const LocalStorage&);
      LocalStorage& operator=(const LocalStorage&);
    };

  public:
    static void SetDefaultProperties(DataNode*, BaseRenderer* = NULL, bool = false);

    mitkClassMacro(PlanarFigureVtkMapper3D, VtkMapper);
    itkNewMacro(Self);

    void ApplyColorAndOpacityProperties(BaseRenderer* renderer, vtkActor* actor);
    void ApplyPlanarFigureProperties(BaseRenderer* renderer, vtkActor* actor);
    vtkProp* GetVtkProp(BaseRenderer* renderer);
    void UpdateVtkTransform(BaseRenderer*);

  private:
    PlanarFigureVtkMapper3D();
    ~PlanarFigureVtkMapper3D();

    PlanarFigureVtkMapper3D(const Self&);
    Self& operator=(const Self&);

    void GenerateDataForRenderer(BaseRenderer* renderer);

    LocalStorageHandler<LocalStorage> m_LocalStorageHandler;
  };
}

#endif
