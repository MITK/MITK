#ifndef GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0
#define GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0

#include "mitkCommon.h"
#include "mitkGeometry3D.h"

class vtkTransform;

namespace mitk {

//##ModelId=3DDE4DEC0291
//##Documentation
//## @brief Describes the geometry of a two-dimensional object
//## @ingroup Geometry
//## Describes a two-dimensional manifold, i.e., simply speaken, 
//## an object that can be described using a 2D coordinate-system. 
//##
//## Geometry2D can map points between 3D world coordinates 
//## (in mm) and the described 2D coordinate-system (in mm) by first projecting
//## the 3D point onto the 2D manifold and then calculating the 2D-coordinates
//## (in mm). These 2D-mm-coordinates can be further converted into 
//## 2D-unit-coordinates (e.g., pixels), giving a parameter representation of the 
//## object with parameter values inside a rectangle (e.g., [0,0]..[width, height]).
//##
//## A Geometry2D describes the 2D representation within a 3D object and is 
//## therefore itself a Geometry3D (derived from Geometry3D). For example,
//## a single CT-image (slice) is 2D in the sense that you can access the 
//## pixels using 2D-coordinates, but is also 3D, as the pixels are really 
//## voxels, thus have an extension (thickness) in the 3rd dimension.
//## 
//## Most often, instances of Geometry2D will be used to descibe a plane, 
//## which is represented by the sub-class PlaneGeometry, but curved 
//## surfaces are also possible.
//## 
//## @warning The Geometry2Ds are not necessarily up-to-date and not even
//## initialized. As described in the previous paragraph, one of the
//## Generate-/Copy-/UpdateOutputInformation methods have to initialize it.
//## mitk::BaseData::GetGeometry2D() makes sure, that the Geometry2D is
//## up-to-date before returning it (by setting the update extent appropriately
//## and calling UpdateOutputInformation).
//## 
//## Rule: everything is in mm (or ms for temporal information) if not 
//## stated otherwise.
class Geometry2D : public mitk::Geometry3D
{
public:
  mitkClassMacro(Geometry2D, mitk::Geometry3D);
  itkNewMacro(Self);

  /** Method for creation through the object factory. */
  //itkNewMacro(Self);

  /** @brief get the width of the described area in units, e.g., for an image: the
  * number of pixels in x-direction*/
  itkGetConstMacro(WidthInUnits, unsigned int);
  /** @brief set the width of the described area in units, e.g., for an image: the
  * number of pixels in x-direction*/
  itkSetMacro(WidthInUnits, unsigned int);

  /** @brief get the height of the described area in units, e.g., for an image: the
  * number of pixels in y-direction*/
  itkGetConstMacro(HeightInUnits, unsigned int);
  /** @brief set the height of the described area in units, e.g., for an image: the
  * number of pixels in y-direction*/
  itkSetMacro(HeightInUnits, unsigned int);

  //##ModelId=3DDE65E00122
  //##Documentation
  //## @brief Project a 3D point given in mm (@a pt3d_mm) onto the 2D
  //## geometry. The result is a 2D point in mm (@a pt2d_mm).
  //## 
  //## The result is a 2D point in mm (@a pt2d_mm) relative to the upper-left
  //## corner of the geometry. To convert this point into units (e.g., pixels
  //## in case of an image), use MMToUnits.
  //## @return true projection was possible
  //## @sa Project(const mitk::Point3D &pt3d_mm, mitk::Point3D
  //## &projectedPt3d_mm) 
  virtual bool Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const;

  //##ModelId=3DDE65E301DE
  //##Documentation
  //## @brief Converts a 2D point given in mm (@a pt2d_mm) relative to the
  //## upper-left corner of the geometry into the corresponding
  //## world-coordinate (a 3D point in mm, @a pt3d_mm). 
  //## 
  //## To convert a 2D point given in units (e.g., pixels in case of an
  //## image) into a 2D point given in mm (as required by this method), use
  //## UnitsToMM.
  virtual void Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const;

  //##ModelId=3DE7895602F7
  //##Documentation
  //## @brief Convert a 2D point given in units (e.g., pixels in case of an
  //## image) into a 2D point given in mm
  virtual void UnitsToMM(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const;

  //##ModelId=3DE7895C01CE
  //##Documentation
  //## @brief Convert a 2D point given in mm into a 2D point given in mm
  //## (e.g., pixels in case of an image)
  virtual void MMToUnits(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const;

  //##ModelId=3E3B98C5019F
  //##Documentation
  //## @brief Convert a 2D vector given in units (e.g., pixels in case of an
  //## image) into a 2D vector given in mm
  //## @warning strange: in contrast to vtkTransform the class itk::Transform
  //## does not have the parameter, @em where the vector that is to be
  //## transformed is located. This method here should also need this
  //## information for general transforms.
  virtual void UnitsToMM(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const;
  //##ModelId=3E3B98C9019B
  //##Documentation
  //## @brief Convert a 2D vector given in mm into a 2D point vector in mm
  //## (e.g., pixels in case of an image)
  //## @warning strange: in contrast to vtkTransform the class itk::Transform
  //## does not have the parameter, @em where the vector that is to be
  //## transformed is located. This method here should also need this
  //## information for general transforms.
  virtual void MMToUnits(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const;

  //##ModelId=3E3C3A92033E
  //##Documentation
  //## @brief Set the width and height of the geometry in units (stored in
  //## the members m_WidthInUnits, m_HeightInUnits). For an image, this is
  //## the number of pixels in x-/y-direction.
  virtual void SetSizeInUnits(unsigned int width, unsigned int height);

  //##ModelId=3EF48EA10320
  //##Documentation
  //## @brief Project a 3D point given in mm (@a pt3d_mm) onto the 2D
  //## geometry. The result is a 3D point in mm (@a projectedPt3d_mm).
  //## 
  //## @return true projection was possible
  virtual bool Project(const mitk::Point3D &pt3d_mm, mitk::Point3D &projectedPt3d_mm) const;


  //##ModelId=3EF48F170280
  //##Documentation
  //## @brief Project a 3D vector given in mm (@a vec3d_mm) onto the 2D
  //## geometry. The result is a 2D vector in mm (@a vec2d_mm).
  //## 
  //## The result is a 2D vector in mm (@a vec2d_mm) relative to the
  //## upper-left
  //## corner of the geometry. To convert this point into units (e.g., pixels
  //## in case of an image), use MMToUnits.
  //## @return true projection was possible
  //## @sa Project(const mitk::Vector3D &vec3d_mm, mitk::Vector3D
  //## &projectedVec3d_mm) 
  virtual bool Map(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector2D &vec2d_mm) const;


  //##ModelId=3EF48F2E00D4
  //##Documentation
  //## @brief Converts a 2D vector given in mm (@a vec2d_mm) relative to the
  //## upper-left corner of the geometry into the corresponding
  //## world-coordinate (a 3D vector in mm, @a vec3d_mm). 
  //## 
  //## To convert a 2D vector given in units (e.g., pixels in case of an
  //## image) into a 2D vector given in mm (as required by this method), use
  //## UnitsToMM.
  virtual void Map(const mitk::Point2D & atPt2d_mm, const mitk::Vector2D &vec2d_mm, mitk::Vector3D &vec3d_mm) const;


  //##ModelId=3EF48F8F01B0
  //##Documentation
  //## @brief Project a 3D vector given in mm (@a vec3d_mm) onto the 2D
  //## geometry. The result is a 3D vector in mm (@a projectedVec3d_mm).
  //## 
  //## @return true projection was possible
  virtual bool Project(const mitk::Point3D & atPt3d_mm, const mitk::Vector3D &vec3d_mm, mitk::Vector3D &projectedVec3d_mm) const;

protected:
  //##ModelId=3E395E0802E6
  Geometry2D();
  //##ModelId=3E395E080318
  virtual ~Geometry2D();

  //##ModelId=3E3C429101C5
  //##Documentation
  //## @brief width of the described area in units, e.g., for an image: the
  //## number of pixels in x-direction.
  unsigned int m_WidthInUnits;
  //##ModelId=3E3C4291020B
  //##Documentation
  //## @brief width of the described area in units, e.g., for an image: the
  //## number of pixels in y-direction.
  unsigned int m_HeightInUnits;
};

} // namespace mitk

#endif /* GEOMETRY2D_H_HEADER_INCLUDED_C1F4D8E0 */
