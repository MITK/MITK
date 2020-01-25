/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_VECTOR_IMAGE_MAPPER_2D__H_
#define _MITK_VECTOR_IMAGE_MAPPER_2D__H_

#include "MitkMapperExtExports.h"

#include "mitkCommon.h"
#include "mitkGLMapper.h"
#include "mitkImage.h"

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

  class MITKMAPPEREXT_EXPORT VectorImageMapper2D : public GLMapper
  {
  public:
    mitkClassMacro(VectorImageMapper2D, GLMapper);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self) typedef double vtkScalarType;

    /**
     * @returns the image held by the associated with the mapper or the image
     *          which has been explicitly set by SetImage(...)
     */
    const mitk::Image *GetInput(void);

    void Paint(mitk::BaseRenderer *renderer) override;

    /**
     * Explicitly set an vector image. This image will be used for
     * rendering instead of the image returned by GetData()
     */
    itkSetConstObjectMacro(Image, mitk::Image);

    /**
     * Get the explicitly set image
     * @returns nullptr if no Image has been set instead of GetData();
     */
    itkGetConstObjectMacro(Image, mitk::Image);

    virtual void PaintCells(vtkPolyData *contour,
                            const PlaneGeometry * /*worldGeometry*/,
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
