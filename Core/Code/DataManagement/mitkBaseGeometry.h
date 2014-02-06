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

class vtkMatrix4x4;
class vtkMatrixToLinearTransform;

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

    typedef itk::QuaternionRigidTransform< ScalarType > QuaternionTransformType;
    typedef QuaternionTransformType::VnlQuaternionType VnlQuaternionType;

    typedef itk::ScalableAffineTransform<ScalarType, 3>    TransformType;
    typedef itk::BoundingBox<unsigned long, 3, ScalarType> BoundingBoxType;
    typedef BoundingBoxType::BoundsArrayType               BoundsArrayType;
    typedef BoundingBoxType::Pointer                       BoundingBoxPointer;

    //##Documentation
    //## @brief Get the origin, e.g. the upper-left corner of the plane
    const Point3D& GetOrigin() const;

    //##Documentation
    //## @brief Set the origin, i.e. the upper-left corner of the plane
    //##
    virtual void SetOrigin(const Point3D& origin);

    //##Documentation
    //## @brief Copy the ITK transform
    //## (m_IndexToWorldTransform) to the VTK transform
    //## \sa SetIndexToWorldTransform
    virtual void TransferItkToVtkTransform();

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
    //## @brief Initialize the Geometry3D
    virtual void Initialize();

    //##Documentation
    //## \brief Set the bounding box (in index/unit coordinates)
    //##
    //## Only possible via the BoundsArray to make clear that a
    //## copy of the bounding-box is stored, not a reference to it.
    virtual void SetBounds(const BoundsArrayType& bounds);

    virtual void InitializeGeometry(Self * newGeometry) const;

    //##Documentation
    //## @brief Set the bounding box (in index/unit coordinates) via a float array
    virtual void SetFloatBounds(const float bounds[6]);
    //##Documentation
    //## @brief Set the bounding box (in index/unit coordinates) via a double array
    virtual void SetFloatBounds(const double bounds[6]);

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

    static void CopySpacingFromTransform(mitk::AffineTransform3D* transform, mitk::Vector3D& spacing, float floatSpacing[3]);

  protected:
    BaseGeometry();
    BaseGeometry(const BaseGeometry& other);
    virtual ~BaseGeometry();

    AffineTransform3D::Pointer m_IndexToWorldTransform;

    vtkMatrixToLinearTransform* m_VtkIndexToWorldTransform;

    vtkMatrix4x4* m_VtkMatrix;

    bool m_ImageGeometry;

    bool m_Valid;

    unsigned int m_FrameOfReferenceID;

    mutable unsigned long m_IndexToWorldTransformLastModified;  //this was private

    float m_FloatSpacing[3]; //this was private

    mutable mitk::BoundingBox::Pointer m_ParametricBoundingBox;

    mutable mitk::TimeBounds m_TimeBounds;

    mutable BoundingBoxPointer m_BoundingBox;

    VnlQuaternionType m_RotationQuaternion; //this was private

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
  };
} // namespace mitk

#endif BaseGeometry_H_HEADER_INCLUDED
