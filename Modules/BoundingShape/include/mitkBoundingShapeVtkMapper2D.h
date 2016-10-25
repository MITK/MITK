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

#ifndef mitkBoundingShapeVtkMapper2D_h
#define mitkBoundingShapeVtkMapper2D_h

#include <mitkVtkMapper.h>

#include <MitkBoundingShapeExports.h>

#include <vtkActor2D.h>
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkPropAssembly.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>

namespace mitk
{
  class MITKBOUNDINGSHAPE_EXPORT BoundingShapeVtkMapper2D final : public VtkMapper
  {
    class LocalStorage : public Mapper::BaseLocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage();

      bool IsUpdateRequired(mitk::BaseRenderer *renderer, mitk::Mapper *mapper, mitk::DataNode *dataNode);

      vtkSmartPointer<vtkActor> m_Actor;
      vtkSmartPointer<vtkActor2D> m_HandleActor;
      vtkSmartPointer<vtkActor2D> m_SelectedHandleActor;
      vtkSmartPointer<vtkPolyDataMapper> m_Mapper;
      vtkSmartPointer<vtkPolyDataMapper2D> m_HandleMapper;
      vtkSmartPointer<vtkPolyDataMapper2D> m_SelectedHandleMapper;
      vtkSmartPointer<vtkCutter> m_Cutter;
      vtkSmartPointer<vtkPlane> m_CuttingPlane;
      std::vector<vtkSmartPointer<vtkSphereSource>> m_Handles;
      vtkSmartPointer<vtkPropAssembly> m_PropAssembly;
      double m_ZoomFactor;

    private:
      LocalStorage(const LocalStorage &);
      LocalStorage &operator=(const LocalStorage &);
    };

  public:
    static void SetDefaultProperties(DataNode *node, BaseRenderer *renderer = nullptr, bool overwrite = false);

    mitkClassMacro(BoundingShapeVtkMapper2D, VtkMapper);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      void ApplyColorAndOpacityProperties(BaseRenderer *, vtkActor *) override;
    vtkProp *GetVtkProp(BaseRenderer *renderer) override;

  private:
    BoundingShapeVtkMapper2D();
    ~BoundingShapeVtkMapper2D();

    BoundingShapeVtkMapper2D(const Self &);
    Self &operator=(const Self &);

    void GenerateDataForRenderer(BaseRenderer *renderer) override;
    void Update(mitk::BaseRenderer *renderer) override;

    class Impl;
    Impl *m_Impl;
  };
}

#endif
