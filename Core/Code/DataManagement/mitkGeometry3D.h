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

#include <MitkCoreExports.h>
#include <mitkCommon.h>
#include "mitkNumericTypes.h"

#include "itkScalableAffineTransform.h"
#include <itkIndex.h>

#include "mitkBaseGeometry.h"

class vtkLinearTransform;

namespace mitk {
  //##Documentation
  //## @brief Standard implementation of BaseGeometry.
  //##
  //## @ingroup Geometry
  class MITK_CORE_EXPORT Geometry3D : public BaseGeometry
  {
  public:
    mitkClassMacro(Geometry3D, mitk::BaseGeometry);

    typedef itk::QuaternionRigidTransform< ScalarType > QuaternionTransformType;
    typedef QuaternionTransformType::VnlQuaternionType VnlQuaternionType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self)
    mitkNewMacro1Param(Self, const Self&);

    itkCloneMacro(Self)
      //itkGetConstReferenceMacro(TimeBounds, TimeBounds);

      //virtual void SetTimeBounds(const TimeBounds& timebounds);

  protected:
    Geometry3D();
    Geometry3D(const Geometry3D& other);

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

    virtual ~Geometry3D();
  };
} // namespace mitk

#endif /* GEOMETRY3D_H_HEADER_INCLUDED_C1EBD0AD */
