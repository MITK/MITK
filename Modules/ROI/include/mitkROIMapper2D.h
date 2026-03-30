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
  /**
   * \brief VTK-based mapper for rendering ROI elements in 2D slice views.
   *
   * This mapper renders each ROI element as the intersection of its axis-aligned
   * bounding box (cube) with the current slice plane. The intersection is computed
   * using a plane cutter on the transformed cube geometry. Optional captions are
   * displayed at the bottom-left corner of each slice intersection.
   *
   * Supported data node properties for caption rendering:
   *   - \c caption (StringProperty): Caption template with placeholder support
   *   - \c font.size (IntProperty): Font size in points
   *   - \c font.bold (BoolProperty): Bold font style
   *   - \c font.italic (BoolProperty): Italic font style
   *
   * \sa ROI, ROIMapper3D, ROIMapperLocalStorage
   */
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
    /**
     * \brief Initialize default properties on the given data node for 2D ROI rendering.
     *
     * \param[in] node The data node to initialize properties on.
     * \param[in] renderer The renderer context, or \c nullptr for the default context.
     * \param[in] override If \c true, overwrite existing properties.
     */
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool override = false);

    mitkClassMacro(ROIMapper2D, VtkMapper)
    itkFactorylessNewMacro(Self)

    /**
     * \brief Get the VTK prop assembly for the given renderer.
     *
     * \param[in] renderer The renderer to get the prop for.
     * \return The VTK prop assembly containing all rendered ROI elements.
     */
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
