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


#ifndef GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD
#define GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD

#include "mitkCommon.h"
#include "mitkVector.h"
#include <itkAffineGeometryFrame.h>
#include "mitkOperationActor.h"
#include <itkBoundingBox.h>

class vtkTransform;
class Operation;

namespace mitk {

//##ModelId=3E8600D800C5
//##Documentation
//## @brief Standard 3D-BoundingBox typedef
//##
//## Standard 3D-BoundingBox typedef to get rid of template arguments (3D, type).
typedef itk::BoundingBox<unsigned long, 3, mitk::ScalarType>   BoundingBox;

//##Documentation
//## @brief Standard typedef for time-bounds
typedef itk::FixedArray<mitk::ScalarType,2> TimeBounds;

typedef itk::AffineGeometryFrame<mitk::ScalarType, 3> AffineGeometryFrame3D;

//##ModelId=3DCBF389032B
//##Documentation
//## @brief Describes the geometry of a data object
//## @ingroup Geometry
//## Describes the geometry of a data object. At least, it can
//## return the bounding box of the data object. An important etpaub-class is
//## SlicedGeometry3D, which descibes data objects consisting of
//## slices, e.g., objects of type Image.
//## Conversions between world coordinates (in mm) and unit coordinates 
//## (e.g., pixels in the case of an Image) can be performed.
//## 
//## Geometry3D have to be initialized in the method GenerateOutputInformation() 
//## of BaseProcess (or CopyInformation/ UpdateOutputInformation of BaseData, 
//## if possible, e.g., by analyzing pic tags in Image) subclasses. See also
//## itk::ProcessObject::GenerateOutputInformation(),
//## itk::DataObject::CopyInformation() and
//## itk::DataObject::UpdateOutputInformation().
//## 
//## Rule: everything is in mm (ms) if not stated otherwise.
class Geometry3D : public AffineGeometryFrame3D, public OperationActor
{
public:
  mitkClassMacro(Geometry3D, AffineGeometryFrame3D);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  virtual void SetFloatBounds(const float bounds[6]);

  //##Documentation
  //## @brief Get the parametric bounding-box
  //## 
  itkGetConstObjectMacro(ParametricBoundingBox, BoundingBox);
#if ITK_VERSION_MINOR <= 6
  const BoundingBox::BoundsArrayType GetParametricBounds() const
#else
  const BoundingBox::BoundsArrayType& GetParametricBounds() const
#endif 
  {
    assert(m_ParametricBoundingBox.IsNotNull());
    return m_ParametricBoundingBox->GetBounds();
  }

  virtual void SetParametricBounds(const BoundingBox::BoundsArrayType& bounds);
  virtual void SetFloatParametricBounds(const float bounds[6]);
 
  mitk::ScalarType GetParametricExtent(int direction) const
  {
    assert(direction>=0 && direction<3);
    assert(m_ParametricBoundingBox.IsNotNull());

    BoundingBoxType::BoundsArrayType bounds = m_ParametricBoundingBox->GetBounds();
    return bounds[direction*2+1]-bounds[direction*2];
  }
 
  virtual mitk::ScalarType GetParametricExtentInMM(int direction) const
  {
    return GetExtentInMM(direction);
  }

  virtual void SetIndexToWorldTransform(mitk::AffineTransform3D* transform);

  itkGetConstReferenceMacro(TimeBoundsInMS, TimeBounds);
  virtual void SetTimeBoundsInMS(const TimeBounds& timebounds);

  Point3D GetCornerPoint(int id) const
  {
    assert(id >= 0);
    assert(m_BoundingBox.IsNotNull());

    BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();

    Point3D cornerpoint;
    switch(id)
    {
      case 0: FillVector3D(cornerpoint, bounds[0],bounds[2],bounds[4]); break;
      case 1: FillVector3D(cornerpoint, bounds[0],bounds[2],bounds[5]); break;
      case 2: FillVector3D(cornerpoint, bounds[0],bounds[3],bounds[4]); break;
      case 3: FillVector3D(cornerpoint, bounds[0],bounds[3],bounds[5]); break;
      case 4: FillVector3D(cornerpoint, bounds[1],bounds[2],bounds[4]); break;
      case 5: FillVector3D(cornerpoint, bounds[1],bounds[2],bounds[5]); break;
      case 6: FillVector3D(cornerpoint, bounds[1],bounds[3],bounds[4]); break;
      case 7: FillVector3D(cornerpoint, bounds[1],bounds[3],bounds[5]); break;
      default: assert(id < 8);
    }

    return m_IndexToWorldTransform->TransformPoint(cornerpoint);
  }

  Point3D GetCornerPoint(bool xFront=true, bool yFront=true, bool zFront=true) const
  {
    assert(m_BoundingBox.IsNotNull());
    BoundingBox::BoundsArrayType bounds = m_BoundingBox->GetBounds();

    Point3D cornerpoint;
    cornerpoint[0] = (xFront ? bounds[0] : bounds[1]);
    cornerpoint[1] = (yFront ? bounds[2] : bounds[3]);
    cornerpoint[2] = (zFront ? bounds[4] : bounds[5]);

    return m_IndexToWorldTransform->TransformPoint(cornerpoint);
  }

  //##Documentation
  //## @brief Get vector along bounding-box in the specified @a direction in mm
  //##
  //## The length of the vector is the size of the bounding-box in the 
  //## specified @a direction in mm
  Vector3D GetAxisVector(unsigned int direction) const
  {
    Vector3D frontToBack;
    frontToBack.Set_vnl_vector(m_IndexToWorldTransform->GetMatrix().GetVnlMatrix().get_column(direction));
    frontToBack *= GetExtent(direction);
    return frontToBack;
  }

  //##Documentation
  //## @brief Get the center of the bounding-box in mm
  //## 
  Point3D GetCenter() const 
  {    
    assert(m_BoundingBox.IsNotNull());
    return m_IndexToWorldTransform->TransformPoint(m_BoundingBox->GetCenter());  
  }

  //##Documentation
  //## @brief Get the squared length of the diagonal of the bounding-box in mm
  //## 
  double GetDiagonalLength2() const
  {
    Vector3D diagonalvector = GetCornerPoint()-GetCornerPoint(false, false, false);
    return diagonalvector.GetSquaredNorm();
  }

  //##Documentation
  //## @brief Get the length of the diagonal of the bounding-box in mm
  //## 
  double GetDiagonalLength() const
  {
    return sqrt(GetDiagonalLength2());
  }

  //##Documentation
  //## @brief Get a VnlVector along bounding-box in the specified 
  //## @a direction (length is spacing)
  //##
  VnlVector GetMatrixColumn(unsigned int direction) const
  {
    return m_IndexToWorldTransform->GetMatrix().GetVnlMatrix().get_column(direction);
  }

  //##Documentation
  //## @brief Get the extent of the bounding-box in the specified @a direction in mm
  //##
  ScalarType GetExtentInMM(int direction) const
  {
    return m_IndexToWorldTransform->GetMatrix().GetVnlMatrix().get_column(direction).magnitude()*GetExtent(direction);
  }

  //##Documentation
  //## @brief Set the extent of the bounding-box in the specified @a direction in mm
  //##
  //## @note This changes the matrix in the transform, @a not the bounds, which are given in units!
  void SetExtentInMM(int direction, ScalarType extentInMM)
  {
    ScalarType len = GetExtentInMM(direction);
    if(fabs(len - extentInMM)>=mitk::eps)
    {
      AffineTransform3D::MatrixType::InternalMatrixType vnlmatrix;
      vnlmatrix = m_IndexToWorldTransform->GetMatrix().GetVnlMatrix();
      if(len>extentInMM)
        vnlmatrix.set_column(direction, vnlmatrix.get_column(direction)/len*extentInMM);
      else
        vnlmatrix.set_column(direction, vnlmatrix.get_column(direction)*extentInMM/len);
      Matrix3D matrix;
      matrix = vnlmatrix;
      m_IndexToWorldTransform->SetMatrix(matrix);
      Modified();
    }
  }

  vtkTransform* GetVtkTransform();

  itkGetConstObjectMacro(ParametricTransform, mitk::Transform3D);

  //##ModelId=3DDE65D1028A
  void MMToUnits(const mitk::Point3D &pt_mm, mitk::Point3D &pt_units) const;

  //##ModelId=3DDE65DC0151
  void UnitsToMM(const mitk::Point3D &pt_units, mitk::Point3D &pt_mm) const;

  //##ModelId=3E3B986602CF
  void MMToUnits(const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units) const;

  //##ModelId=3E3B987503A3
  void UnitsToMM(const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm) const;

  //##ModelId=3E3453C703AF
  virtual void Initialize();

  itkGetConstMacro(FrameOfReferenceID, unsigned int);
  itkSetMacro(FrameOfReferenceID, unsigned int);

  void TransferItkToVtkTransform();

  void TransferVtkToITKTransform();

  //##Documentation
  //## @brief Calculates a bounding-box around the geometry relative 
  //## to a coordinate system defined by a transform
  //##
  mitk::BoundingBox::Pointer CalculateBoundingBoxRelativeToTransform(const mitk::AffineTransform3D* transform);

  //##Documentation
  //## @brief clones the geometry
  //##
  //## Overwrite in all sub-classes.
  //## Normally looks like:
  //## \code
  //##  Self::Pointer newGeometry = Self::New();
  //##  newGeometry->Initialize();
  //##  InitializeGeometry(newGeometry);
  //##  return newGeometry.GetPointer();
  //## \endcode
  //## \sa InitializeGeometry
  virtual AffineGeometryFrame3D::Pointer Clone() const;

  //##Documentation
  //##@brief executes affine operations (translate, rotate, scale)
  void ExecuteOperation(Operation* operation); 

  const Vector3D GetXAxis();
  const Vector3D GetYAxis();
  const Vector3D GetZAxis();
protected:
  Geometry3D();

  //##ModelId=3E3456C50067
  virtual ~Geometry3D();

  //##Documentation
  //## @brief used in clone to initialize the newly created geometry
  //##
  //## Has to be overwritten in sub-classes, if they add members.
  //## Do the following:
  //## \li call Superclass::InitializeGeometry(newGeometry)
  //## \li transfer all additional members of Self compared to Superclass
  virtual void InitializeGeometry(Self * newGeometry) const;

  mutable mitk::BoundingBox::Pointer m_ParametricBoundingBox;

  mutable mitk::TimeBounds m_TimeBoundsInMS;

  vtkTransform* m_VtkIndexToWorldTransform;
  
  mitk::Transform3D::Pointer m_ParametricTransform;

  unsigned int m_FrameOfReferenceID;
};

} // namespace mitk

#endif /* GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD */
