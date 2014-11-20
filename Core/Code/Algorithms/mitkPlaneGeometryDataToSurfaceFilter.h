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

#ifndef MITKGEOMETRY2DDATATOSURFACEDATAFILTER_H_HEADER_INCLUDED_C10B22CD
#define MITKGEOMETRY2DDATATOSURFACEDATAFILTER_H_HEADER_INCLUDED_C10B22CD

#include "mitkSurfaceSource.h"
#include "mitkGeometry3D.h"
#include "vtkSystemIncludes.h"

class vtkPlaneSource;
class vtkTransformPolyDataFilter;
class vtkCubeSource;
class vtkTransform;
class vtkPlane;
class vtkCutter;
class vtkStripper;
class vtkPolyData;
class vtkPPolyDataNormals;
class vtkTriangleFilter;
class vtkTextureMapToPlane;
class vtkBox;
class vtkClipPolyData;

namespace mitk {
  class PlaneGeometryData;
  class PlaneGeometryDataToSurfaceFilter;
  /** \deprecatedSince{2014_10} This class is deprecated. Please use PlaneGeometryDataToSurfaceFilter instead. */
  DEPRECATED( typedef PlaneGeometryDataToSurfaceFilter Geometry2DDataToSurfaceFilter);
  /** \brief Superclass of all classes having a PlaneGeometryData as input and
  *  generating Images as output
  *
  *  Currently implemented for PlaneGeometry and AbstractTransformGeometry.
  *  Currently, this class does not really have subclasses, but does the job
  *  for itself. It checks which kind of PlaneGeometry is stored in the
  *  PlaneGeometryData and - if it knows how - it generates the respective
  *  Surface. Of course, this has the disadvantage that for any new type of
  *  PlaneGeometry this class (PlaneGeometryDataToSurfaceFilter) has to be
  *  changed/extended. The idea is to move the type specific generation code in
  *  subclasses, and internally (within this class) use a factory to create an
  *  instance of the required subclass and delegate the surface generation to
  *  it.
  *
  *  \sa mitk::DeformablePlane
  *  \todo make extension easier
  *  \ingroup Process
  */
  class MITK_CORE_EXPORT PlaneGeometryDataToSurfaceFilter : public SurfaceSource
  {
  public:
    mitkClassMacro(PlaneGeometryDataToSurfaceFilter, SurfaceSource);
    itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      virtual void GenerateOutputInformation();

    virtual void GenerateData();

    const PlaneGeometryData *GetInput(void);
    const PlaneGeometryData *GetInput(unsigned int idx);

    virtual void SetInput(const PlaneGeometryData *image);
    using itk::ProcessObject::SetInput;
    virtual void SetInput(unsigned int index, const PlaneGeometryData *image);

    /** \brief If \a true (default), use Geometry3D::GetParametricBounds() to define the resolution in parameter space,
    *  otherwise use m_XResolution and m_YResolution
    */
    itkGetMacro(UseGeometryParametricBounds, bool);
    /** \brief If \a true (default), use Geometry3D::GetParametricBounds() to define the resolution in parameter space,
    *  otherwise use m_XResolution and m_YResolution
    */
    itkSetMacro(UseGeometryParametricBounds, bool);

    /** \brief Get x-resolution in parameter space
    *
    *  The m_PlaneSource will create this many sub-rectangles
    *  in x-direction (see vtkPlaneSource::SetXResolution)
    *  \note Only used, when GetUseGeometryParametricBounds() is \a false, otherwise the
    *  the x-bounds of Geometry3D::GetParametricBounds() are used.
    *  \sa m_XResolution
    */
    itkGetMacro(XResolution, int);
    /** \brief Set x-resolution in parameter space
    *
    *  The m_PlaneSource will create this many sub-rectangles
    *  in x-direction (see vtkPlaneSource::SetXResolution)
    *  \note Only used, when GetUseGeometryParametricBounds() is \a false, otherwise the
    *  the x-bounds of Geometry3D::GetParametricBounds() are used.
    *  \sa m_XResolution
    */
    itkSetMacro(XResolution, int);

    /** \brief Get y-resolution in parameter space
    *
    *  The m_PlaneSource will create this many sub-rectangles
    *  in y-direction (see vtkPlaneSource::SetYResolution)
    *  \note Only used, when GetUseGeometryParametricBounds() is \a false, otherwise the
    *  the y-bounds of Geometry3D::GetParametricBounds() are used.
    *  \sa m_YResolution
    */
    itkGetMacro(YResolution, int);

    /** \brief Set y-resolution in parameter space
    *
    *  The m_PlaneSource will create this many sub-rectangles
    *  in y-direction (see vtkPlaneSource::SetYResolution)
    *  \note Only used, when GetUseGeometryParametricBounds() is \a false, otherwise the
    *  the y-bounds of Geometry3D::GetParametricBounds() are used.
    *  \sa m_YResolution
    */
    itkSetMacro(YResolution, int);

    /** \brief Get whether the Surface is at the origin and placed using the Geometry
    *
    *  Default is \a false, i.e., the transform of the Geometry is the identity, thus
    *  the points within the Surface are at their final position. Otherwise
    *  (m_PlaceByGeometry==\a true), the first cornerpoint of the created Surface is
    *  at the origin and the actual position is determined by the transform of the
    *  Geometry.
    *  \sa m_PlaceByGeometry
    */
    itkGetConstMacro(PlaceByGeometry, bool);

    /** \brief Set whether the Surface is at the origin and placed using the Geometry
    *
    *  Default is \a false, i.e., the transform of the Geometry is the identity, thus
    *  the points within the Surface are at their final position. Otherwise
    *  (m_PlaceByGeometry==\a true), the first cornerpoint of the created Surface is
    *  at the origin and the actual position is determined by the transform of the
    *  Geometry.
    *  \sa m_PlaceByGeometry
    */
    itkSetMacro(PlaceByGeometry, bool);
    itkBooleanMacro(PlaceByGeometry);

    itkGetConstMacro( UseBoundingBox, bool );
    itkSetMacro( UseBoundingBox, bool );
    itkBooleanMacro( UseBoundingBox );

    void SetBoundingBox( const BoundingBox *boundingBox );
    const BoundingBox *GetBoundingBox() const;

  protected:

    PlaneGeometryDataToSurfaceFilter();

    virtual ~PlaneGeometryDataToSurfaceFilter();

    /** \brief Source to create the vtk-representation of the parameter space rectangle of the PlaneGeometry
    */
    vtkPlaneSource* m_PlaneSource;

    /** \brief Filter to create the vtk-representation of the PlaneGeometry, which is a
    *  transformation of the m_PlaneSource
    */
    vtkTransformPolyDataFilter* m_VtkTransformPlaneFilter;

    /** \brief If \a true, use Geometry3D::GetParametricBounds() to define the resolution in parameter space,
    *  otherwise use m_XResolution and m_YResolution
    */
    bool m_UseGeometryParametricBounds;

    /** \brief X-resolution in parameter space
    *
    *  The m_PlaneSource will create this many sub-rectangles
    *  in x-direction (see vtkPlaneSource::SetXResolution)
    *  \note Only used, when GetUseGeometryParametricBounds() is \a false, otherwise the
    *  the x-bounds of Geometry3D::GetParametricBounds() are used.
    *  \sa m_XResolution
    */
    int m_XResolution;

    /** \brief Y-resolution in parameter space
    *
    *  The m_PlaneSource will create this many sub-rectangles
    *  in y-direction (see vtkPlaneSource::SetYResolution)
    *  \note Only used, when GetUseGeometryParametricBounds() is \a false, otherwise the
    *  the y-bounds of Geometry3D::GetParametricBounds() are used.
    */
    int m_YResolution;

    /** \brief Define whether the Surface is at the origin and placed using the Geometry
    *
    *  Default is \a false, i.e., the transform of the Geometry is the identity, thus
    *  the points within the Surface are at their final position. Otherwise
    *  (m_PlaceByGeometry==\a true), the first cornerpoint of the created Surface is
    *  at the origin and the actual position is determined by the transform of the
    *  Geometry.
    */
    bool m_PlaceByGeometry;

    bool m_UseBoundingBox;

    BoundingBox::ConstPointer m_BoundingBox;

    vtkCubeSource *m_CubeSource;
    vtkTransform *m_Transform;
    vtkTransformPolyDataFilter *m_PolyDataTransformer;

    vtkPlane *m_Plane;
    vtkCutter *m_PlaneCutter;
    vtkStripper *m_PlaneStripper;
    vtkPolyData *m_PlanePolyData;
    vtkPPolyDataNormals * m_NormalsUpdater;
    vtkTriangleFilter *m_PlaneTriangler;
    vtkTextureMapToPlane *m_TextureMapToPlane;

    vtkBox *m_Box;
    vtkClipPolyData *m_PlaneClipper;
  };
} // namespace mitk

#endif /* MITKGEOMETRY2DDATATOSURFACEDATAFILTER_H_HEADER_INCLUDED_C10B22CD */
