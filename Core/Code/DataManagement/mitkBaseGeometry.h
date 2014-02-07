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

#ifndef BaseGeometry_H_HEADER_INCLUDED
#define BaseGeometry_H_HEADER_INCLUDED

#include <MitkExports.h>
#include <mitkCommon.h>
#include "mitkoperationactor.h"

#include <itkBoundingBox.h>
#include "mitkvector.h"
#include <itkAffineGeometryFrame.h>
#include <itkQuaternionRigidTransform.h>
#include "itkScalableAffineTransform.h"
#include <itkIndex.h>

class vtkMatrix4x4;
class vtkMatrixToLinearTransform;
class vtkLinearTransform;

namespace mitk {
  //##Documentation
  //## @brief Standard 3D-BoundingBox typedef
  //##
  //## Standard 3D-BoundingBox typedef to get rid of template arguments (3D, type).
  typedef itk::BoundingBox<unsigned long, 3, ScalarType> BoundingBox;

  //##Documentation
  //## @brief Standard typedef for time-bounds
  typedef itk::FixedArray<ScalarType,2>  TimeBounds;
  typedef itk::FixedArray<ScalarType, 3> FixedArrayType;

  //##Documentation
  //## @brief BaseGeometry xxxxxxxxxxxxxx
  //##
  //## xxxxxxxxxxx
  //##
  //## Rule: everything is in mm (ms) if not stated otherwise.
  //## @ingroup Geometry
  class MITK_CORE_EXPORT BaseGeometry : public itk::Object, public OperationActor
  {
  public:
    mitkClassMacro(BaseGeometry, itk::Object);

    // ********************************** TypeDef **********************************

    typedef itk::QuaternionRigidTransform< ScalarType > QuaternionTransformType;
    typedef QuaternionTransformType::VnlQuaternionType VnlQuaternionType;

    typedef itk::ScalableAffineTransform<ScalarType, 3>    TransformType;
    typedef itk::BoundingBox<unsigned long, 3, ScalarType> BoundingBoxType;
    typedef BoundingBoxType::BoundsArrayType               BoundsArrayType;
    typedef BoundingBoxType::Pointer                       BoundingBoxPointer;

    // ********************************** Origin, Spacing **********************************

    //##Documentation
    //## @brief Get the origin, e.g. the upper-left corner of the plane
    const Point3D& GetOrigin() const;

    //##Documentation
    //## @brief Set the origin, i.e. the upper-left corner of the plane
    //##
    virtual void SetOrigin(const Point3D& origin);

    //##Documentation
    //## @brief Get the origin as VnlVector
    //##
    //## \sa GetOrigin
    // xx This was not vitrual!
    virtual VnlVector GetOriginVnl() const = 0 ; //ToDo

    //##Documentation
    //## @brief Get the spacing (size of a pixel).
    //##
    itkGetConstReferenceMacro(Spacing, mitk::Vector3D);

    //##Documentation
    //## @brief Get the spacing as a float[3] array.
    const float* GetFloatSpacing() const;

    //##Documentation
    //## @brief Set the spacing (m_Spacing)
    virtual void SetSpacing(const mitk::Vector3D& aSpacing) = 0 ; //ToDo

    // ********************************** other functions **********************************

    //##Documentation
    //## @brief Is this Geometry3D in a state that is valid?
    virtual bool IsValid() const = 0 ; //ToDo

    //##Documentation
    //## @brief Get the DICOM FrameOfReferenceID referring to the
    //## used world coordinate system
    itkGetConstMacro(FrameOfReferenceID, unsigned int);
    //##Documentation
    //## @brief Set the DICOM FrameOfReferenceID referring to the
    //## used world coordinate system
    itkSetMacro(FrameOfReferenceID, unsigned int);

    // ********************************** Functions Initialize **********************************

    //##Documentation
    //## @brief Initialize the Geometry3D
    virtual void Initialize();

    virtual void InitializeGeometry(Self * newGeometry) const;

    static void CopySpacingFromTransform(mitk::AffineTransform3D* transform, mitk::Vector3D& spacing, float floatSpacing[3]);

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
    virtual itk::LightObject::Pointer InternalClone() const = 0 ;  //ToDo

    virtual void PrintSelf(std::ostream& os, itk::Indent indent) const = 0 ; //ToDo: This was protected!

    // ********************************** Transformations Set/Get **********************************

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
    virtual void SetIndexToWorldTransformByVtkMatrix(vtkMatrix4x4* vtkmatrix) = 0 ; //ToDo

    /** Set/Get the IndexToWorldTransform */
    itkGetConstObjectMacro(IndexToWorldTransform, AffineTransform3D);
    itkGetObjectMacro(IndexToWorldTransform, AffineTransform3D);

    //##Documentation
    //## @brief Get the m_IndexToWorldTransform as a vtkLinearTransform
    // xx This was not vitrual!
    virtual     vtkLinearTransform* GetVtkTransform() const = 0 ; //ToDo

    //##Documentation
    //## @brief Set the transform to identity and origin to 0
    //##
    virtual void SetIdentity() = 0 ; //ToDo

    //##Documentation
    //## @brief Get the parametric transform
    //##
    //## See AbstractTransformGeometry for an example usage of this.
    virtual const Transform3D* GetParametricTransform() const = 0 ; //ToDo

    // ********************************** Transformations **********************************

    //##Documentation
    //## @brief Copy the ITK transform
    //## (m_IndexToWorldTransform) to the VTK transform
    //## \sa SetIndexToWorldTransform
    virtual void TransferItkToVtkTransform();

    //##Documentation
    //## @brief Copy the VTK transform
    //## to the ITK transform (m_IndexToWorldTransform)
    //## \sa SetIndexToWorldTransform
    // xx This was not vitrual!
    virtual     void TransferVtkToItkTransform() = 0 ; //ToDo

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
    virtual void Compose( const BaseGeometry::TransformType * other, bool pre = 0 ) = 0 ; //ToDo

    //##Documentation
    //## @brief Compose new IndexToWorldTransform with a given vtkMatrix4x4.
    //##
    //## Converts the vtkMatrix4x4 into a itk-transform and calls the previous method.
    virtual void Compose( const vtkMatrix4x4 * vtkmatrix, bool pre = 0 ) = 0 ; //ToDo

    //##Documentation
    //## @brief Translate the origin by a vector
    //##
    virtual void Translate(const Vector3D&  vector) = 0 ; //ToDo

    //##Documentation
    //## @brief Convert world coordinates (in mm) of a \em point to (continuous!) index coordinates
    //## \warning If you need (discrete) integer index coordinates (e.g., for iterating easily over an image),
    //## use WorldToIndex(const mitk::Point3D& pt_mm, itk::Index<VIndexDimension> &index).
    //## For further information about coordinates types, please see the Geometry documentation
    // xx This was not vitrual!
    virtual void WorldToIndex(const mitk::Point3D& pt_mm, mitk::Point3D& pt_units) const = 0 ; //ToDo

    //##Documentation
    //## @brief Convert (continuous or discrete) index coordinates of a \em point to world coordinates (in mm)
    //## For further information about coordinates types, please see the Geometry documentation
    // xx This was not vitrual!
    virtual void IndexToWorld(const mitk::Point3D& pt_units, mitk::Point3D& pt_mm) const = 0 ; //ToDo

    //##Documentation
    //## @brief Convert (discrete) index coordinates of a \em point to world coordinates (in mm)
    //## For further information about coordinates types, please see the Geometry documentation
    template <unsigned int VIndexDimension>
    void IndexToWorld(const itk::Index<VIndexDimension> &index, mitk::Point3D& pt_mm ) const = 0 ; //ToDo

    //##Documentation
    //## @brief Convert world coordinates (in mm) of a \em vector
    //## \a vec_mm to (continuous!) index coordinates.
    //## For further information about coordinates types, please see the Geometry documentation
    // xx This was not vitrual!
    virtual void WorldToIndex(const mitk::Vector3D& vec_mm, mitk::Vector3D& vec_units) const = 0 ; //ToDo

    //##Documentation
    //## @brief Convert (continuous or discrete) index coordinates of a \em vector
    //## \a vec_units to world coordinates (in mm)
    //## For further information about coordinates types, please see the Geometry documentation
    // xx This was not vitrual!
    virtual void IndexToWorld(const mitk::Vector3D& vec_units, mitk::Vector3D& vec_mm) const = 0 ; //ToDo

    //##Documentation
    //## @brief Convert world coordinates (in mm) of a \em point to (discrete!) index coordinates.
    //## This method rounds to integer indices!
    //## For further information about coordinates types, please see the Geometry documentation
    template <unsigned int VIndexDimension>
    void WorldToIndex(const mitk::Point3D& pt_mm, itk::Index<VIndexDimension> &index) const = 0 ; //ToDo

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
      itk::Point<TCoordRep, 3>& itkPhysicalPoint) const = 0 ; //ToDo

    //##Documentation
    //## @brief Deprecated for use with ITK version 3.10 or newer.
    //## Convert ITK physical coordinates of a \em point (in mm,
    //## but without a rotation) into MITK world coordinates (in mm)
    //##
    //## For more information, see WorldToItkPhysicalPoint.
    template<class TCoordRep>
    void ItkPhysicalPointToWorld(const itk::Point<TCoordRep, 3>& itkPhysicalPoint,
      mitk::Point3D& pt_mm) const = 0 ; //ToDo

    //Without redundant parameter Point3D
    virtual void BackTransform(const mitk::Vector3D& in, mitk::Vector3D& out) const = 0 ;  //This was protected! //ToDo
    virtual void BackTransform(const mitk::Point3D& in, mitk::Point3D& out) const = 0 ;//This was protected! //ToDo

    /** Resets sub-transforms that compose m_IndexToWorldTransform, by using
    * the current value of m_IndexToWorldTransform and setting the rotation
    * component to zero. */
    virtual void ResetSubTransforms() = 0 ;  //This was protected! Empty Function! //ToDo

    //##Documentation
    //##@brief executes affine operations (translate, rotate, scale)
    virtual void ExecuteOperation(Operation* operation) = 0 ; //ToDo

    //xxxx   static const std::string GetTransformAsString( TransformType* transformType );//This was protected! //ToDo

    // ********************************** BoundingBox **********************************

    //##Documentation
    //## @brief Get the parametric bounding-box
    //##
    //## See AbstractTransformGeometry for an example usage of this.
    itkGetConstObjectMacro(ParametricBoundingBox, BoundingBox);
    //##Documentation
    //## @brief Get the parametric bounds
    //##
    //## See AbstractTransformGeometry for an example usage of this.
    // xx This was not vitrual!
    virtual const BoundingBox::BoundsArrayType& GetParametricBounds() const = 0 ; //ToDo

    //##Documentation
    //## @brief Set the parametric bounds
    //##
    //## Protected in this class, made public in some sub-classes, e.g.,
    //## ExternAbstractTransformGeometry.
    virtual void SetParametricBounds(const BoundingBox::BoundsArrayType& bounds) = 0; //This was protected!   //ToDo

    //##Documentation
    //## @brief Calculates a bounding-box around the geometry relative
    //## to a coordinate system defined by a transform
    //##
    // xx This was not vitrual!
    virtual  mitk::BoundingBox::Pointer CalculateBoundingBoxRelativeToTransform(const mitk::AffineTransform3D* transform) const = 0 ; //ToDo

    /** Get the bounding box */
    itkGetConstObjectMacro(BoundingBox, BoundingBoxType);

    //##Documentation
    //## @brief Get a VnlVector along bounding-box in the specified
    //## @a direction, length is spacing
    //##
    //## \sa GetAxisVector
    // xx This was not vitrual!
    virtual VnlVector GetMatrixColumn(unsigned int direction) const = 0 ; //ToDo

    //##Documentation
    //## @brief Get the time bounds (in ms)
    itkGetConstReferenceMacro(TimeBounds, TimeBounds);

    const BoundsArrayType GetBounds() const
    {
      assert(m_BoundingBox.IsNotNull());
      return m_BoundingBox->GetBounds();
    }

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
#endif

    //##Documentation
    //## \brief Set the bounding box (in index/unit coordinates)
    //##
    //## Only possible via the BoundsArray to make clear that a
    //## copy of the bounding-box is stored, not a reference to it.
    virtual void SetBounds(const BoundsArrayType& bounds);

    //##Documentation
    //## @brief Set the bounding box (in index/unit coordinates) via a float array
    virtual void SetFloatBounds(const float bounds[6]);
    //##Documentation
    //## @brief Set the bounding box (in index/unit coordinates) via a double array
    virtual void SetFloatBounds(const double bounds[6]);

    //##Documentation
    //## @brief Set the time bounds (in ms)
    virtual void SetTimeBounds(const TimeBounds& timebounds) = 0 ; //ToDo

    // ********************************** Geometry **********************************

    //##Documentation
    //## @brief Checks, if the given geometry can be converted to 2D without information loss
    //## e.g. when a 2D image is saved, the matrix is usually cropped to 2x2, and when you load it back to MITK
    //## it will be filled with standard values. This function checks, if information would be lost during this
    //## procedure
    virtual bool Is2DConvertable() = 0 ; //ToDo

    //##Documentation
    //## @brief Get the position of the corner number \a id (in world coordinates)
    //##
    //## See SetImageGeometry for how a corner is defined on images.
    // xx This was not vitrual!
    virtual Point3D GetCornerPoint(int id) const = 0 ; //ToDo

    //##Documentation
    //## @brief Get the position of a corner (in world coordinates)
    //##
    //## See SetImageGeometry for how a corner is defined on images.
    // xx This was not vitrual!
    virtual Point3D GetCornerPoint(bool xFront=true, bool yFront=true, bool zFront=true) const = 0 ; //ToDo

    //##Documentation
    //## @brief Get vector along bounding-box in the specified @a direction in mm
    //##
    //## The length of the vector is the size of the bounding-box in the
    //## specified @a direction in mm
    //## \sa GetMatrixColumn
    // xx This was not vitrual!
    virtual Vector3D GetAxisVector(unsigned int direction) const = 0 ; //ToDo

    //##Documentation
    //## @brief Get the center of the bounding-box in mm
    //##
    // xx This was not vitrual!
    virtual Point3D GetCenter() const = 0 ; //ToDo

    //##Documentation
    //## @brief Get the squared length of the diagonal of the bounding-box in mm
    //##
    // xx This was not vitrual!
    virtual double GetDiagonalLength2() const = 0 ; //ToDo

    //##Documentation
    //## @brief Get the length of the diagonal of the bounding-box in mm
    //##
    // xx This was not vitrual!
    virtual double GetDiagonalLength() const = 0 ; //ToDo

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
    // xx This was not vitrual!
    virtual ScalarType GetExtentInMM(int direction) const = 0 ; //ToDo

    //##Documentation
    //## @brief Set the extent of the bounding-box in the specified @a direction in mm
    //##
    //## @note This changes the matrix in the transform, @a not the bounds, which are given in units!
    virtual void SetExtentInMM(int direction, ScalarType extentInMM) = 0 ; //ToDo

    //##Documentation
    //## @brief Test whether the point \a p (world coordinates in mm) is
    //## inside the bounding box
    // xx This was not vitrual!
    virtual  bool IsInside(const mitk::Point3D& p) const = 0 ; //ToDo

    //##Documentation
    //## @brief Test whether the point \a p ((continous!)index coordinates in units) is
    //## inside the bounding box
    // xx This was not vitrual!
    virtual bool IsIndexInside(const mitk::Point3D& index) const = 0 ; //ToDo

    //##Documentation
    //## @brief Get the parametric extent
    //##
    //## See AbstractTransformGeometry for an example usage of this.
    // xx This was not vitrual!
    virtual  mitk::ScalarType GetParametricExtent(int direction) const = 0 ; //ToDo

    //##Documentation
    //## @brief Get the parametric extent in mm
    //##
    //## See AbstractTransformGeometry for an example usage of this.
    virtual mitk::ScalarType GetParametricExtentInMM(int direction) const = 0 ; //ToDo

  protected:

    // ********************************** Constructor **********************************
    BaseGeometry();
    BaseGeometry(const BaseGeometry& other);
    virtual ~BaseGeometry();

    // ********************************** Variables **********************************

    AffineTransform3D::Pointer m_IndexToWorldTransform;

    vtkMatrixToLinearTransform* m_VtkIndexToWorldTransform;

    vtkMatrix4x4* m_VtkMatrix;

    bool m_Valid;

    unsigned int m_FrameOfReferenceID;

    mutable mitk::BoundingBox::Pointer m_ParametricBoundingBox;

    mutable mitk::TimeBounds m_TimeBounds;

    mutable BoundingBoxPointer m_BoundingBox;

    //##Documentation
    //## @brief Origin, i.e. upper-left corner of the plane
    //##
    Point3D m_Origin;

    //##Documentation
    //## @brief Spacing of the data. Only significant if the geometry describes
    //## an Image (m_ImageGeometry==true).
    mitk::Vector3D m_Spacing;

    static const unsigned int NDimensions = 3;

    mutable TransformType::Pointer m_InvertedTransform; //this was private

    mutable unsigned long m_IndexToWorldTransformLastModified;  //this was private

    float m_FloatSpacing[3]; //this was private

    VnlQuaternionType m_RotationQuaternion; //this was private
  };
} // namespace mitk

// ********************************** Equal Functions **********************************

//
// Static compare functions mainly for testing
//

/**
* @brief Equal A function comparing two bounding boxes (BoundingBoxType) for beeing identical.
*
* @ingroup MITKTestingAPI
*
* The function compares the bounds (elementwise).
*
* The parameter eps is a tolarence value for all methods which are internally used for comparion.
* @param rightHandSide Compare this against leftHandSide.
* @param leftHandSide Compare this against rightHandSide.
* @param eps Tolarence for comparison. You can use mitk::eps in most cases.
* @param verbose Flag indicating if the user wants detailed console output or not.
* @return True, if all comparison are true. False in any other case.
*/
MITK_CORE_EXPORT bool Equal( const  mitk::BaseGeometry::BoundingBoxType *leftHandSide, const mitk::BaseGeometry::BoundingBoxType *rightHandSide, mitk::ScalarType eps, bool verbose); //ToDo

//
// Static compare functions mainly for testing
//
/**
* @brief Equal A function comparing two geometries for beeing identical.
*
* @ingroup MITKTestingAPI
*
* The function compares the spacing, origin, axisvectors, extents, the matrix of the
* IndexToWorldTransform (elementwise), the bounding (elementwise) and the ImageGeometry flag.
*
* The parameter eps is a tolarence value for all methods which are internally used for comparion.
* If you want to use different tolarance values for different parts of the geometry, feel free to use
* the other comparison methods and write your own implementation of Equal.
* @param rightHandSide Compare this against leftHandSide.
* @param leftHandSide Compare this against rightHandSide.
* @param eps Tolarence for comparison. You can use mitk::eps in most cases.
* @param verbose Flag indicating if the user wants detailed console output or not.
* @return True, if all comparison are true. False in any other case.
*/
MITK_CORE_EXPORT bool Equal(const mitk::BaseGeometry* leftHandSide, const mitk::BaseGeometry* rightHandSide, mitk::ScalarType eps, bool verbose); //ToDo

/**
* @brief Equal A function comparing two transforms (TransformType) for beeing identical.
*
* @ingroup MITKTestingAPI
*
* The function compares the IndexToWorldTransform (elementwise).
*
* The parameter eps is a tolarence value for all methods which are internally used for comparion.
* @param rightHandSide Compare this against leftHandSide.
* @param leftHandSide Compare this against rightHandSide.
* @param eps Tolarence for comparison. You can use mitk::eps in most cases.
* @param verbose Flag indicating if the user wants detailed console output or not.
* @return True, if all comparison are true. False in any other case.
*/
MITK_CORE_EXPORT bool Equal(const mitk::BaseGeometry::TransformType *leftHandSide, const mitk::BaseGeometry::TransformType *rightHandSide, mitk::ScalarType eps, bool verbose); //ToDo

#endif /* BaseGeometry_H_HEADER_INCLUDED */
