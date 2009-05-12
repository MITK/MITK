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


#ifndef MITKVTKABSTRACTTRANSFORMPLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C
#define MITKVTKABSTRACTTRANSFORMPLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C

#include "mitkCommon.h"
#include "mitkGeometry2D.h"
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
//## AbstractTransformGeometry (since it is a subclass of Geometry2D) describes a 
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
class MITK_CORE_EXPORT AbstractTransformGeometry : public Geometry2D
{
public:
  mitkClassMacro(AbstractTransformGeometry, Geometry2D);
  
  itkNewMacro(Self);
  
  //##Documentation
  //## @brief Get the vtkAbstractTransform (stored in m_VtkAbstractTransform)
  virtual vtkAbstractTransform* GetVtkAbstractTransform() const;

  virtual unsigned long GetMTime() const;

  //##Documentation
  //## @brief Get the rectangular area that is used for transformation by 
  //## m_VtkAbstractTransform and therewith defines the 2D manifold described by 
  //## AbstractTransformGeometry
  itkGetConstObjectMacro(Plane, PlaneGeometry);
  
  virtual bool Project(const mitk::Point3D &pt3d_mm, mitk::Point3D &projectedPt3d_mm) const;

  virtual bool Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const;
  
  virtual void Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const;
    
  virtual bool Project(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector3D &projectedVec3d_mm) const;

  virtual bool Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const;
  
  virtual void Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const;

  virtual void IndexToWorld(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const;

  virtual void WorldToIndex(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const;

  virtual void IndexToWorld(const mitk::Point2D &atPt2d_untis, const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const;

  virtual void WorldToIndex(const mitk::Point2D &atPt2d_mm, const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const;

  virtual bool IsAbove(const Point3D& pt3d_mm) const;

  virtual mitk::ScalarType GetParametricExtentInMM(int direction) const;

  virtual const Transform3D* GetParametricTransform() const;

  //##Documentation
  //## @brief Change the parametric bounds to @a oversampling times 
  //## the bounds of m_Plane.
  //##
  //## The change is done once (immediately). Later changes of the bounds
  //## of m_Plane will not influence the parametric bounds. (Consequently, 
  //## there is no method to get the oversampling.)
  virtual void SetOversampling(float oversampling);

  virtual void Initialize();

  //##Documentation
  //## @brief Calculates the standard part of a Geometry3D 
  //## (IndexToWorldTransform and bounding box) around the 
  //## curved geometry. Has to be implemented in subclasses.
  //##
  //## \sa SetFrameGeometry
  virtual void CalculateFrameGeometry();

  //##Documentation
  //## @brief Set the frame geometry which is used as the standard
  //## part of an Geometry3D (IndexToWorldTransform and bounding box)
  //##
  //## Maybe used as a hint within which the interpolation shall occur 
  //## by concrete sub-classes.
  //## \sa CalculateFrameGeometry
  virtual void SetFrameGeometry(const mitk::Geometry3D* frameGeometry);

  virtual AffineGeometryFrame3D::Pointer Clone() const;
protected:
  AbstractTransformGeometry();
  virtual ~AbstractTransformGeometry();
  
  void InitializeGeometry(Self * newGeometry) const;
  
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

  mitk::Geometry3D::Pointer m_FrameGeometry;
};

} // namespace mitk
#endif /* MITKVTKABSTRACTTRANSFORMPLANEGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
