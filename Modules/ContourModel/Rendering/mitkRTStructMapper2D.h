/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRTStructMapper2D_h
#define mitkRTStructMapper2D_h

#include <mitkVtkMapper.h>
#include <MitkContourModelExports.h>

namespace mitk
{
  class MITKCONTOURMODEL_EXPORT RTStructMapper2D : public VtkMapper
  {
    class LocalStorage : public VtkMapper::LocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage() override;

      vtkPropAssembly* GetPropAssembly() const;
      void SetPropAssembly(vtkPropAssembly* propAssembly);

    private:
      vtkSmartPointer<vtkPropAssembly> m_PropAssembly;
    };

  public:
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool override = false);

    mitkClassMacro(RTStructMapper2D, VtkMapper)
    itkFactorylessNewMacro(Self)

    vtkProp* GetVtkProp(mitk::BaseRenderer* renderer) override;

  protected:
    RTStructMapper2D();
    ~RTStructMapper2D() override;

    void GenerateDataForRenderer(BaseRenderer* renderer) override;

  private:
    LocalStorageHandler<LocalStorage> m_LocalStorageHandler;
  };
}

#endif
