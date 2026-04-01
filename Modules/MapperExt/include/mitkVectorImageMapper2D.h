/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVectorImageMapper2D_h
#define mitkVectorImageMapper2D_h

#include <MitkMapperExtExports.h>

#include <mitkCommon.h>
#include <mitkGLMapper.h>
#include <mitkImage.h>

class vtkLookupTable;
class vtkScalarsToColors;
class vtkImageReslice;
class vtkPolyData;
class vtkGlyph2D;
class vtkPlane;
class vtkCutter;

namespace mitk
{
  class BaseRenderer;
  class PlaneGeometry;

  /** \brief OpenGL-based mapper for rendering 2D vector field visualizations.
   *
   * Renders vector image data as glyph arrows on 2D slice views. Each voxel's
   * vector direction is visualized as an arrow glyph. An explicit image can be
   * set via SetImage() to override the data node's associated image.
   *
   * \sa GLMapper
   * \ingroup Mapper
   */
  class MITKMAPPEREXT_EXPORT VectorImageMapper2D : public GLMapper
  {
  public:
    mitkClassMacro(VectorImageMapper2D, GLMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self) typedef double vtkScalarType;

    /** \brief Get the input image to render.
     *
     * Returns the explicitly set image if available, otherwise the image
     * from the associated data node.
     *
     * \return The input image, or \c nullptr if none is available.
     */
    const mitk::Image *GetInput(void);

    /** \brief Render the vector field as glyphs on the current 2D slice.
     *
     * \param[in] renderer The renderer to paint into.
     */
    void Paint(mitk::BaseRenderer *renderer) override;

    /** \brief Explicitly set a vector image for rendering.
     *
     * When set, this image is used instead of the image from GetData().
     *
     * \param[in] _arg The vector image to render.
     */
    itkSetConstObjectMacro(Image, mitk::Image);

    /** \brief Get the explicitly set image.
     *
     * \return The explicitly set image, or \c nullptr if none was set.
     */
    itkGetConstObjectMacro(Image, mitk::Image);

    /** \brief Render the cells of a contour poly data as vector glyphs.
     *
     * \param[in] contour       The poly data contour to render.
     * \param[in] worldGeometry The current world geometry (unused).
     * \param[in] vtktransform  The VTK linear transform for coordinate conversion.
     * \param[in] renderer      The renderer to paint into.
     * \param[in] lut           The lookup table for scalar-to-color mapping.
     * \param[in] color         The default glyph color.
     * \param[in] lwidth        The line width for rendering.
     * \param[in] spacing       The voxel spacing array.
     */
    virtual void PaintCells(vtkPolyData *contour,
                            const PlaneGeometry *worldGeometry,
                            vtkLinearTransform *vtktransform,
                            BaseRenderer *renderer,
                            vtkScalarsToColors *lut,
                            mitk::Color color,
                            float lwidth,
                            double *spacing);

  protected:
    int GetCurrentTimeStep(mitk::BaseData *data, mitk::BaseRenderer *renderer);

    VectorImageMapper2D();

    ~VectorImageMapper2D() override;

    mitk::Image::ConstPointer m_Image;

    vtkLookupTable *m_LUT;

    vtkPlane *m_Plane;

    vtkCutter *m_Cutter;
  };

} // namespace mitk
#endif
