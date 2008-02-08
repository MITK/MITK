/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#ifndef MITKGEOMETRY2DDATATOSURFACEDATAFILTER_H_HEADER_INCLUDED_C10B22CD
#define MITKGEOMETRY2DDATATOSURFACEDATAFILTER_H_HEADER_INCLUDED_C10B22CD

#include "mitkSurfaceSource.h"
#include "mitkGeometry3D.h"
#include "vtkSystemIncludes.h"

class vtkPlaneSource;
class vtkLinearTransform;
class vtkTransformPolyDataFilter;
class vtkCubeSource;
class vtkTransform;
class vtkTransformPolyDataFilter;
class vtkPlane;
class vtkCutter;
class vtkStripper;
class vtkPolyData;
class vtkTriangleFilter;
class vtkTextureMapToPlane;
class vtkBox;
class vtkClipPolyData;

namespace mitk {

class Geometry2DData;

//##ModelId=3EF4A3D701D4
//##Documentation
//## @brief Superclass of all classes having a Geometry2DData as input and
//## generating Images as output
//##
//## Currently implemented for PlaneGeometry and AbstractTransformGeometry.
//## Currently, this class does not really have subclasses, but does the job
//## for itself. It checks which kind of Geometry2D is stored in the
//## Geometry2DData and - if it knows how - it generates the respective
//## Surface. Of course, this has the disadvantage that for any new type of
//## Geometry2D this class (Geometry2DDataToSurfaceFilter) has to be
//## changed/extended. The idea is to move the type specific generation code in
//## subclasses, and internally (within this class) use a factory to create an
//## instance of the required subclass and delegate the surface generation to
//## it.
//## @todo make extension easier
//## @ingroup Process
class MITK_CORE_EXPORT Geometry2DDataToSurfaceFilter : public SurfaceSource
{
  public:
    mitkClassMacro(Geometry2DDataToSurfaceFilter, SurfaceSource);
    itkNewMacro(Self);

    //##ModelId=3EF56DCD021E
    virtual void GenerateOutputInformation();

    //##ModelId=3EF56DD10039
    virtual void GenerateData();

    //##ModelId=3EF5795002B2
    const mitk::Geometry2DData *GetInput(void);
    //##ModelId=3EF5795202A1
    const mitk::Geometry2DData *GetInput(unsigned int idx);

    //##ModelId=3EF59AF703BE
    virtual void SetInput(const mitk::Geometry2DData *image);
    //##ModelId=3EF59AFA01D8
    virtual void SetInput(unsigned int index, const mitk::Geometry2DData *image);

    //##Documentation
    //## @brief If @a true (default), use Geometry3D::GetParametricBounds() to define the resolution in parameter space, 
    //## otherwise use m_XResolution and m_YResolution
    itkGetMacro(UseGeometryParametricBounds, bool);
    //##Documentation
    //## @brief If @a true (default), use Geometry3D::GetParametricBounds() to define the resolution in parameter space, 
    //## otherwise use m_XResolution and m_YResolution
    itkSetMacro(UseGeometryParametricBounds, bool);

    //##Documentation
    //## @brief Get x-resolution in parameter space
    //##
    //## The m_PlaneSource will create this many sub-rectangles
    //## in x-direction (see vtkPlaneSource::SetXResolution)
    //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
    //## the x-bounds of Geometry3D::GetParametricBounds() are used.
    //## @sa m_XResolution
    itkGetMacro(XResolution, int);
    //##Documentation
    //## @brief Set x-resolution in parameter space
    //##
    //## The m_PlaneSource will create this many sub-rectangles
    //## in x-direction (see vtkPlaneSource::SetXResolution)
    //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
    //## the x-bounds of Geometry3D::GetParametricBounds() are used.
    //## @sa m_XResolution
    itkSetMacro(XResolution, int);

    //##Documentation
    //## @brief Get y-resolution in parameter space
    //##
    //## The m_PlaneSource will create this many sub-rectangles
    //## in y-direction (see vtkPlaneSource::SetYResolution)
    //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
    //## the y-bounds of Geometry3D::GetParametricBounds() are used.
    //## @sa m_YResolution
    itkGetMacro(YResolution, int);
    //##Documentation
    //## @brief Set y-resolution in parameter space
    //##
    //## The m_PlaneSource will create this many sub-rectangles
    //## in y-direction (see vtkPlaneSource::SetYResolution)
    //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
    //## the y-bounds of Geometry3D::GetParametricBounds() are used.
    //## @sa m_YResolution
    itkSetMacro(YResolution, int);

    //##Documentation
    //## @brief Get whether the Surface is at the origin and placed using the Geometry
    //##
    //## Default is @a false, i.e., the transform of the Geometry is the identity, thus
    //## the points within the Surface are at their final position. Otherwise 
    //## (m_PlaceByGeometry==@a true), the first cornerpoint of the created Surface is
    //## at the origin and the actual position is determined by the transform of the 
    //## Geometry.
    //## \sa m_PlaceByGeometry
    itkGetConstMacro(PlaceByGeometry, bool);
    //##Documentation
    //## @brief Set whether the Surface is at the origin and placed using the Geometry
    //##
    //## Default is @a false, i.e., the transform of the Geometry is the identity, thus
    //## the points within the Surface are at their final position. Otherwise 
    //## (m_PlaceByGeometry==@a true), the first cornerpoint of the created Surface is
    //## at the origin and the actual position is determined by the transform of the 
    //## Geometry.
    //## \sa m_PlaceByGeometry
    itkSetMacro(PlaceByGeometry, bool);
    itkBooleanMacro(PlaceByGeometry);


    itkGetConstMacro( UseBoundingBox, bool );
    itkSetMacro( UseBoundingBox, bool );
    itkBooleanMacro( UseBoundingBox );

    void SetBoundingBox( const mitk::BoundingBox *boundingBox );
    const mitk::BoundingBox *GetBoundingBox() const;

    
  protected:
    //##ModelId=3EF4A4A70345
    Geometry2DDataToSurfaceFilter();

    //##ModelId=3EF4A4A70363
    virtual ~Geometry2DDataToSurfaceFilter();

    //##ModelId=3EF5734B013A
    //##Documentation
    //## @brief Source to create the vtk-representation of the parameter space rectangle of the Geometry2D
    vtkPlaneSource* m_PlaneSource;

    //##ModelId=3EF5734B01DA
    //##Documentation
    //## @brief Filter to create the vtk-representation of the Geometry2D, which is a
    //## transformation of the m_PlaneSource
    vtkTransformPolyDataFilter* m_VtkTransformPlaneFilter;

    //##Documentation
    //## @brief If @a true, use Geometry3D::GetParametricBounds() to define the resolution in parameter space, 
    //## otherwise use m_XResolution and m_YResolution
    bool m_UseGeometryParametricBounds;

    //##ModelId=3EF5734B020C
    //##Documentation
    //## @brief X-resolution in parameter space
    //##
    //## The m_PlaneSource will create this many sub-rectangles
    //## in x-direction (see vtkPlaneSource::SetXResolution)
    //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
    //## the x-bounds of Geometry3D::GetParametricBounds() are used.
    //## @sa m_XResolution
    int m_XResolution;

    //##ModelId=3EF5734B023E
    //##Documentation
    //## @brief Y-resolution in parameter space
    //##
    //## The m_PlaneSource will create this many sub-rectangles
    //## in y-direction (see vtkPlaneSource::SetYResolution)
    //## @note Only used, when GetUseGeometryParametricBounds() is @a false, otherwise the
    //## the y-bounds of Geometry3D::GetParametricBounds() are used.
    int m_YResolution;

    //##Documentation
    //## @brief Define whether the Surface is at the origin and placed using the Geometry
    //##
    //## Default is @a false, i.e., the transform of the Geometry is the identity, thus
    //## the points within the Surface are at their final position. Otherwise 
    //## (m_PlaceByGeometry==@a true), the first cornerpoint of the created Surface is
    //## at the origin and the actual position is determined by the transform of the 
    //## Geometry.
    bool m_PlaceByGeometry;

    bool m_UseBoundingBox;

    mitk::BoundingBox::ConstPointer m_BoundingBox;

    vtkCubeSource *m_CubeSource;
    vtkTransform *m_Transform;
    vtkTransformPolyDataFilter *m_PolyDataTransformer;
    
    vtkPlane *m_Plane;
    vtkCutter *m_PlaneCutter;
    vtkStripper *m_PlaneStripper;
    vtkPolyData *m_PlanePolyData;
    vtkTriangleFilter *m_PlaneTriangler;
    vtkTextureMapToPlane *m_TextureMapToPlane;

    vtkBox *m_Box;
    vtkClipPolyData *m_PlaneClipper;
};

} // namespace mitk

#endif /* MITKGEOMETRY2DDATATOSURFACEDATAFILTER_H_HEADER_INCLUDED_C10B22CD */
