#ifndef MITKABSTRACTTRANSFORMGEOMETRY_H_HEADER_INCLUDED_C1C68A2C
#define MITKABSTRACTTRANSFORMGEOMETRY_H_HEADER_INCLUDED_C1C68A2C

#include "mitkCommon.h"
#include "mitkGeometry2D.h"
#include "SpaceGeometry.h"

class vtkTransform;
class vtkAbstractTransform;

namespace mitk {

//##Documentation
//## @brief Describes a geometry defined by an vtkAbstractTransform
//## @ingroup Geometry
//## vtkAbstractTransform is the most general transform in vtk 
//## (superclass for all vtk geometric transformations).
//## It defines an arbitrary 3D transformation, i.e., a transformation
//## of 3D space into 3D space. In contrast, AbstractTransformGeometry
//## (since it is a subclass of Geometry2D) describes a 2D manifold in
//## 3D space. The 2D manifold is defined as the manifold that results
//## from transforming a rectangle (given in m_PlaneView) by the
//## vtkAbstractTransform (given in m_VtkAbstractTransform). 
//## @todo GetMTime is overwritten to include the timestamp of the
//## VtkAbstractTransform int the MTime. The implementation of this is
//## probably not final. One thing is strange: calling TransformPoint
//## of vtkThinPlateSplineTransform causes a change of the returned
//## MTime -> check with an Debug-version of vtk.
class AbstractTransformGeometry : public Geometry2D
{
public:
  mitkClassMacro(AbstractTransformGeometry,Geometry2D);
  
  itkNewMacro(Self);
  
  //##ModelId=3EF4A2660237
  //##Documentation
  //## @brief Get the vtkAbstractTransform (stored in m_VtkAbstractTransform)
  virtual vtkAbstractTransform* GetVtkAbstractTransform() const;
  
  //##ModelId=3EF4A2660239
  //##Documentation
  //## @brief Set the vtkAbstractTransform (stored in m_VtkAbstractTransform)
  virtual void SetVtkAbstractTransform(vtkAbstractTransform* aVtkAbstractTransform);
  
  //##ModelId=3EF4A2660241
  //##Documentation
  //## @brief Get the rectangular area that is used for transformation by 
  //## m_VtkAbstractTransform and therewith defines the 2D manifold described by 
  //## AbstractTransformGeometry
  virtual const mitk::PlaneView& GetPlaneView() const;
  //##ModelId=3EF4A2660243
  //##Documentation
  //## @brief Set the rectangular area that is used for transformation by 
  //## m_VtkAbstractTransform and therewith defines the 2D manifold described by 
  //## AbstractTransformGeometry
  virtual void SetPlaneView(const mitk::PlaneView& aPlaneView);
  
  //##ModelId=3EF4A266024C
  //##Documentation
  //## @todo not yet implemented
  virtual void TransformGeometry(const vtkTransform * transform);
  
  //##ModelId=3EF4A2660256
  virtual bool Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const;
  
  //##ModelId=3EF4A266025F
  virtual void Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const;
  
  //##ModelId=3EF4A2660263
  virtual void UnitsToMM(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const;
  
  //##ModelId=3EF4A266026C
  virtual void MMToUnits(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const;
  
  //##ModelId=3EF4A2660275
  virtual void UnitsToMM(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const;
  
  //##ModelId=3EF4A266027E
  virtual void MMToUnits(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const;
  
  //##ModelId=3EF4A2660288
  virtual bool Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const;
  
  //##ModelId=3EF4A2660292
  virtual void Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const;
  
  virtual unsigned long GetMTime() const;
protected:
  //##ModelId=3EF4A266029C
  AbstractTransformGeometry();
  //##ModelId=3EF4A266029D
  virtual ~AbstractTransformGeometry();
  
  //##ModelId=3EF4A26601A2
  //##Documentation
  //## @brief Instance of the vtkAbstractTransform
  vtkAbstractTransform* m_VtkAbstractTransform;
  
  //##ModelId=3EF4A2660219
  //##Documentation
  //## @brief Instance of the vtkAbstractTransform
  vtkAbstractTransform* m_InverseVtkAbstractTransform;
  
  //##ModelId=3EF4A266021A
  //##Documentation
  //## @brief The rectangular area that is used for transformation by 
  //## m_VtkAbstractTransform and therewith defines the 2D manifold described by 
  //## AbstractTransformGeometry.
  mitk::PlaneView m_PlaneView;
  
  //##ModelId=3EF4A2660223
  //##Documentation
  //## @brief factor to convert x-coordinates from mm to units and vice versa
  //## 
  //## Is calculated in SetPlaneView from the value of m_WidthInUnits and the
  //## PlaneView
  float m_ScaleFactorMMPerUnitX;
  //##ModelId=3EF4A266022D
  //##Documentation
  //## @brief factor to convert y-coordinates from mm to units and vice versa
  //## 
  //## Is calculated in SetPlaneView from the value of m_HeightInUnits and
  //## the PlaneView
  float m_ScaleFactorMMPerUnitY;

  mutable unsigned long m_LastVtkAbstractTransformTimeStamp;
};

} // namespace mitk
#endif /* MITKABSTRACTTRANSFORMGEOMETRY_H_HEADER_INCLUDED_C1C68A2C */
