/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGeometry3D_h
#define mitkGeometry3D_h

#include <mitkNumericTypes.h>
#include <MitkCoreExports.h>
#include <mitkCommon.h>

#include <itkScalableAffineTransform.h>
#include <itkIndex.h>

#include <mitkBaseGeometry.h>

class vtkLinearTransform;

namespace mitk
{
  /**
   * \brief Standard three-dimensional geometry.
   *
   * Geometry3D is the default concrete implementation of BaseGeometry.
   * It provides a standard 3D spatial reference (origin, spacing, and
   * affine IndexToWorldTransform) without any additional constraints
   * beyond those defined by BaseGeometry.
   *
   * Most MITK data objects (Image, Surface, PointSet) use Geometry3D
   * as their spatial geometry unless a more specialized geometry
   * (PlaneGeometry, SlicedGeometry3D, etc.) is required.
   *
   * \sa BaseGeometry, PlaneGeometry, SlicedGeometry3D
   * \ingroup Geometry
   */
  class MITKCORE_EXPORT Geometry3D : public BaseGeometry
  {
  public:
    mitkClassMacro(Geometry3D, mitk::BaseGeometry);

    /** \brief Quaternion rigid transform type (for rotation representations). */
    typedef itk::QuaternionRigidTransform<ScalarType> QuaternionTransformType;
    /** \brief VNL quaternion type derived from QuaternionTransformType. */
    typedef QuaternionTransformType::VnlQuaternionType VnlQuaternionType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    mitkNewMacro1Param(Self, const Self&);

    itkCloneMacro(Self);

  protected : Geometry3D();
    Geometry3D(const Geometry3D &);

    ~Geometry3D() override;

    mitkCloneMacro(Self);

    /**
      * @brief PreSetSpacing
      *
      * These virtual function allows a different beahiour in subclasses.
      * Do implement them in every subclass of BaseGeometry. If not needed, use
      * \c Superclass::PreSetSpacing().
      */
    void PreSetSpacing(const mitk::Vector3D &aSpacing) override { Superclass::PreSetSpacing(aSpacing); };
  };
} // namespace mitk

#endif
