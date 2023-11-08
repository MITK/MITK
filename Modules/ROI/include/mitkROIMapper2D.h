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
#include <mitkROIMapperLocalStorage.h>
#include <mitkVtkMapper.h>
#include <MitkROIExports.h>

namespace mitk
{
  class MITKROI_EXPORT ROIMapper2D : public VtkMapper
  {
    class LocalStorage : public ROIMapperLocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage() override;

      const PlaneGeometry* GetLastPlaneGeometry() const;
      void SetLastPlaneGeometry(const PlaneGeometry* planeGeometry);

    protected:
      PlaneGeometry::ConstPointer m_LastPlaneGeometry;
    };

  public:
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool override = false);

    mitkClassMacro(ROIMapper2D, VtkMapper)
    itkFactorylessNewMacro(Self)

    vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override;

  protected:
    ROIMapper2D();
    ~ROIMapper2D() override;

    void GenerateDataForRenderer(BaseRenderer* renderer) override;
    void ApplyColorAndOpacityProperties(BaseRenderer* renderer, vtkActor* actor) override;

  private:
    LocalStorageHandler<LocalStorage> m_LocalStorageHandler;
  };
}

#endif
