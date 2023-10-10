/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROIMapper2D_h
#define mitkROIMapper2D_h

#include <mitkLocalStorageHandler.h>
#include <mitkVtkMapper.h>
#include <MitkROIExports.h>

#include <vtkPropAssembly.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  class MITKROI_EXPORT ROIMapper2D : public VtkMapper
  {
    class LocalStorage : public Mapper::BaseLocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage() override;

      vtkPropAssembly* GetPropAssembly() const;

    protected:
      vtkSmartPointer<vtkPropAssembly> m_PropAssembly;
    };

  public:
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool override = false);

    mitkClassMacro(ROIMapper2D, VtkMapper)
    itkFactorylessNewMacro(Self)

    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

  protected:
    ROIMapper2D();
    ~ROIMapper2D() override;

  private:
    LocalStorageHandler<LocalStorage> m_LocalStorageHandler;
  };
}

#endif
