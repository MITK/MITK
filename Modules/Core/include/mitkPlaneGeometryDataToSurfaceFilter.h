/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlaneGeometryDataToSurfaceFilter_h
#define mitkPlaneGeometryDataToSurfaceFilter_h

#include <mitkGeometry3D.h>
#include <mitkSurfaceSource.h>
#include <vtkSystemIncludes.h>

class vtkPlaneSource;
class vtkTransformPolyDataFilter;
class vtkCubeSource;
class vtkTransform;
class vtkPlane;
class vtkCutter;
class vtkPolyData;
class vtkPPolyDataNormals;
class vtkTextureMapToPlane;
class vtkBox;
class vtkClipPolyData;
class vtkContourTriangulator;

namespace mitk
{
  class PlaneGeometryData;
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
  class MITKCORE_EXPORT PlaneGeometryDataToSurfaceFilter : public SurfaceSource
  {
  public:
    mitkClassMacro(PlaneGeometryDataToSurfaceFilter, SurfaceSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Generate information about the output (size, spacing, etc.). */
    void GenerateOutputInformation() override;

    /** \brief Generate the Surface output from the PlaneGeometryData input. */
    void GenerateData() override;

    /** \brief Get the first PlaneGeometryData input. */
    const PlaneGeometryData *GetInput(void);

    /**
     * \brief Get the PlaneGeometryData input at the given index.
     * \param idx The input index.
     */
    const PlaneGeometryData *GetInput(unsigned int idx);

    /**
     * \brief Set the PlaneGeometryData input (at index 0).
     * \param image The PlaneGeometryData to use as input.
     */
    virtual void SetInput(const PlaneGeometryData *image);
    using itk::ProcessObject::SetInput;

    /**
     * \brief Set the PlaneGeometryData input at the given index.
     * \param index The input index.
     * \param image The PlaneGeometryData to use as input.
     */
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

    /** \brief Get whether a bounding box is used to clip the generated surface. */
    itkGetConstMacro(UseBoundingBox, bool);
    /** \brief Set whether a bounding box should be used to clip the generated surface. */
    itkSetMacro(UseBoundingBox, bool);
    /** \brief Toggle use of bounding box for clipping. */
    itkBooleanMacro(UseBoundingBox);

    /**
     * \brief Set the bounding box used to clip the generated surface.
     * \param boundingBox The bounding box. Only used when UseBoundingBox is true.
     */
    void SetBoundingBox(const BoundingBox *boundingBox);

    /**
     * \brief Get the bounding box used to clip the generated surface.
     * \return The current bounding box, or nullptr if none is set.
     */
    const BoundingBox *GetBoundingBox() const;

  protected:
    PlaneGeometryDataToSurfaceFilter();

    ~PlaneGeometryDataToSurfaceFilter() override;

    /** \brief Source to create the vtk-representation of the parameter space rectangle of the PlaneGeometry
    */
    vtkPlaneSource *m_PlaneSource;

    /** \brief Filter to create the vtk-representation of the PlaneGeometry, which is a
    *  transformation of the m_PlaneSource
    */
    vtkTransformPolyDataFilter *m_VtkTransformPlaneFilter;

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
    vtkContourTriangulator *m_ContourTriangulator;
    vtkPPolyDataNormals *m_NormalsUpdater;
    vtkTextureMapToPlane *m_TextureMapToPlane;

    vtkBox *m_Box;
    vtkClipPolyData *m_PlaneClipper;
  };
} // namespace mitk

#endif
