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
    void SetOrigin(const Point3D& origin);

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

    // ********************************** other functions **********************************

    //##Documentation
    //## @brief Get the DICOM FrameOfReferenceID referring to the
    //## used world coordinate system
    itkGetConstMacro(FrameOfReferenceID, unsigned int);
    //##Documentation
    //## @brief Set the DICOM FrameOfReferenceID referring to the
    //## used world coordinate system
    itkSetMacro(FrameOfReferenceID, unsigned int);

    //##Documentation
    //## @brief Is this Geometry3D in a state that is valid?
    virtual bool IsValid() const;

    // ********************************** Initialize **********************************

    //##Documentation
    //## @brief Initialize the Geometry3D
    virtual void Initialize();

    virtual void InitializeGeometry(Self * newGeometry) const;

    static void CopySpacingFromTransform(mitk::AffineTransform3D* transform, mitk::Vector3D& spacing, float floatSpacing[3]);

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

    /** Set/Get the IndexToWorldTransform */
    itkGetConstObjectMacro(IndexToWorldTransform, AffineTransform3D);
    itkGetObjectMacro(IndexToWorldTransform, AffineTransform3D);

    // ********************************** Transformations **********************************

    //##Documentation
    //## @brief Copy the ITK transform
    //## (m_IndexToWorldTransform) to the VTK transform
    //## \sa SetIndexToWorldTransform
    void TransferItkToVtkTransform();

    //##Documentation
    //## @brief Convert world coordinates (in mm) of a \em point to (continuous!) index coordinates
    //## \warning If you need (discrete) integer index coordinates (e.g., for iterating easily over an image),
    //## use WorldToIndex(const mitk::Point3D& pt_mm, itk::Index<VIndexDimension> &index).
    //## For further information about coordinates types, please see the Geometry documentation
    virtual void WorldToIndex(const mitk::Point3D& pt_mm, mitk::Point3D& pt_units) const;

    //##Documentation
    //## @brief Convert world coordinates (in mm) of a \em vector
    //## \a vec_mm to (continuous!) index coordinates.
    //## For further information about coordinates types, please see the Geometry documentation
    virtual void WorldToIndex(const mitk::Vector3D& vec_mm, mitk::Vector3D& vec_units) const;

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
    //## @brief Convert world coordinates (in mm) of a \em vector
    //## \a vec_mm to (continuous!) index coordinates.
    //## @deprecated First parameter (Point3D) is not used. If possible, please use void WorldToIndex(const mitk::Vector3D& vec_mm, mitk::Vector3D& vec_units) const.
    //## For further information about coordinates types, please see the Geometry documentation
    virtual void WorldToIndex(const mitk::Point3D& atPt3d_mm, const mitk::Vector3D& vec_mm, mitk::Vector3D& vec_units) const;

    // ********************************** BoundingBox **********************************

    //##Documentation
    //## @brief Get the parametric bounding-box
    //##
    //## See AbstractTransformGeometry for an example usage of this.
    itkGetConstObjectMacro(ParametricBoundingBox, BoundingBox);

    /** Get the bounding box */
    itkGetConstObjectMacro(BoundingBox, BoundingBoxType);

    //##Documentation
    //## @brief Get the time bounds (in ms)
    itkGetConstReferenceMacro(TimeBounds, TimeBounds);

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
    virtual void SetBounds(const BoundsArrayType& bounds);

    //##Documentation
    //## @brief Set the bounding box (in index/unit coordinates) via a float array
    void SetFloatBounds(const float bounds[6]);
    //##Documentation
    //## @brief Set the bounding box (in index/unit coordinates) via a double array
    void SetFloatBounds(const double bounds[6]);

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
    virtual Point3D GetCornerPoint(int id) const;

    //##Documentation
    //## @brief Get the position of a corner (in world coordinates)
    //##
    //## See SetImageGeometry for how a corner is defined on images.
    virtual Point3D GetCornerPoint(bool xFront=true, bool yFront=true, bool zFront=true) const;

    //##Documentation
    //## @brief Set the extent of the bounding-box in the specified @a direction in mm
    //##
    //## @note This changes the matrix in the transform, @a not the bounds, which are given in units!
    virtual void SetExtentInMM(int direction, ScalarType extentInMM);

    //##Documentation
    //## @brief Test whether the point \a p (world coordinates in mm) is
    //## inside the bounding box
    bool IsInside(const mitk::Point3D& p) const;

    //##Documentation
    //## @brief Test whether the point \a p ((continous!)index coordinates in units) is
    //## inside the bounding box
    virtual bool IsIndexInside(const mitk::Point3D& index) const;

    //##Documentation
    //## @brief Convenience method for working with ITK indices
    template <unsigned int VIndexDimension>
    bool IsIndexInside(const itk::Index<VIndexDimension> &index) const;

  protected:

    // ********************************** Constructor **********************************
    BaseGeometry();
    BaseGeometry(const BaseGeometry& other);
    virtual ~BaseGeometry();

    //itkGetConstMacro(IndexToWorldTransformLastModified, unsigned long);

    virtual void BackTransform(const mitk::Point3D& in, mitk::Point3D& out) const;

    //Without redundant parameter Point3D
    virtual void BackTransform(const mitk::Vector3D& in, mitk::Vector3D& out) const;

    //##Documentation
    //## @brief Deprecated
    virtual void BackTransform(const mitk::Point3D& at, const mitk::Vector3D& in, mitk::Vector3D& out) const;

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

    //    DEPRECATED(VnlQuaternionType m_RotationQuaternion); //this was private
  };

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
} // namespace mitk

#endif /* BaseGeometry_H_HEADER_INCLUDED */
