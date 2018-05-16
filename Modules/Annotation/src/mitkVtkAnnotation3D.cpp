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

#include "mitkVtkAnnotation3D.h"

mitk::VtkAnnotation3D::VtkAnnotation3D()
{
  mitk::Point3D offsetVector;
  offsetVector.Fill(0);
  SetOffsetVector(offsetVector);
}

mitk::VtkAnnotation3D::~VtkAnnotation3D()
{
}

void mitk::VtkAnnotation3D::SetPosition3D(const Point3D &position3D)
{
  mitk::Point3dProperty::Pointer position3dProperty = mitk::Point3dProperty::New(position3D);
  SetProperty("VtkAnnotation3D.Position3D", position3dProperty.GetPointer());
}

mitk::Point3D mitk::VtkAnnotation3D::GetPosition3D() const
{
  mitk::Point3D position3D;
  position3D.Fill(0);
  GetPropertyValue<mitk::Point3D>("VtkAnnotation3D.Position3D", position3D);
  return position3D;
}

void mitk::VtkAnnotation3D::SetOffsetVector(const Point3D &OffsetVector)
{
  mitk::Point3dProperty::Pointer OffsetVectorProperty = mitk::Point3dProperty::New(OffsetVector);
  SetProperty("VtkAnnotation3D.OffsetVector", OffsetVectorProperty.GetPointer());
}

mitk::Point3D mitk::VtkAnnotation3D::GetOffsetVector() const
{
  mitk::Point3D OffsetVector;
  OffsetVector.Fill(0);
  GetPropertyValue<mitk::Point3D>("VtkAnnotation3D.OffsetVector", OffsetVector);
  return OffsetVector;
}
