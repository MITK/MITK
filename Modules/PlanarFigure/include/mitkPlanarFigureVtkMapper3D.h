/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanarFigureVtkMapper3D_h
#define mitkPlanarFigureVtkMapper3D_h

#include <MitkPlanarFigureExports.h>
#include <mitkVtkMapper.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  class MITKPLANARFIGURE_EXPORT PlanarFigureVtkMapper3D : public VtkMapper
  {
    class LocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage();

      vtkSmartPointer<vtkActor> m_Actor;
      unsigned long m_LastMTime;

    private:
      LocalStorage(const LocalStorage &);
      LocalStorage &operator=(const LocalStorage &);
    };

  public:
    static void SetDefaultProperties(DataNode *, BaseRenderer * = nullptr, bool = false);

    mitkClassMacro(PlanarFigureVtkMapper3D, VtkMapper);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      void ApplyColorAndOpacityProperties(BaseRenderer *renderer, vtkActor *actor) override;
    void ApplyPlanarFigureProperties(BaseRenderer *renderer, vtkActor *actor);
    vtkProp *GetVtkProp(BaseRenderer *renderer) override;
    void UpdateVtkTransform(BaseRenderer *) override;

  private:
    PlanarFigureVtkMapper3D();
    ~PlanarFigureVtkMapper3D() override;

    PlanarFigureVtkMapper3D(const Self &);
    Self &operator=(const Self &);

    void GenerateDataForRenderer(BaseRenderer *renderer) override;

    LocalStorageHandler<LocalStorage> m_LocalStorageHandler;
    bool m_FillPf;
  };
}

#endif
