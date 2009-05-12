/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkExternAbstractTransformGeometry.h"
#include <vtkAbstractTransform.h>

mitk::ExternAbstractTransformGeometry::ExternAbstractTransformGeometry()
{
}

mitk::ExternAbstractTransformGeometry::~ExternAbstractTransformGeometry()
{
}

void mitk::ExternAbstractTransformGeometry::SetVtkAbstractTransform(vtkAbstractTransform* aVtkAbstractTransform)
{
  Superclass::SetVtkAbstractTransform(aVtkAbstractTransform);
}

void mitk::ExternAbstractTransformGeometry::SetPlane(const mitk::PlaneGeometry* aPlane)
{
  Superclass::SetPlane(aPlane);
}

void mitk::ExternAbstractTransformGeometry::SetParametricBounds(const BoundingBox::BoundsArrayType& bounds)
{
  Superclass::SetParametricBounds(bounds);
  //@warning affine-transforms and bounding-box should be set by specific sub-classes!
  SetBounds(bounds);

  if(m_Plane.IsNotNull())
  {
    m_Plane->SetSizeInUnits(bounds[1]-bounds[0], bounds[3]-bounds[2]);
    m_Plane->SetBounds(bounds);
  }
}

mitk::AffineGeometryFrame3D::Pointer mitk::ExternAbstractTransformGeometry::Clone() const
{
  Self::Pointer newGeometry = Self::New();
  newGeometry->Initialize();
  InitializeGeometry(newGeometry);
  return newGeometry.GetPointer();
}

void mitk::ExternAbstractTransformGeometry::InitializeGeometry(Self * newGeometry) const
{
  Superclass::InitializeGeometry(newGeometry);
}
