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

#ifndef MITKREGISTRATIONOBJECT_H
#define MITKREGISTRATIONOBJECT_H

#include <MitkDiffusionCoreExports.h>

#include "mitkBaseData.h"
#include "mitkGeometry3D.h"

#include "mitkImage.h"

namespace mitk
{
/**
 * @brief The RegistrationObject class stores all necessary information for image registration.
 *
 *
 */

class MITKDIFFUSIONCORE_EXPORT RegistrationObject : public BaseData
{
public:

  mitkClassMacro( RegistrationObject, BaseData );
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  void SetReferenceGeometry( mitk::Geometry3D::Pointer );

  void SetReferenceImage( mitk::Image::Pointer );

  void SetTransformation( mitk::AffineGeometryFrame3D::Pointer );

  mitk::Geometry3D::Pointer GetReferenceGeometry() const
  {
      return m_ReferenceGeometry;
  }

  mitk::AffineGeometryFrame3D::Pointer GetTransformation() const
  {
      return m_Transformation;
  }

protected:
  RegistrationObject();
  virtual ~RegistrationObject(){}

  /**
   * @brief Reference geometry of the fixed image used for registration computation
   */
  mitk::Geometry3D::Pointer m_ReferenceGeometry;

  mitk::AffineGeometryFrame3D::Pointer m_Transformation;


};

}
#endif // MITKREGISTRATIONOBJECT_H
