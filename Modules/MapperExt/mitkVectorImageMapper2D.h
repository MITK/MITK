/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


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
class DisplayGeometry;

class MitkMapperExt_EXPORT VectorImageMapper2D : public GLMapper
{
public:
    mitkClassMacro( VectorImageMapper2D, GLMapper );

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    typedef double vtkScalarType;

    /**
     * @returns the image held by the associated with the mapper or the image
     *          which has been explicitly set by SetImage(...)
     */
    const mitk::Image * GetInput( void );

    virtual void Paint( mitk::BaseRenderer * renderer );

    /**
     * Explicitly set an vector image. This image will be used for
     * rendering instead of the image returned by GetData()
     */
    itkSetConstObjectMacro( Image, mitk::Image );

    /**
     * Get the explicitly set image
     * @returns NULL if no Image has been set instead of GetData();
     */
    itkGetConstObjectMacro( Image, mitk::Image );

    virtual void PaintCells( vtkPolyData* contour, const PlaneGeometry* worldGeometry, const DisplayGeometry* displayGeometry, vtkLinearTransform* vtktransform, BaseRenderer* renderer, vtkScalarsToColors *lut, mitk::Color color, float lwidth, double *spacing );

protected:

    int GetCurrentTimeStep( mitk::BaseData* data, mitk::BaseRenderer* renderer );

    VectorImageMapper2D();

    virtual ~VectorImageMapper2D();

    mitk::Image::ConstPointer m_Image;

    vtkLookupTable *m_LUT;

    vtkPlane* m_Plane;

    vtkCutter* m_Cutter;

};


} // namespace mitk
#endif
