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

#ifndef MITKVTKABSTRACTTRANSFORMPLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C
#define MITKVTKABSTRACTTRANSFORMPLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C

#include <MitkCoreExports.h>
#include "mitkPlaneGeometry.h"

#include "itkVtkAbstractTransform.h"

class vtkAbstractTransform;

namespace mitk {
  //##Documentation
  //## @brief Describes a geometry defined by an vtkAbstractTransform and a plane
  //##
  //## vtkAbstractTransform is the most general transform in vtk (superclass for
  //## all vtk geometric transformations). It defines an arbitrary 3D transformation,
  //## i.e., a transformation of 3D space into 3D space. In contrast,
  //## AbstractTransformGeometry (since it is a subclass of PlaneGeometry) describes a
  //## 2D manifold in 3D space. The 2D manifold is defined as the manifold that results
  //## from transforming a rectangle (given in m_Plane as a PlaneGeometry) by the
  //## vtkAbstractTransform (given in m_VtkAbstractTransform).
  //## The PlaneGeometry m_Plane is used to define the parameter space. 2D coordinates are
  //## first mapped by the PlaneGeometry and the resulting 3D coordinates are put into
  //## the vtkAbstractTransform.
  //## @note This class is the superclass of concrete geometries. Since there is no
  //## write access to the vtkAbstractTransform and m_Plane, this class is somehow
  //## abstract. For full write access from extern, use ExternAbstractTransformGeometry.
  //## @note The bounds of the PlaneGeometry are used as the parametric bounds.
  //## @sa ExternAbstractTransformGeometry
  //## @ingroup Geometry
  class MITK_CORE_EXPORT AbstractTransformGeometry : public PlaneGeometry
  {
  public:
    mitkClassMacro(AbstractTransformGeometry, PlaneGeometry);

    itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      //##Documentation
      //## @brief Get the vtkAbstractTransform (stored in m_VtkAbstractTransform)
      virtual vtkAbstractTransform* GetVtkAbstractTransform() const;

    virtual unsigned long GetMTime() const;

    //##Documentation
    //## @brief Get the rectangular area that is used for transformation by
    //## m_VtkAbstractTransform and therewith defines the 2D manifold described by
    //## AbstractTransformGeometry
    itkGetConstObjectMacro(Plane, PlaneGeometry);

    /**
    * \brief projects the given point onto the curved plane
    */
    virtual bool Project(const mitk::Point3D &pt3d_mm, mitk::Point3D &projectedPt3d_mm) const;

    /**
    * \brief projects a given vector starting from given point onto the curved plane
    * \warning no satisfiyng implementation existing yet
    */
    virtual bool Project(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector3D &projectedVec3d_mm) const;

    /**
    * \brief projects a given vector starting from standard point onto the curved plane
    * \warning no satisfying implementation existing yet
    */
    virtual bool Project(const mitk::Vector3D &vec3d_mm, mitk::Vector3D &projectedVec3d_mm) const;

    virtual bool Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const;

    virtual void Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const;

    virtual bool Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const;

    virtual void Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const;

    virtual void IndexToWorld(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const;

    virtual void WorldToIndex(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const;

    //##Documentation
    //## @brief Convert (continuous or discrete) index coordinates of a \em vector
    //## \a vec_units to world coordinates (in mm)
    //## @deprecated First parameter (Point2D) is not used. If possible, please use void IndexToWorld(const mitk::Vector2D& vec_units, mitk::Vector2D& vec_mm) const.
    //## For further information about coordinates types, please see the Geometry documentation
    virtual void IndexToWorld(const mitk::Point2D &atPt2d_units, const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const;

    //##Documentation
    //## @brief Convert (continuous or discrete) index coordinates of a \em vector
    //## \a vec_units to world coordinates (in mm)
    //## For further information about coordinates types, please see the Geometry documentation
    virtual void IndexToWorld(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const;

    //##Documentation
    //## @brief Convert world coordinates (in mm) of a \em vector
    //## \a vec_mm to (continuous!) index coordinates.
    //## @deprecated First parameter (Point2D) is not used. If possible, please use void WorldToIndex(const mitk::Vector2D& vec_mm, mitk::Vector2D& vec_units) const.
    //## For further information about coordinates types, please see the Geometry documentation
    virtual void WorldToIndex(const mitk::Point2D &atPt2d_mm, const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const;

    //##Documentation
    //## @brief Convert world coordinates (in mm) of a \em vector
    //## \a vec_mm to (continuous!) index coordinates.
    //## For further information about coordinates types, please see the Geometry documentation
    virtual void WorldToIndex(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const;

    virtual bool IsAbove(const Point3D& pt3d_mm, bool considerBoundingBox=false) const;

    virtual mitk::ScalarType GetParametricExtentInMM(int direction) const;

  virtual const itk::Transform<mitk::ScalarType, 3, 3>* GetParametricTransform() const;

    //##Documentation
    //## @brief Change the parametric bounds to @a oversampling times
    //## the bounds of m_Plane.
    //##
    //## The change is done once (immediately). Later changes of the bounds
    //## of m_Plane will not influence the parametric bounds. (Consequently,
    //## there is no method to get the oversampling.)
    virtual void SetOversampling(mitk::ScalarType oversampling);

    //##Documentation
    //## @brief Calculates the standard part of a BaseGeometry
    //## (IndexToWorldTransform and bounding box) around the
    //## curved geometry. Has to be implemented in subclasses.
    //##
    //## \sa SetFrameGeometry
    virtual void CalculateFrameGeometry();

    //##Documentation
    //## @brief Set the frame geometry which is used as the standard
    //## part of an BaseGeometry (IndexToWorldTransform and bounding box)
    //##
    //## Maybe used as a hint within which the interpolation shall occur
    //## by concrete sub-classes.
    //## \sa CalculateFrameGeometry
    virtual void SetFrameGeometry(const mitk::BaseGeometry* frameGeometry);

    virtual itk::LightObject::Pointer InternalClone() const;

    //##Documentation
    //## @brief Get the parametric bounding-box
    //##
    //## See AbstractTransformGeometry for an example usage of this.
    itkGetConstObjectMacro(ParametricBoundingBox, BoundingBox);

    //##Documentation
    //## @brief Get the parametric bounds
    //##
    //## See AbstractTransformGeometry for an example usage of this.
    const BoundingBox::BoundsArrayType& GetParametricBounds() const;

    //##Documentation
    //## @brief Get the parametric extent
    //##
    //## See AbstractTransformGeometry for an example usage of this.
    mitk::ScalarType GetParametricExtent(int direction) const;

  protected:
    AbstractTransformGeometry();
    AbstractTransformGeometry(const AbstractTransformGeometry& other);

    virtual ~AbstractTransformGeometry();

    //##Documentation
    //## @brief Set the vtkAbstractTransform (stored in m_VtkAbstractTransform)
    //##
    //## Protected in this class, made public in ExternAbstractTransformGeometry.
    virtual void SetVtkAbstractTransform(vtkAbstractTransform* aVtkAbstractTransform);

    //##Documentation
    //## @brief Set the rectangular area that is used for transformation by
    //## m_VtkAbstractTransform and therewith defines the 2D manifold described by
    //## ExternAbstractTransformGeometry
    //##
    //## Protected in this class, made public in ExternAbstractTransformGeometry.
    //## @note The bounds of the PlaneGeometry are used as the parametric bounds.
    //## @note The PlaneGeometry is cloned, @em not linked/referenced.
    virtual void SetPlane(const mitk::PlaneGeometry* aPlane);

    //##Documentation
    //## @brief The rectangular area that is used for transformation by
    //## m_VtkAbstractTransform and therewith defines the 2D manifold described by
    //## AbstractTransformGeometry.
    mitk::PlaneGeometry::Pointer m_Plane;

    itk::VtkAbstractTransform<ScalarType>::Pointer m_ItkVtkAbstractTransform;

    mitk::BaseGeometry::Pointer m_FrameGeometry;

    //##Documentation
    //## @brief Set the parametric bounds
    //##
    //## Protected in this class, made public in some sub-classes, e.g.,
    //## ExternAbstractTransformGeometry.
    virtual void SetParametricBounds(const BoundingBox::BoundsArrayType& bounds);

    mutable mitk::BoundingBox::Pointer m_ParametricBoundingBox;
  private:
    virtual void PostInitialize();
  };
} // namespace mitk
#endif /* MITKVTKABSTRACTTRANSFORMPLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
