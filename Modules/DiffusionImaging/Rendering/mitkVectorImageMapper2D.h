/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_VECTOR_IMAGE_MAPPER_2D__H_
#define _MITK_VECTOR_IMAGE_MAPPER_2D__H_

#include "MitkDiffusionImagingExports.h"

#include "mitkCommon.h"
#include "mitkGLMapper2D.h"
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
class Geometry2D;
class DisplayGeometry;

class MitkDiffusionImaging_EXPORT VectorImageMapper2D : public GLMapper2D
{
public:
    mitkClassMacro( VectorImageMapper2D, GLMapper2D );

    itkNewMacro( Self );
    typedef vtkFloatingPointType vtkScalarType;

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

    virtual void PaintCells( vtkPolyData* contour, const Geometry2D* worldGeometry, const DisplayGeometry* displayGeometry, vtkLinearTransform* vtktransform, BaseRenderer* renderer, vtkScalarsToColors *lut, mitk::Color color, float lwidth, vtkFloatingPointType *spacing );

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
