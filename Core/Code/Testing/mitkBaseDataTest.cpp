/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 17495 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkBaseDataImplementation.h"
#include "mitkTestingMacros.h"


int mitkBaseDataTest(int /*argc*/, char* /*argv*/[])
{

  MITK_TEST_BEGIN("BaseData")

  //Create a BaseData implementation
  std::cout << "Creating a base data instance..." << std::endl;
  mitk::BaseDataImplementation::Pointer bd = mitk::BaseDataImplementation::New();

  MITK_TEST_CONDITION_REQUIRED(bd.IsNotNull(),"Testing instantiation")

  MITK_TEST_CONDITION_REQUIRED(bd->GetTimeSlicedGeometry(), "Testing creation of TimeSlicedGeometry");

  std::cout << "Testing setter and getter for geometries..." << std::endl;

  mitk::TimeSlicedGeometry* geo = NULL;
  bd->SetGeometry(geo);

  MITK_TEST_CONDITION_REQUIRED(bd->GetTimeSlicedGeometry() == NULL, "Reset Geometry");

  mitk::TimeSlicedGeometry::Pointer geo2 = mitk::TimeSlicedGeometry::New();
  bd->SetGeometry(geo2);
  bd->InitializeTimeSlicedGeometry(2);

  MITK_TEST_CONDITION_REQUIRED(bd->GetTimeSlicedGeometry() == geo2, "Correct Reinit of TimeslicedGeometry");
  MITK_TEST_CONDITION_REQUIRED(bd->GetGeometry(1) != NULL, "... and single Geometries");

  mitk::Geometry3D::Pointer geo3 = mitk::Geometry3D::New();
  bd->SetGeometry(geo3, 1);

  MITK_TEST_CONDITION_REQUIRED(bd->GetUpdatedGeometry(1) == geo3, "Set Geometry for time step 1");

  bd->SetClonedGeometry(geo3, 1);

  float x[3];
  x[0] = 2; x[1]= 4; x[2] = 6;
  mitk::Point3D p3d(x);
  bd->SetOrigin(p3d);

  geo3->SetOrigin(p3d);

  MITK_TEST_CONDITION_REQUIRED(bd->GetGeometry(1)->GetOrigin() == geo3->GetOrigin(), "Testing Origin set");

  MITK_TEST_END()
}
