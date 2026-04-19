/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelMapper2D_h
#define mitkContourModelMapper2D_h

#include <mitkCommon.h>
#include <mitkContourModel.h>
#include <mitkContourModelMapper2DBase.h>
#include <MitkContourModelExports.h>

namespace mitk
{
  class BaseRenderer;
  class ContourModel;

  /**
   * \brief OpenGL-based mapper to display a mitk::ContourModel in a 2D render window.
   *
   * Renders a ContourModel by projecting its vertices onto the current 2D slice
   * and drawing line segments between them using OpenGL primitives. Supports
   * optional subdivision curve smoothing via the "subdivision curve" property.
   *
   * Properties evaluated by this mapper:
   * - "contour.color": line color (default: yellow-green)
   * - "contour.points.color": point marker color
   * - "contour.width": line width in pixels
   * - "contour.points.show": show point markers
   * - "contour.controlpoints.show": show control point markers
   * - "subdivision curve": enable DLG subdivision smoothing
   *
   * \sa ContourModel, ContourModelMapper2DBase, ContourModelMapper3D
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ContourModelMapper2D : public ContourModelMapper2DBase
  {
  public:
    mitkClassMacro(ContourModelMapper2D, ContourModelMapper2DBase);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Perform the rendering of the contour model.
       *
       * Projects the contour onto the current 2D slice and draws it.
       * If the "subdivision curve" property is enabled, the contour is
       * first smoothed using a ContourModelSubDivisionFilter.
       *
       * \param[in] renderer The renderer for the current render window.
       * \param[in] type The render type (opaque, transparent, overlay).
       */
      void MitkRender(BaseRenderer *renderer, mitk::VtkPropRenderer::RenderType type) override;

    /** \brief Set default rendering properties for ContourModel nodes.
     * \param[in] node The data node to configure.
     * \param[in] renderer The renderer context. If nullptr, properties are set globally.
     * \param[in] overwrite If true, existing properties are overwritten.
     */
    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool overwrite = false);

    /** \brief Local storage handler for per-renderer data. */
    LocalStorageHandler<BaseLocalStorage> m_LSH;

  protected:
    ContourModelMapper2D();

    ~ContourModelMapper2D() override;

    mitk::ContourModel::Pointer m_SubdivisionContour;
    bool m_InitSubdivisionCurve;

  private:
    /**
    * return a reference of the rendered data object
    */
    ContourModel *GetInput(void);
  };

} // namespace mitk

#endif
