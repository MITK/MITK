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


#include "mitkBoundingObject.h"
#include "mitkBaseProcess.h"

mitk::BoundingObject::BoundingObject()
  : Surface(), m_Positive(true)
{
//  Initialize(1);

  /* bounding box around the unscaled bounding object */
  ScalarType bounds[6]={-1,1,-1,1,-1,1};  //{xmin,x_max, ymin,y_max,zmin,z_max}
  GetGeometry()->SetBounds(bounds);
  GetTimeSlicedGeometry()->UpdateInformation();
}

mitk::BoundingObject::~BoundingObject()
{
}

mitk::ScalarType mitk::BoundingObject::GetVolume()
{
  return 0.0;
}

void mitk::BoundingObject::FitGeometry(mitk::Geometry3D* aGeometry3D)
{

  // Adjusted this function to fix
  // BUG 6951 - Image Cropper - Bounding Box is strange
  // Still, the behavior of the BoundingObject is really strange.
  // One would think that writing "setGeometry(aGeometry3D)" here would do the job.
  // But apparently the boundingObject can only be handled correctly, when it's
  // indexBounds are from -1 to 1 in all axis (so it is only 2x2x2 Pixels big) and the spacing
  // specifies it's actual bounds. This behavior needs to be analyzed and maybe changed.
  // Check also BUG 11406

  if (aGeometry3D->GetImageGeometry())
  {
    aGeometry3D->ChangeImageGeometryConsideringOriginOffset(false);
  }

  GetGeometry()->SetIdentity();
  GetGeometry()->Compose(aGeometry3D->GetIndexToWorldTransform());

  GetGeometry()->SetOrigin(aGeometry3D->GetCenter());

  mitk::Vector3D size;
  for(unsigned int i=0; i < 3; ++i)
    size[i] = (aGeometry3D->GetExtentInMM(i)/2.0) -1;
  GetGeometry()->SetSpacing( size );
  GetTimeSlicedGeometry()->UpdateInformation();
}
