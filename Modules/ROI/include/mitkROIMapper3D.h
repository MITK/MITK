/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROIMapper3D_h
#define mitkROIMapper3D_h

#include <mitkLocalStorageHandler.h>
#include <mitkVtkMapper.h>
#include <MitkROIExports.h>

#include <vtkPropAssembly.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  class MITKROI_EXPORT ROIMapper3D : public VtkMapper
  {
    class LocalStorage : public Mapper::BaseLocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage() override;

      vtkPropAssembly* GetPropAssembly() const;
      void SetPropAssembly(vtkPropAssembly* propAssembly);

    protected:
      vtkSmartPointer<vtkPropAssembly> m_PropAssembly;
    };

  public:
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool override = false);

    mitkClassMacro(ROIMapper3D, VtkMapper)
    itkFactorylessNewMacro(Self)

    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

  protected:
    ROIMapper3D();
    ~ROIMapper3D() override;

    void GenerateDataForRenderer(BaseRenderer* renderer) override;
    void ApplyColorAndOpacityProperties(BaseRenderer* renderer, vtkActor* actor) override;

  private:
    LocalStorageHandler<LocalStorage> m_LocalStorageHandler;
  };
}

#endif
