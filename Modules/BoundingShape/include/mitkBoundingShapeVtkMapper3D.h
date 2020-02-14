/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  public:
    static void SetDefaultProperties(DataNode *node, BaseRenderer *renderer = nullptr, bool overwrite = false);

    mitkClassMacro(BoundingShapeVtkMapper3D, VtkMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      void ApplyColorAndOpacityProperties(BaseRenderer *, vtkActor *) override;
    void ApplyBoundingShapeProperties(BaseRenderer *renderer, vtkActor *);
    vtkProp *GetVtkProp(BaseRenderer *renderer) override;
    //   virtual void UpdateVtkTransform(mitk::BaseRenderer* renderer) override;
  protected:
    void GenerateDataForRenderer(BaseRenderer *renderer) override;

  private:
    BoundingShapeVtkMapper3D();
    ~BoundingShapeVtkMapper3D() override;

    BoundingShapeVtkMapper3D(const Self &);
    Self &operator=(const Self &);

    class Impl;
    Impl *m_Impl;
  };
}

#endif
