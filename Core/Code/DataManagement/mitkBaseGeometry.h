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
    virtual void TransferItkToVtkTransform();

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
    //virtual const BoundingBox::BoundsArrayType& GetParametricBounds() const = 0 ; //ToDo

    /** Get the bounding box */
    itkGetConstObjectMacro(BoundingBox, BoundingBoxType);

    //##Documentation
    //## @brief Get the time bounds (in ms)
    itkGetConstReferenceMacro(TimeBounds, TimeBounds);

    const BoundsArrayType GetBounds() const;

    // a bit of a misuse, but we want only doxygen to see the following:
#ifdef DOXYGEN_SKIP
    //##Documentation
    //## @brief Get bounding box (in index/unit coordinates)
    itkGetConstObjectMacro(BoundingBox, BoundingBoxType);
    //##Documentation
    //## @brief Get bounding box (in index/unit coordinates) as a BoundsArrayType
    const BoundsArrayType GetBounds() const;
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
    //virtual void SetTimeBounds(const TimeBounds& timebounds) = 0 ; //ToDo

    // ********************************** Geometry **********************************

#ifdef DOXYGEN_SKIP
    //##Documentation
    //## @brief Get the extent of the bounding box (in index/unit coordinates)
    //##
    //## To access the extent in mm use GetExtentInMM
    ScalarType GetExtent(unsigned int direction) const;
#endif

  protected:

    // ********************************** Constructor **********************************
    BaseGeometry();
    BaseGeometry(const BaseGeometry& other);
    virtual ~BaseGeometry();

    //itkGetConstMacro(IndexToWorldTransformLastModified, unsigned long);

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
