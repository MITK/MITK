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

#include "mitkRegistrationObject.h"

mitk::RegistrationObject::RegistrationObject()
    : m_ReferenceGeometry(0),
      m_Transformation(0)
{

}

void mitk::RegistrationObject::SetReferenceGeometry(mitk::Geometry3D::Pointer geom)
{
    if( m_ReferenceGeometry.IsNull() )
        m_ReferenceGeometry = mitk::Geometry3D::New();

    m_ReferenceGeometry = geom->Clone();
}

void mitk::RegistrationObject::SetReferenceImage(mitk::Image::Pointer image)
{
    this->SetReferenceGeometry( image->GetTimeGeometry()->GetGeometryForTimeStep() );
}

void mitk::RegistrationObject::SetTransformation(itk::AffineGeometryFrame::Pointer transform)
{
    if( m_Transformation.IsNull() )
        m_Transformation = mitk::AffineGeometryFrame3D::New();

    m_Transformation = transform;
}
