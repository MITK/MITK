#ifndef MITKSLICEDGEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD
#define MITKSLICEDGEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD

#include "mitkGeometry3D.h"
#include "mitkGeometry2D.h"
#include <ipPic/ipPic.h>

class vtkTransform;

namespace mitk {

#if _MSC_VER < 1300
#define GImageDimension 4
#endif

//##ModelId=3DCBF389032B
//##Documentation
//## @brief Describes the geometry of a data object consisting of slices
//## @ingroup Geometry
//## A Geometry2D can be requested for each slice.
//## 
//## SlicedGeometry3D and the associated Geometry2Ds have to be
//## initialized in the method GenerateOutputInformation() of BaseProcess (or
//## CopyInformation/ UpdateOutputInformation of BaseData, if possible, e.g.,
//## by analyzing pic tags in Image) subclasses. See also
//## itk::ProcessObject::GenerateOutputInformation(),
//## itk::DataObject::CopyInformation() and
//## itk::DataObject::UpdateOutputInformation().
//## 
//## Rule: everything is in mm (or ms for temporal information) if not 
//## stated otherwise.
class SlicedGeometry3D : public mitk::Geometry3D
{
public:
  mitkClassMacro(SlicedGeometry3D, Geometry3D);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  //##ModelId=3DCBF50C0377
  //##Documentation
  //## Return the Geometry2D of the slice (@a s, @a t).
  //## If (a) we don't have a Geometry2D stored for the requested slice, 
  //## (b) m_EvenlySpaced is activated and (c) the first slice (s=0,t=0) 
  //## is a PlaneGeometry instance, then we calculate geometry of the
  //## requested
  //## as the plane of the first slice shifted by m_Spacing*s.
  //## 
  //## @warning The Geometry2Ds are not necessarily up-to-date and not even
  //## initialized. Geometry2Ds (if applicable) have to be initialized in the
  //## method GenerateOutputInformation() of BaseProcess (or CopyInformation/
  //## UpdateOutputInformation of BaseData, if possible, e.g., by analyzing
  //## pic tags in Image) subclasses. See also
  //## itk::ProcessObject::GenerateOutputInformation(),
  //## itk::DataObject::CopyInformation() and
  //## itk::DataObject::UpdateOutputInformation().
  //## mitk::BaseData::GetGeometry2D() makes sure, that the Geometry2D is
  //## up-to-date before returning it (by setting the update extent
  //## appropriately and calling UpdateOutputInformation).
  mitk::Geometry2D::ConstPointer GetGeometry2D(int s, int t) const;

  //##ModelId=3DCBF5D40253
  virtual mitk::BoundingBox::ConstPointer GetBoundingBox(int t = 0) const;

  //##ModelId=3DCBF5E9037F
  double GetTime(int t) const;

  //##ModelId=3E15578402BD
  //##Documentation
  //## @brief Set Geometry2D of slice @a s and time @a t.
  virtual bool SetGeometry2D(const mitk::Geometry2D* geometry2D, int s, int t = 0);
  //##ModelId=3E155839024F
  //##Documentation
  //## @brief Set this SlicedGeometry3D according to the tags in @a pic. 
  //## 
  //## @a pic can be 2D, 3D or 4D.  For 3D,
  //## set according to tags in @a pic. @a pic can be 2D, 3D or 4D.  For 3D,
  //## the parameter @a s is ignored, for 4D @a s and @a t are ignored.
  //## @return @a false: geometry not changed, either because of inconsistent
  //## data (e.g., dimensions do not match the dimensions of a slice) or
  //## read-only geometry.
  //## @return @a true: geometry successfully updated.
  virtual bool SetGeometry2D(ipPicDescriptor* pic, int s = 0, int t = 0);

  //##ModelId=3E3BE1F10106
  virtual bool IsValidSlice(int s = 0, int t = 0) const;

  //##Documentation
  //## @brief Get the spacing as a float[3] array 
  const float* mitk::SlicedGeometry3D::GetSpacing() const;

  //##ModelId=3E3BE8CF010E
  virtual void SetSpacing(mitk::Vector3D aSpacing);
  virtual void SetSpacing(const float aSpacing[3]);
  //##ModelId=3E3C13F802A6
  virtual void SetEvenlySpaced(bool on = true);
  //##ModelId=3E3C2C37031B
  //##Documentation
  //## Set the spacing according to the tags in @ pic.
  virtual void SetSpacing(ipPicDescriptor* pic);

  virtual Geometry3D::Pointer Clone();

  //##ModelId=3E3453C703AF
  virtual void Initialize(unsigned int dimension, const unsigned int* dimensions);

protected:
  SlicedGeometry3D();

  //##ModelId=3E3456C50067
  virtual ~SlicedGeometry3D();

  //##ModelId=3E3968CC000E
  mutable std::vector<Geometry2D::ConstPointer> m_Geometry2Ds;

  //##ModelId=3E3BE8BF0288
  mitk::Vector3D m_Spacing;

  //##ModelId=3E3C13B70180
  bool m_EvenlySpaced;
};

} // namespace mitk

#endif /* MITKSLICEDGEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD */
