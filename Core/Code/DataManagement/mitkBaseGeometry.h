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

#include <MitkCoreExports.h>
#include <mitkCommon.h>
#include "mitkOperationActor.h"

#include <itkBoundingBox.h>
#include "mitkNumericTypes.h"
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

  typedef itk::AffineGeometryFrame<ScalarType, 3>        AffineGeometryFrame3D;

  //##Documentation
  //## @brief BaseGeometry Describes the geometry of a data object
  //##
  //## The class holds
  //## \li a bounding box which is axes-parallel in intrinsic coordinates
  //## (often integer indices of pixels), to be accessed by
  //## GetBoundingBox()
  //## \li a transform to convert intrinsic coordinates into a
  //## world-coordinate system with coordinates in millimeters
  //## and milliseconds (all are floating point values), to
  //## be accessed by GetIndexToWorldTransform()
  //## \li an origin and spacing to define the geometry
  //##
  //## BaseGeometry and its sub-classes allow converting between
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
  //## BaseGeometry instances referring to an Image need a slightly
  //## different definition of corners, see SetImageGeometry. This
  //## is usualy automatically called by Image.
  //##
  //## BaseGeometry have to be initialized in the method GenerateOutputInformation()
  //## of BaseProcess (or CopyInformation/ UpdateOutputInformation of BaseData,
  //## if possible, e.g., by analyzing pic tags in Image) subclasses. See also
  //## itk::ProcessObject::GenerateOutputInformation(),
  //## itk::DataObject::CopyInformation() and
  //## itk::DataObject::UpdateOutputInformation().
  //##
  //## At least, it can return the bounding box of the data object.
  //##
  //## The BaseGeometry class is an abstract class. The most simple implementation
  //## is the sublass Geometry3D.
  //##
  //## Rule: everything is in mm (ms) if not stated otherwise.
  //## @ingroup Geometry
  class MITK_CORE_EXPORT BaseGeometry : public itk::Object, public OperationActor
  {
  public:
    mitkClassMacro(BaseGeometry, itk::Object);
    itkCloneMacro(Self)

    // ********************************** TypeDef **********************************

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
    void SetOrigin(const Point3D& origin);

    //##Documentation
    //## @brief Get the spacing (size of a pixel).
    //##
    itkGetConstReferenceMacro(Spacing, mitk::Vector3D);

    //##Documentation
    //## @brief Set the spacing (m_Spacing).
    //##
    //##The spacing is also changed in the IndexToWorldTransform.
    void SetSpacing(const mitk::Vector3D& aSpacing, bool enforceSetSpacing = false);

    //##Documentation
    //## @brief Get the origin as VnlVector
    //##
    //## \sa GetOrigin
    VnlVector GetOriginVnl() const;

    // ********************************** other functions **********************************

    //##Documentation
    //## @brief Get the DICOM FrameOfReferenceID referring to the
    //## used world coordinate system
    itkGetConstMacro(FrameOfReferenceID, unsigned int);
    //##Documentation
    //## @brief Set the DICOM FrameOfReferenceID referring to the
    //## used world coordinate system
    itkSetMacro(FrameOfReferenceID, unsigned int);

    itkGetConstMacro(IndexToWorldTransformLastModified, unsigned long);

    //##Documentation
    //## @brief Overload of function Modified() to prohibit several calls of Modified() using the ModifiedLock class.
    //##
    //## For the use of Modified(), see class ModifiedLock.
    void Modified() const;

    friend class ModifiedLock;

    //##Documentation
    //## @brief Is this BaseGeometry in a state that is valid?
    //##
    //## This function returns always true in the BaseGeometry class. Other implementations are possible in subclasses.
    virtual bool IsValid() const;

    // ********************************** Initialize **********************************

    //##Documentation
    //## @brief Initialize the BaseGeometry
    void Initialize();

    void InitializeGeometry(Self * newGeometry) const;

    // ********************************** Transformations Set/Get **********************************

    // a bit of a misuse, but we want only doxygen to see the following:
#ifdef DOXYGEN_SKIP
    //##Documentation
    //## @brief Get the transformation used to convert from index
    //## to world coordinates
    itkGetObjectMacro(IndexToWorldTransform, AffineTransform3D);
#endif
    //## @brief Set the transformation used to convert from index
    //## to world coordinates. The spacing of the new transform is
    //## copied to m_spacing.
    void SetIndexToWorldTransform(mitk::AffineTransform3D* transform);

    //##Documentation
    //## @brief Convenience method for setting the ITK transform
    //## (m_IndexToWorldTransform) via an vtkMatrix4x4.The spacing of
    //## the new transform is copied to m_spacing.
    //## \sa SetIndexToWorldTransform
    virtual void SetIndexToWorldTransformByVtkMatrix(vtkMatrix4x4* vtkmatrix);

    //## Get the IndexToWorldTransform
    itkGetConstObjectMacro(IndexToWorldTransform, AffineTransform3D);
    itkGetObjectMacro(IndexToWorldTransform, AffineTransform3D);

    //## Get the Vtk Matrix which describes the transform.
    vtkMatrix4x4* GetVtkMatrix();

    //##Documentation
    //## @brief Get the m_IndexToWorldTransform as a vtkLinearTransform
    vtkLinearTransform* GetVtkTransform() const;

    //##Documentation
    //## @brief Set the transform to identity, the spacing to 1 and origin to 0
    //##
    virtual void SetIdentity();

    // ********************************** Transformations **********************************

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
    //## This method also changes m_spacing.
    void Compose( const BaseGeometry::TransformType * other, bool pre = 0 );

    //##Documentation
    //## @brief Compose new IndexToWorldTransform with a given vtkMatrix4x4.
    //##
    //## Converts the vtkMatrix4x4 into a itk-transform and calls the previous method.
    void Compose( const vtkMatrix4x4 * vtkmatrix, bool pre = 0 );

    //##Documentation
    //## @brief Translate the origin by a vector
    //##
    void Translate(const Vector3D&  vector);

    //##Documentation
    //##@brief executes affine operations (translate, rotate, scale)
    virtual void ExecuteOperation(Operation* operation);

    //##Documentation
    //## @brief Convert world coordinates (in mm) of a \em point to (continuous!) index coordinates
    //## \warning If you need (discrete) integer index coordinates (e.g., for iterating easily over an image),
    //## use WorldToIndex(const mitk::Point3D& pt_mm, itk::Index<VIndexDimension> &index).
    //## For further information about coordinates types, please see the Geometry documentation
    void WorldToIndex(const mitk::Point3D& pt_mm, mitk::Point3D& pt_units) const;

    //##Documentation
    //## @brief Convert world coordinates (in mm) of a \em vector
    //## \a vec_mm to (continuous!) index coordinates.
    //## For further information about coordinates types, please see the Geometry documentation
    void WorldToIndex(const mitk::Vector3D& vec_mm, mitk::Vector3D& vec_units) const;

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
    //## @brief Convert (continuous or discrete) index coordinates of a \em vector
    //## \a vec_units to world coordinates (in mm)
    //## For further information about coordinates types, please see the Geometry documentation
    void IndexToWorld(const mitk::Vector3D& vec_units, mitk::Vector3D& vec_mm) const;

    //##Documentation
    //## @brief Convert (continuous or discrete) index coordinates of a \em point to world coordinates (in mm)
    //## For further information about coordinates types, please see the Geometry documentation
    void IndexToWorld(const mitk::Point3D& pt_units, mitk::Point3D& pt_mm) const;

    //##Documentation
    //## @brief Convert (discrete) index coordinates of a \em point to world coordinates (in mm)
    //## For further information about coordinates types, please see the Geometry documentation
    template <unsigned int VIndexDimension>
    void IndexToWorld(const itk::Index<VIndexDimension> &index, mitk::Point3D& pt_mm ) const
    {
      mitk::Point3D pt_units;
      pt_units.Fill(0);
      int i, dim=index.GetIndexDimension();
      if(dim>3)
      {
        dim=3;
      }
      for(i=0;i<dim;++i)
      {
        pt_units[i] = index[i];
      }

      IndexToWorld(pt_units,pt_mm);
    }

    //##Documentation
    //## @brief Convert (continuous or discrete) index coordinates of a \em vector
    //## \a vec_units to world coordinates (in mm)
    //## @deprecated First parameter (Point3D) is not used. If possible, please use void IndexToWorld(const mitk::Vector3D& vec_units, mitk::Vector3D& vec_mm) const.
    //## For further information about coordinates types, please see the Geometry documentation
    void IndexToWorld(const mitk::Point3D& atPt3d_units, const mitk::Vector3D& vec_units, mitk::Vector3D& vec_mm) const;

    //##Documentation
    //## @brief Convert world coordinates (in mm) of a \em vector
    //## \a vec_mm to (continuous!) index coordinates.
    //## @deprecated First parameter (Point3D) is not used. If possible, please use void WorldToIndex(const mitk::Vector3D& vec_mm, mitk::Vector3D& vec_units) const.
    //## For further information about coordinates types, please see the Geometry documentation
    void WorldToIndex(const mitk::Point3D& atPt3d_mm, const mitk::Vector3D& vec_mm, mitk::Vector3D& vec_units) const;

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
    //## @brief Deprecated for use with ITK version 3.10 or newer.
    //## Convert world coordinates (in mm) of a \em point to
    //## ITK physical coordinates (in mm, but without a possible rotation)
    //##
    //## This method is useful if you have want to access an mitk::Image
    //## via an itk::Image. ITK v3.8 and older did not support rotated (tilted)
    //## images, i.e., ITK images are always parallel to the coordinate axes.
    //## When accessing a (possibly rotated) mitk::Image via an itk::Image
    //## the rotational part of the transformation in the BaseGeometry is
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

    // ********************************** BoundingBox **********************************

    /** Get the bounding box */
    itkGetConstObjectMacro(BoundingBox, BoundingBoxType);

    //##Documentation
    //## @brief Get the time bounds (in ms)
    //itkGetConstReferenceMacro(TimeBounds, TimeBounds);

    // a bit of a misuse, but we want only doxygen to see the following:
#ifdef DOXYGEN_SKIP
    //##Documentation
    //## @brief Get bounding box (in index/unit coordinates)
    itkGetConstObjectMacro(BoundingBox, BoundingBoxType);
    //##Documentation
    //## @brief Get bounding box (in index/unit coordinates) as a BoundsArrayType
    const BoundsArrayType GetBounds() const;
#endif
    const BoundsArrayType GetBounds() const;

    //##Documentation
    //## \brief Set the bounding box (in index/unit coordinates)
    //##
    //## Only possible via the BoundsArray to make clear that a
    //## copy of the bounding-box is stored, not a reference to it.
    void SetBounds(const BoundsArrayType& bounds);

    //##Documentation
    //## @brief Set the bounding box (in index/unit coordinates) via a float array
    void SetFloatBounds(const float bounds[6]);
    //##Documentation
    //## @brief Set the bounding box (in index/unit coordinates) via a double array
    void SetFloatBounds(const double bounds[6]);

    //##Documentation
    //## @brief Get a VnlVector along bounding-box in the specified
    //## @a direction, length is spacing
    //##
    //## \sa GetAxisVector
    VnlVector GetMatrixColumn(unsigned int direction) const;

    //##Documentation
    //## @brief Calculates a bounding-box around the geometry relative
    //## to a coordinate system defined by a transform
    //##
    mitk::BoundingBox::Pointer CalculateBoundingBoxRelativeToTransform(const mitk::AffineTransform3D* transform) const;

    //##Documentation
    //## @brief Set the time bounds (in ms)
    //void SetTimeBounds(const TimeBounds& timebounds);

    // ********************************** Geometry **********************************

#ifdef DOXYGEN_SKIP
    //##Documentation
    //## @brief Get the extent of the bounding box (in index/unit coordinates)
    //##
    //## To access the extent in mm use GetExtentInMM
    ScalarType GetExtent(unsigned int direction) const;
#endif

    /** Get the extent of the bounding box */
    ScalarType GetExtent(unsigned int direction) const;

    //##Documentation
    //## @brief Get the extent of the bounding-box in the specified @a direction in mm
    //##
    //## Equals length of GetAxisVector(direction).
    ScalarType GetExtentInMM(int direction) const;

    //##Documentation
    //## @brief Get vector along bounding-box in the specified @a direction in mm
    //##
    //## The length of the vector is the size of the bounding-box in the
    //## specified @a direction in mm
    //## \sa GetMatrixColumn
    Vector3D GetAxisVector(unsigned int direction) const;

    //##Documentation
    //## @brief Checks, if the given geometry can be converted to 2D without information loss
    //## e.g. when a 2D image is saved, the matrix is usually cropped to 2x2, and when you load it back to MITK
    //## it will be filled with standard values. This function checks, if information would be lost during this
    //## procedure
    virtual bool Is2DConvertable();

    //##Documentation
    //## @brief Get the center of the bounding-box in mm
    //##
    Point3D GetCenter() const;

    //##Documentation
    //## @brief Get the squared length of the diagonal of the bounding-box in mm
    //##
    double GetDiagonalLength2() const;

    //##Documentation
    //## @brief Get the length of the diagonal of the bounding-box in mm
    //##
    double GetDiagonalLength() const;

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
    //## @brief Set the extent of the bounding-box in the specified @a direction in mm
    //##
    //## @note This changes the matrix in the transform, @a not the bounds, which are given in units!
    void SetExtentInMM(int direction, ScalarType extentInMM);

    //##Documentation
    //## @brief Test whether the point \a p (world coordinates in mm) is
    //## inside the bounding box
    bool IsInside(const mitk::Point3D& p) const;

    //##Documentation
    //## @brief Test whether the point \a p ((continous!)index coordinates in units) is
    //## inside the bounding box
    bool IsIndexInside(const mitk::Point3D& index) const;

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

    // ********************************* Image Geometry ********************************
    //##Documentation
    //## @brief When switching from an Image Geometry to a normal Geometry (and the other way around), you have to change the origin as well (See Geometry Documentation)! This function will change the "isImageGeometry" bool flag and changes the origin respectively.
    virtual void ChangeImageGeometryConsideringOriginOffset( const bool isAnImageGeometry );

    //##Documentation
    //## @brief Is this an ImageGeometry?
    //##
    //## For more information, see SetImageGeometry
    itkGetConstMacro(ImageGeometry, bool);
    //##Documentation
    //## @brief Define that this BaseGeometry is refering to an Image
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
    //## that the BaseGeometry is referring to an Image.
    itkSetMacro(ImageGeometry, bool);
    itkBooleanMacro(ImageGeometry);

  protected:

    // ********************************** Constructor **********************************
    BaseGeometry();
    BaseGeometry(const BaseGeometry& other);
    virtual ~BaseGeometry();

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
    virtual itk::LightObject::Pointer InternalClone() const =0;

    virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

    void BackTransform(const mitk::Point3D& in, mitk::Point3D& out) const;

    //Without redundant parameter Point3D
    void BackTransform(const mitk::Vector3D& in, mitk::Vector3D& out) const;

    //##Documentation
    //## @brief Deprecated
    void BackTransform(const mitk::Point3D& at, const mitk::Vector3D& in, mitk::Vector3D& out) const;

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

    static const std::string GetTransformAsString( TransformType* transformType );

    itkGetConstMacro(NDimensions, unsigned int);

    bool IsBoundingBoxNull() const;

    bool IsIndexToWorldTransformNull() const;

    //##Documentation
    //## @brief Intern functions to assure a consistent behaviour of SetSpacing.
    void _SetSpacing(const mitk::Vector3D& aSpacing, bool enforceSetSpacing = false);

  private:
    //##Documentation
    //## @brief Pre- and Post-functions are empty in BaseGeometry
    //##
    //## These virtual functions allow for a different beahiour in subclasses.
    virtual void PreSetBounds(const BoundsArrayType& bounds);

    virtual void PostInitialize();
    virtual void PostInitializeGeometry(Self * newGeometry) const;

    virtual void PostSetExtentInMM(int direction, ScalarType extentInMM);

    //virtual void PostSetTimeBounds(const TimeBounds& timebounds);

    virtual void PreSetIndexToWorldTransform(mitk::AffineTransform3D* transform);
    virtual void PostSetIndexToWorldTransform(mitk::AffineTransform3D* transform);

    virtual void PreSetSpacing(const mitk::Vector3D& aSpacing);

    // ********************************** Variables **********************************
    //##Documentation
    //## @brief Spacing, measurement of the resolution
    //##
    mitk::Vector3D m_Spacing;

    //##Documentation
    //## @brief Index to World Transform, contains a transformation matrix to convert
    //## points from indes coordinates to world coordinates (mm). The Spacing is included in this variable.
    AffineTransform3D::Pointer m_IndexToWorldTransform;

    //##Documentation
    //## @brief Bounding Box, which is axes-parallel in intrinsic coordinates
    //## (often integer indices of pixels)
    BoundingBoxPointer m_BoundingBox;

    vtkMatrixToLinearTransform* m_VtkIndexToWorldTransform;

    vtkMatrix4x4* m_VtkMatrix;

    unsigned int m_FrameOfReferenceID;

    //mitk::TimeBounds m_TimeBounds;

    static const unsigned int m_NDimensions = 3;

    mutable TransformType::Pointer m_InvertedTransform;

    mutable unsigned long m_IndexToWorldTransformLastModified;

    //##Documentation
    //## @brief Origin, i.e. upper-left corner of the plane
    //##
    Point3D m_Origin;

    bool m_ImageGeometry;

    //##Documentation
    //## @brief ModifiedLockFlag is used to prohibit the call of Modified()
    //##
    //## For the use of this Flag, see class ModifiedLock. This flag should only be set
    //## by the ModifiedLock class!
    bool m_ModifiedLockFlag;

    //##Documentation
    //## @brief ModifiedcalledFlag is used to collect calls of Modified().
    //##
    //## For the use of this Flag, see class ModifiedLock. This flag should only be set
    //## by the Modified() function!
    mutable bool m_ModifiedCalledFlag;
  };

  // ********************************** Equal Functions **********************************
  //
  // Static compare functions mainly for testing
  //
  /**
  * @brief Equal A function comparing two geometries for beeing identical.
  * @warning This method is deprecated and will not be available in the future. Use the \a bool mitk::Equal(const mitk::mitk::BaseGeometry& g1, const mitk::BaseGeometry& g2) instead.
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
  DEPRECATED( MITK_CORE_EXPORT bool Equal(const mitk::BaseGeometry* leftHandSide, const mitk::BaseGeometry* rightHandSide, ScalarType eps, bool verbose));

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
  MITK_CORE_EXPORT bool Equal(const mitk::BaseGeometry& leftHandSide, const mitk::BaseGeometry& rightHandSide, ScalarType eps, bool verbose);

  /**
  * @brief Equal A function comparing two transforms (TransformType) for beeing identical.
  * @warning This method is deprecated and will not be available in the future. Use the \a bool mitk::Equal(const mitk::mitk::BaseGeometry::TransformType& t1, const mitk::BaseGeometry::TransformType& t2) instead.
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
  DEPRECATED( MITK_CORE_EXPORT bool Equal(const mitk::BaseGeometry::TransformType *leftHandSide, const mitk::BaseGeometry::TransformType *rightHandSide, ScalarType eps, bool verbose));

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
  MITK_CORE_EXPORT bool Equal(const mitk::BaseGeometry::TransformType& leftHandSide, const mitk::BaseGeometry::TransformType& rightHandSide, ScalarType eps, bool verbose);

  /**
  * @brief Equal A function comparing two bounding boxes (BoundingBoxType) for beeing identical.
  * @warning This method is deprecated and will not be available in the future. Use the \a bool mitk::Equal(const mitk::mitk::BaseGeometry::BoundingBoxType& b1, const mitk::BaseGeometry::BoundingBoxType& b2) instead.
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
  DEPRECATED( MITK_CORE_EXPORT bool Equal( const mitk::BaseGeometry::BoundingBoxType *leftHandSide, const mitk::BaseGeometry::BoundingBoxType *rightHandSide, ScalarType eps, bool verbose));

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
  MITK_CORE_EXPORT bool Equal( const  mitk::BaseGeometry::BoundingBoxType& leftHandSide, const mitk::BaseGeometry::BoundingBoxType& rightHandSide, ScalarType eps, bool verbose);
} // namespace mitk

#endif /* BaseGeometry_H_HEADER_INCLUDED */
