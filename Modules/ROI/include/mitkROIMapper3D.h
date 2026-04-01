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
#include <mitkROIMapperLocalStorage.h>
#include <mitkVtkMapper.h>
#include <MitkROIExports.h>

namespace mitk
{
  /**
   * \brief VTK-based mapper for rendering ROI elements in 3D views.
   *
   * This mapper renders each ROI element as a full 3D cube (axis-aligned bounding box)
   * transformed according to the ROI data's geometry. Individual ROI elements support
   * per-element color, opacity, and line width properties.
   *
   * \sa ROI, ROIMapper2D, ROIMapperLocalStorage
   */
  class MITKROI_EXPORT ROIMapper3D : public VtkMapper
  {
    class LocalStorage : public ROIMapperLocalStorage
    {
    public:
      LocalStorage();
      ~LocalStorage() override;
    };

  public:
    /**
     * \brief Initialize default properties on the given data node for 3D ROI rendering.
     *
     * \param[in] node The data node to initialize properties on.
     * \param[in] renderer The renderer context, or \c nullptr for the default context.
     * \param[in] override If \c true, overwrite existing properties.
     */
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool override = false);

    mitkClassMacro(ROIMapper3D, VtkMapper)
    itkFactorylessNewMacro(Self)

    /**
     * \brief Get the VTK prop assembly for the given renderer.
     *
     * \param[in] renderer The renderer to get the prop for.
     * \return The VTK prop assembly containing all rendered ROI elements.
     */
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
