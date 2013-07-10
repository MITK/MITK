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

#include "mitkGeometry3D.h"
#include "mitkTestingMacros.h"


int mitkCompareGeometry3DTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN(mitkCompareGeometry3DTest);

  // Build up a new image Geometry
  mitk::Geometry3D::Pointer geometry3d = mitk::Geometry3D::New();
  //  float bounds[ ] = {-10.0, 17.0, -12.0, 188.0, 13.0, 211.0};

  geometry3d->Initialize();
  //  geometry3d->SetFloatBounds(bounds);

  mitk::Geometry3D::Pointer geometry3dTmpCopy = geometry3d->Clone();

  MITK_TEST_CONDITION( mitk::Geometry3D::AreEqual( geometry3d, geometry3dTmpCopy), "A clone should be equal to its original.");

  mitk::Point3D origin;
  origin[0] = 0.0;
  origin[1] = 0.0;
  origin[2] = mitk::eps * 1.01;
  geometry3dTmpCopy->SetOrigin(origin);

  MITK_TEST_CONDITION( !mitk::Geometry3D::AreEqual( geometry3d, geometry3dTmpCopy), "We modified the origin. It should differ.");

  MITK_TEST_END();

  return EXIT_SUCCESS;
}
