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


#ifndef GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD
#define GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD

#include <MitkExports.h>
#include <mitkCommon.h>
#include "mitkVector.h"
#include "mitkOperationActor.h"

#include <itkIndex.h>
#include <itkBoundingBox.h>
#include <itkQuaternionRigidTransform.h>
#include <itkAffineGeometryFrame.h>

class vtkLinearTransform;
class vtkMatrixToLinearTransform;
class vtkMatrix4x4;

namespace mitk {

//##Documentation
//## @brief Standard 3D-BoundingBox typedef
//##
//## Standard 3D-BoundingBox typedef to get rid of template arguments (3D, type).
typedef itk::BoundingBox<unsigned long, 3, ScalarType>   BoundingBox;

//##Documentation
//## @brief Standard typedef for time-bounds
typedef itk::FixedArray<ScalarType,2>  TimeBounds;
typedef itk::FixedArray<ScalarType, 3> FixedArrayType;

typedef itk::AffineGeometryFrame<ScalarType, 3> AffineGeometryFrame3D;

//##Documentation
//## @brief Describes the geometry of a data object
//##
//## At least, it can return the bounding box of the data object.
//##
//## The class holds
//## \li a bounding box which is axes-parallel in intrinsic coordinates
//## (often integer indices of pixels), to be accessed by
//## GetBoundingBox()
//## \li a transform to convert intrinsic coordinates into a
//## world-coordinate system with coordinates in millimeters
//## and milliseconds (all are floating point values), to
//## be accessed by GetIndexToWorldTransform()
//## \li a life span, i.e. a bounding box in time in ms (with
//## start and end time), to be accessed by GetTimeBounds().
//## The default is minus infinity to plus infinity.
//##
//## Geometry3D and its sub-classes allow converting between
//## intrinsic coordinates (called index or unit coordinates)
//## and world-coordinates (called world or mm coordinates),
//## e.g. WorldToIndex.
//## In case you need integer index coordinates, provide an
//## mitk::Index3D (or itk::Index) as target variable to
//## WorldToIndex, otherwise you will get a continuous index
//## (floating point values).
//##
//## An important sub-class is SlicedGeometry3D, which descibes
//## data objects consisting of slices, e.g., objects of type Image.
//## Conversions between world coordinates (in mm) and unit coordinates
//## (e.g., pixels in the case of an Image) can be performed.
//##
//## For more information on related classes, see \ref Geometry.
//##
//## Geometry3D instances referring to an Image need a slightly
//## different definition of corners, see SetImageGeometry. This
//## is usualy automatically called by Image.
//##
//## Geometry3D have to be initialized in the method GenerateOutputInformation()
//## of BaseProcess (or CopyInformation/ UpdateOutputInformation of BaseData,
//## if possible, e.g., by analyzing pic tags in Image) subclasses. See also
//## itk::ProcessObject::GenerateOutputInformation(),
//## itk::DataObject::CopyInformation() and
//## itk::DataObject::UpdateOutputInformation().
//##
//## Rule: everything is in mm (ms) if not stated otherwise.
//## @ingroup Geometry
class MITK_CORE_EXPORT Geometry3D : public AffineGeometryFrame3D, public OperationActor
{
public:
  mitkClassMacro(Geometry3D, AffineGeometryFrame3D);

  typedef itk::QuaternionRigidTransform< ScalarType > QuaternionTransformType;
  typedef QuaternionTransformType::VnlQuaternionType VnlQuaternionType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  // a bit of a misuse, but we want only doxygen to see the following:
#ifdef DOXYGEN_SKIP
  //##Documentation
  //## @brief Get the transformation used to convert from index
  //## to world coordinates
  itkGetObjectMacro(IndexToWorldTransform, AffineTransform3D);
#endif
  //## @brief Set the transformation used to convert from index
  //## to world coordinates
  virtual void SetIndexToWorldTransform(mitk::AffineTransform3D* transform);
  //##Documentation
  //## @brief Convenience method for setting the ITK transform
  //## (m_IndexToWorldTransform) via an vtkMatrix4x4
  //## \sa SetIndexToWorldTransform
  virtual void SetIndexToWorldTransformByVtkMatrix(vtkMatrix4x4* vtkmatrix);

#ifdef DOXYGEN_SKIP
  //##Documentation
  //## @brief Get bounding box (in index/unit coordinates)
  itkGetConstObjectMacro(BoundingBox, BoundingBoxType);
  //##Documentation
  //## @brief Get bounding box (in index/unit coordinates) as a BoundsArrayType
  const BoundsArrayType GetBounds() const
  {
    assert(m_BoundingBox.IsNotNull());
    return m_BoundingBox->GetBounds();
  }
  //##Documentation
  //## \brief Set the bounding box (in index/unit coordinates)
  //##
  //## Only possible via the BoundsArray to make clear that a
  //## copy of the bounding-box is stored, not a reference to it.
virtual void SetBounds(const BoundsArrayType& bounds);
#endif
  //##Documentation
  //## @brief Set the bounding box (in index/unit coordinates) via a float array
  virtual void SetFloatBounds(const float bounds[6]);
  //##Documentation
  //## @brief Set the bounding box (in index/unit coordinates) via a double array
  virtual void SetFloatBounds(const double bounds[6]);


  //##Documentation
  //## @brief When switching from an Image Geometry to a normal Geometry (and the other way around), you have to change the origin as well (See Geometry Documentation)! This function will change the "isImageGeometry" bool flag and changes the origin respectively.
  virtual void ChangeImageGeometryConsideringOriginOffset( const bool isAnImageGeometry );


  //##Documentation
  //## @brief Checks, if the given geometry can be converted to 2D without information loss
  //## e.g. when a 2D image is saved, the matrix is usually cropped to 2x2, and when you load it back to MITK
  //## it will be filled with standard values. This function checks, if information would be lost during this
  //## procedure
  virtual bool Is2DConvertable();


  //##Documentation
  //## @brief Get the time bounds (in ms)
  itkGetConstReferenceMacro(TimeBounds, TimeBounds);
  //##Documentation
  //## @brief Set the time bounds (in ms)
  virtual void SetTimeBounds(const TimeBounds& timebounds);

  //##Documentation
  //## @brief Get the position of the corner number \a id (in world coordinates)
  //##
  //## See SetImageGeometry for how a corner is defined on images.
  Point3D GetCornerPoint(int id) const;

  //##Documentation
  //## @brief Get the position of a corner (in world coordinates)
  //##
  //## See SetImageGeometry for how a corner is defined on images.
  Point3D GetCornerPoint(bool xFront=true, bool yFront=true, bool zFront=true) const;

  //##Documentation
  //## @brief Get vector along bounding-box in the specified @a direction in mm
  //##
  //## The length of the vector is the size of the bounding-box in the
  //## specified @a direction in mm
  //## \sa GetMatrixColumn
  Vector3D GetAxisVector(unsigned int direction) const
  {
    Vector3D frontToBack;
    frontToBack.SetVnlVector(m_IndexToWorldTransform->GetMatrix().GetVnlMatrix().get_column(direction));
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
  //## @a direction, length is spacing
  //##
  //## \sa GetAxisVector
  VnlVector GetMatrixColumn(unsigned int direction) const
  {
    return m_IndexToWorldTransform->GetMatrix().GetVnlMatrix().get_column(direction);
  }

#ifdef DOXYGEN_SKIP
  //##Documentation
  //## @brief Get the extent of the bounding box (in index/unit coordinates)
  //##
  //## To access the extent in mm use GetExtentInMM
  ScalarType GetExtent(unsigned int direction) const;
#endif

  //##Documentation
  //## @brief Get the extent of the bounding-box in the specified @a direction in mm
  //##
  //## Equals length of GetAxisVector(direction).
  ScalarType GetExtentInMM(int direction) const
  {
    return m_IndexToWorldTransform->GetMatrix().GetVnlMatrix().get_column(direction).magnitude()*GetExtent(direction);
  }

  //##Documentation
  //## @brief Set the extent of the bounding-box in the specified @a direction in mm
  //##
  //## @note This changes the matrix in the transform, @a not the bounds, which are given in units!
  virtual void SetExtentInMM(int direction, ScalarType extentInMM);

  //##Documentation
  //## @brief Get the m_IndexToWorldTransform as a vtkLinearTransform
  vtkLinearTransform* GetVtkTransform() const
  {
    return (vtkLinearTransform*)m_VtkIndexToWorldTransform;
  }

  //##Documentation
  //## @brief Set the origin, i.e. the upper-left corner of the plane
  //##
  virtual void SetOrigin(const Point3D& origin);

  //##Documentation
  //## @brief Translate the origin by a vector
  //##
  virtual void Translate(const Vector3D&  vector);

  //##Documentation
  //## @brief Set the transform to identity
  //##
  virtual void SetIdentity();

  //##Documentation
  //## @brief Compose new IndexToWorldTransform with a given transform.
  //##
  //## This method composes m_IndexToWorldTransform with another transform,
  //## modifying self to be the composition of self and other.
  //## If the argument pre is true, then other is precomposed with self;
  //## that is, the resulting transformation consists of first applying
  //## other to the source, followed by self. If pre is false or omitted,
  //## then other is post-composed with self; that is the resulting
  //## transformation consists of first applying self to the source,
  //## followed by other.
  virtual void Compose( const AffineGeometryFrame3D::TransformType * other, bool pre = 0 );

  //##Documentation
  //## @brief Compose new IndexToWorldTransform with a given vtkMatrix4x4.
  //##
  //## Converts the vtkMatrix4x4 into a itk-transform and calls the previous method.
  virtual void Compose( const vtkMatrix4x4 * vtkmatrix, bool pre = 0 );

  //##Documentation
  //## @brief Get the origin, e.g. the upper-left corner of the plane
  const Point3D& GetOrigin() const
  {
    return m_Origin;
  }

  //##Documentation
  //## @brief Get the origin as VnlVector
  //##
  //## \sa GetOrigin
  VnlVector GetOriginVnl() const
  {
    return const_cast<Self*>(this)->m_Origin.GetVnlVector();
  }

  //##Documentation
  //## @brief Convert world coordinates (in mm) of a \em point to (continuous!) index coordinates
  //## \warning If you need (discrete) integer index coordinates (e.g., for iterating easily over an image),
  //## use WorldToIndex(const mitk::Point3D& pt_mm, itk::Index<VIndexDimension> &index).
  //## For further information about coordinates types, please see the Geometry documentation
  void WorldToIndex(const mitk::Point3D& pt_mm, mitk::Point3D& pt_units) const;

  //##Documentation
  //## @brief Convert (continuous or discrete) index coordinates of a \em point to world coordinates (in mm)
  //## For further information about coordinates types, please see the Geometry documentation
  void IndexToWorld(const mitk::Point3D& pt_units, mitk::Point3D& pt_mm) const;

  //##Documentation
  //## @brief Convert world coordinates (in mm) of a \em vector
  //## \a vec_mm to (continuous!) index coordinates.
  //## @deprecated First parameter (Point3D) is not used. If possible, please use void WorldToIndex(const mitk::Vector3D& vec_mm, mitk::Vector3D& vec_units) const.
  //## For further information about coordinates types, please see the Geometry documentation
  void WorldToIndex(const mitk::Point3D& atPt3d_mm, const mitk::Vector3D& vec_mm, mitk::Vector3D& vec_units) const;

  //##Documentation
  //## @brief Convert world coordinates (in mm) of a \em vector
  //## \a vec_mm to (continuous!) index coordinates.
  //## For further information about coordinates types, please see the Geometry documentation
  void WorldToIndex(const mitk::Vector3D& vec_mm, mitk::Vector3D& vec_units) const;

  //##Documentation
  //## @brief Convert (continuous or discrete) index coordinates of a \em vector
  //## \a vec_units to world coordinates (in mm)
  //## @deprecated First parameter (Point3D) is not used. If possible, please use void IndexToWorld(const mitk::Vector3D& vec_units, mitk::Vector3D& vec_mm) const.
  //## For further information about coordinates types, please see the Geometry documentation
  void IndexToWorld(const mitk::Point3D& atPt3d_units, const mitk::Vector3D& vec_units, mitk::Vector3D& vec_mm) const;

  //##Documentation
  //## @brief Convert (continuous or discrete) index coordinates of a \em vector
  //## \a vec_units to world coordinates (in mm)
  //## For further information about coordinates types, please see the Geometry documentation
  void IndexToWorld(const mitk::Vector3D& vec_units, mitk::Vector3D& vec_mm) const;

  //##Documentation
  //## @brief Convert world coordinates (in mm) of a \em point to (discrete!) index coordinates.
  //## This method rounds to integer indices!
  //## For further information about coordinates types, please see the Geometry documentation
  template <unsigned int VIndexDimension>
  void WorldToIndex(const mitk::Point3D& pt_mm, itk::Index<VIndexDimension> &index) const
  {
    typedef itk::Index<VIndexDimension> IndexType;
    mitk::Point3D pt_units;
    this->WorldToIndex(pt_mm, pt_units);
    int i, dim=index.GetIndexDimension();
    if(dim>3)
    {
      index.Fill(0);
      dim=3;
    }
    for(i=0;i<dim;++i){
      index[i]=itk::Math::RoundHalfIntegerUp<typename IndexType::IndexValueType>( pt_units[i] );
    }
  }

  //##Documentation
  //## @brief Deprecated for use with ITK version 3.10 or newer.
  //## Convert world coordinates (in mm) of a \em point to
  //## ITK physical coordinates (in mm, but without a possible rotation)
  //##
  //## This method is useful if you have want to access an mitk::Image
  //## via an itk::Image. ITK v3.8 and older did not support rotated (tilted)
  //## images, i.e., ITK images are always parallel to the coordinate axes.
  //## When accessing a (possibly rotated) mitk::Image via an itk::Image
  //## the rotational part of the transformation in the Geometry3D is
  //## simply discarded; in other word: only the origin and spacing is
  //## used by ITK, not the complete matrix available in MITK.
  //## With WorldToItkPhysicalPoint you can convert an MITK world
  //## coordinate (including the rotation) into a coordinate that
  //## can be used with the ITK image as a ITK physical coordinate
  //## (excluding the rotation).
  template<class TCoordRep>
  void WorldToItkPhysicalPoint(const mitk::Point3D& pt_mm,
            itk::Point<TCoordRep, 3>& itkPhysicalPoint) const
  {
      mitk::vtk2itk(pt_mm, itkPhysicalPoint);
  }

  //##Documentation
  //## @brief Deprecated for use with ITK version 3.10 or newer.
  //## Convert ITK physical coordinates of a \em point (in mm,
  //## but without a rotation) into MITK world coordinates (in mm)
  //##
  //## For more information, see WorldToItkPhysicalPoint.
  template<class TCoordRep>
  void ItkPhysicalPointToWorld(const itk::Point<TCoordRep, 3>& itkPhysicalPoint,
            mitk::Point3D& pt_mm) const
  {
      mitk::vtk2itk(itkPhysicalPoint, pt_mm);
   }

  //##Documentation
  //## @brief Initialize the Geometry3D
  virtual void Initialize();

  //##Documentation
  //## @brief Is this an ImageGeometry?
  //##
  //## For more information, see SetImageGeometry
  itkGetConstMacro(ImageGeometry, bool);
  //##Documentation
  //## @brief Define that this Geometry3D is refering to an Image
  //##
  //## A geometry referring to an Image needs a slightly different
  //## definition of the position of the corners (see GetCornerPoint).
  //## The position of a voxel is defined by the position of its center.
  //## If we would use the origin (position of the (center of) the first
  //## voxel) as a corner and display this point, it would seem to be
  //## \em not at the corner but a bit within the image. Even worse for
  //## the opposite corner of the image: here the corner would appear
  //## outside the image (by half of the voxel diameter). Thus, we have
  //## to correct for this and to be able to do that, we need to know
  //## that the Geometry3D is referring to an Image.
  itkSetMacro(ImageGeometry, bool);
  itkBooleanMacro(ImageGeometry);

  //##Documentation
  //## @brief Is this Geometry3D in a state that is valid?
  virtual bool IsValid() const
  {
    return m_Valid;
  }

  //##Documentation
  //## @brief Test whether the point \a p (world coordinates in mm) is
  //## inside the bounding box
  bool IsInside(const mitk::Point3D& p) const
  {
    mitk::Point3D index;
    WorldToIndex(p, index);
    return IsIndexInside(index);
  }

  //##Documentation
  //## @brief Test whether the point \a p ((continous!)index coordinates in units) is
  //## inside the bounding box
  bool IsIndexInside(const mitk::Point3D& index) const
  {
    bool inside = false;
     //if it is an image geometry, we need to convert the index to discrete values
    //this is done by applying the rounding function also used in WorldToIndex (see line 323)
    if (m_ImageGeometry)
    {
      mitk::Point3D discretIndex;
      discretIndex[0]=itk::Math::RoundHalfIntegerUp<mitk::ScalarType>( index[0] );
      discretIndex[1]=itk::Math::RoundHalfIntegerUp<mitk::ScalarType>( index[1] );
      discretIndex[2]=itk::Math::RoundHalfIntegerUp<mitk::ScalarType>( index[2] );

      inside = m_BoundingBox->IsInside(discretIndex);
      //we have to check if the index is at the upper border of each dimension,
      // because the boundingbox is not centerbased
      if (inside)
      {
        const BoundingBox::BoundsArrayType& bounds = m_BoundingBox->GetBounds();
        if((discretIndex[0] == bounds[1]) ||
           (discretIndex[1] == bounds[3]) ||
           (discretIndex[2] == bounds[5]))
         inside = false;
      }
    }
    else
      inside = m_BoundingBox->IsInside(index);

    return inside;
  }

  //##Documentation
  //## @brief Convenience method for working with ITK indices
  template <unsigned int VIndexDimension>
    bool IsIndexInside(const itk::Index<VIndexDimension> &index) const
  {
    int i, dim=index.GetIndexDimension();
    Point3D pt_index;
    pt_index.Fill(0);
    for ( i = 0; i < dim; ++i )
    {
      pt_index[i] = index[i];
    }
    return IsIndexInside(pt_index);
  }


  //##Documentation
  //## @brief Get the spacing (size of a pixel).
  //##
  itkGetConstReferenceMacro(Spacing, mitk::Vector3D);

  //##Documentation
  //## @brief Get the spacing as a float[3] array.
  const float* GetFloatSpacing() const;

  //##Documentation
  //## @brief Set the spacing (m_Spacing)
  virtual void SetSpacing(const mitk::Vector3D& aSpacing);

  //##Documentation
  //## @brief Get the DICOM FrameOfReferenceID referring to the
  //## used world coordinate system
  itkGetConstMacro(FrameOfReferenceID, unsigned int);
  //##Documentation
  //## @brief Set the DICOM FrameOfReferenceID referring to the
  //## used world coordinate system
  itkSetMacro(FrameOfReferenceID, unsigned int);

  //##Documentation
  //## @brief Copy the ITK transform
  //## (m_IndexToWorldTransform) to the VTK transform
  //## \sa SetIndexToWorldTransform
  void TransferItkToVtkTransform();

  //##Documentation
  //## @brief Copy the VTK transform
  //## to the ITK transform (m_IndexToWorldTransform)
  //## \sa SetIndexToWorldTransform
  void TransferVtkToItkTransform();

  //##Documentation
  //## @brief Get the parametric bounding-box
  //##
  //## See AbstractTransformGeometry for an example usage of this.
  itkGetConstObjectMacro(ParametricBoundingBox, BoundingBox);
  //##Documentation
  //## @brief Get the parametric bounds
  //##
  //## See AbstractTransformGeometry for an example usage of this.
  const BoundingBox::BoundsArrayType& GetParametricBounds() const
  {
    assert(m_ParametricBoundingBox.IsNotNull());
    return m_ParametricBoundingBox->GetBounds();
  }

  //##Documentation
  //## @brief Get the parametric extent
  //##
  //## See AbstractTransformGeometry for an example usage of this.
  mitk::ScalarType GetParametricExtent(int direction) const
  {
    assert(direction>=0 && direction<3);
    assert(m_ParametricBoundingBox.IsNotNull());

    BoundingBoxType::BoundsArrayType bounds = m_ParametricBoundingBox->GetBounds();
    return bounds[direction*2+1]-bounds[direction*2];
  }

  //##Documentation
  //## @brief Get the parametric extent in mm
  //##
  //## See AbstractTransformGeometry for an example usage of this.
  virtual mitk::ScalarType GetParametricExtentInMM(int direction) const
  {
    return GetExtentInMM(direction);
  }

  //##Documentation
  //## @brief Get the parametric transform
  //##
  //## See AbstractTransformGeometry for an example usage of this.
  virtual const Transform3D* GetParametricTransform() const
  {
    return m_IndexToWorldTransform;
  }

  //##Documentation
  //## @brief Calculates a bounding-box around the geometry relative
  //## to a coordinate system defined by a transform
  //##
  mitk::BoundingBox::Pointer CalculateBoundingBoxRelativeToTransform(const mitk::AffineTransform3D* transform) const;

  //##Documentation
  //## @brief clones the geometry
  //##
  //## Overwrite in all sub-classes.
  //## Normally looks like:
  //## \code
  //##  Self::Pointer newGeometry = new Self(*this);
  //##  newGeometry->UnRegister();
  //##  return newGeometry.GetPointer();
  //## \endcode
  virtual itk::LightObject::Pointer InternalClone() const;

  //##Documentation
  //##@brief executes affine operations (translate, rotate, scale)
  virtual void ExecuteOperation(Operation* operation);

protected:
  Geometry3D();
  Geometry3D(const Geometry3D& other);

  static const std::string GetTransformAsString( TransformType* transformType );

  virtual ~Geometry3D();

  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

  virtual void BackTransform(const mitk::Point3D& in, mitk::Point3D& out) const;
  //##Documentation
  //## @brief Deprecated
  virtual void BackTransform(const mitk::Point3D& at, const mitk::Vector3D& in, mitk::Vector3D& out) const;

  //Without redundant parameter Point3D
  virtual void BackTransform(const mitk::Vector3D& in, mitk::Vector3D& out) const;

  //##Documentation
  //## @brief Set the parametric bounds
  //##
  //## Protected in this class, made public in some sub-classes, e.g.,
  //## ExternAbstractTransformGeometry.
  virtual void SetParametricBounds(const BoundingBox::BoundsArrayType& bounds);

  /** Resets sub-transforms that compose m_IndexToWorldTransform, by using
   * the current value of m_IndexToWorldTransform and setting the rotation
   * component to zero. */
  virtual void ResetSubTransforms();

  mutable mitk::BoundingBox::Pointer m_ParametricBoundingBox;

  mutable mitk::TimeBounds m_TimeBounds;

  vtkMatrix4x4* m_VtkMatrix;

  bool m_ImageGeometry;

  //##Documentation
  //## @brief Spacing of the data. Only significant if the geometry describes
  //## an Image (m_ImageGeometry==true).
  mitk::Vector3D m_Spacing;

  bool m_Valid;

  unsigned int m_FrameOfReferenceID;

  static const std::string INDEX_TO_OBJECT_TRANSFORM;
  static const std::string OBJECT_TO_NODE_TRANSFORM;
  static const std::string INDEX_TO_NODE_TRANSFORM;
  static const std::string INDEX_TO_WORLD_TRANSFORM;

private:
  mutable TransformType::Pointer m_InvertedTransform;
  mutable unsigned long m_IndexToWorldTransformLastModified;

  VnlQuaternionType m_RotationQuaternion;


  float m_FloatSpacing[3];
  vtkMatrixToLinearTransform* m_VtkIndexToWorldTransform;

  //##Documentation
  //## @brief Origin, i.e. upper-left corner of the plane
  //##
  Point3D m_Origin;
};

} // namespace mitk

#endif /* GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD */
