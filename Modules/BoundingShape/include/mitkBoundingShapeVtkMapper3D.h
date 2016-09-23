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


#ifndef mitkBoundingShapeVtkMapper3D_h
#define mitkBoundingShapeVtkMapper3D_h

#include <MitkBoundingShapeExports.h>

#include <mitkVtkMapper.h>

#include <vtkPropAssembly.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>

namespace mitk
{

  class MITKBOUNDINGSHAPE_EXPORT BoundingShapeVtkMapper3D : public VtkMapper
  {
    class LocalStorage : public Mapper::BaseLocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage();
      std::vector<vtkSmartPointer<vtkSphereSource>> m_Handles;
      vtkSmartPointer<vtkActor> m_Actor;
      vtkSmartPointer<vtkActor> m_HandleActor;
      vtkSmartPointer<vtkActor> m_SelectedHandleActor;
      vtkSmartPointer<vtkPropAssembly> m_PropAssembly;
    private:
      LocalStorage(const LocalStorage&);
      LocalStorage& operator=(const LocalStorage&);
    };

  public:
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false);

    mitkClassMacro(BoundingShapeVtkMapper3D, VtkMapper)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void ApplyColorAndOpacityProperties(BaseRenderer*, vtkActor*) override;
    void ApplyBoundingShapeProperties(BaseRenderer* renderer, vtkActor*);
    vtkProp* GetVtkProp(BaseRenderer* renderer) override;
 //   virtual void UpdateVtkTransform(mitk::BaseRenderer* renderer) override;
  protected:
    void GenerateDataForRenderer(BaseRenderer* renderer) override;

  private:
    BoundingShapeVtkMapper3D();
    ~BoundingShapeVtkMapper3D();

    BoundingShapeVtkMapper3D(const Self&);
    Self& operator=(const Self&);

    class Impl;
    Impl* m_Impl;
  };
}

#endif
