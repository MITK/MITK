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

#include "mitkBaseDataTestImplementation.h"
#include "mitkStringProperty.h"
#include "mitkTestingMacros.h"
#include "itkImage.h"


int mitkBaseDataTest(int /*argc*/, char* /*argv*/[])
{

  MITK_TEST_BEGIN("BaseData")

  //Create a BaseData implementation
  MITK_INFO << "Creating a base data instance...";
  mitk::BaseDataTestImplementation::Pointer baseDataImpl = mitk::BaseDataTestImplementation::New();

  MITK_TEST_CONDITION_REQUIRED(baseDataImpl.IsNotNull(),"Testing instantiation");
  MITK_TEST_CONDITION(baseDataImpl->IsInitialized(), "BaseDataTestImplementation is initialized");
  MITK_TEST_CONDITION(baseDataImpl->IsEmpty(), "BaseDataTestImplementation is initialized and empty");
  MITK_TEST_CONDITION(baseDataImpl->GetExternalReferenceCount()== baseDataImpl->GetReferenceCount(), "Checks external reference count!");

  mitk::BaseDataTestImplementation::Pointer cloneBaseData = baseDataImpl->Clone();
  MITK_TEST_CONDITION_REQUIRED(cloneBaseData.IsNotNull(),"Testing instantiation of base data clone");
  MITK_TEST_CONDITION(cloneBaseData->IsInitialized(), "Clone of BaseDataTestImplementation is initialized");
  MITK_TEST_CONDITION(cloneBaseData->IsEmpty(), "Clone of BaseDataTestImplementation is initialized and empty");
  MITK_TEST_CONDITION(cloneBaseData->GetExternalReferenceCount()== cloneBaseData->GetReferenceCount(), "Checks external reference count of base data clone!");

  MITK_INFO << "Testing setter and getter for geometries...";

  //test method GetTimeSlicedGeometry()
  MITK_TEST_CONDITION(baseDataImpl->GetTimeSlicedGeometry(), "Testing creation of TimeSlicedGeometry");

  mitk::TimeSlicedGeometry* geo = NULL;
  baseDataImpl->SetGeometry(geo);

  MITK_TEST_CONDITION(baseDataImpl->GetTimeSlicedGeometry() == NULL, "Reset Geometry");

  mitk::TimeSlicedGeometry::Pointer geo2 = mitk::TimeSlicedGeometry::New();
  baseDataImpl->SetGeometry(geo2);
  baseDataImpl->InitializeTimeSlicedGeometry(2);
  MITK_TEST_CONDITION(baseDataImpl->GetTimeSlicedGeometry() == geo2, "Correct Reinit of TimeslicedGeometry");

  //test method GetGeometry(int timeStep)
  MITK_TEST_CONDITION(baseDataImpl->GetGeometry(1) != NULL, "... and single Geometries");

  //test method Expand(unsigned int timeSteps)
  baseDataImpl->Expand(5);
  MITK_TEST_CONDITION(baseDataImpl->GetTimeSteps() == 5, "Expand the geometry to further time slices!");

  //test method GetUpdatedGeometry(int timeStep);
  mitk::Geometry3D::Pointer geo3 = mitk::Geometry3D::New();
  mitk::TimeSlicedGeometry::Pointer timeSlicedGeometry = baseDataImpl->GetTimeSlicedGeometry();
  if (timeSlicedGeometry.IsNotNull() )
  {
    timeSlicedGeometry->SetGeometry3D(geo3, 1);
  }

  MITK_TEST_CONDITION(baseDataImpl->GetUpdatedGeometry(1) == geo3, "Set Geometry for time step 1");
  MITK_TEST_CONDITION(baseDataImpl->GetMTime()!= 0, "Check if modified time is set");
  baseDataImpl->SetClonedGeometry(geo3, 1);

  float x[3];
  x[0] = 2;
  x[1] = 4;
  x[2] = 6;
  mitk::Point3D p3d(x);
  baseDataImpl->SetOrigin(p3d);
  geo3->SetOrigin(p3d);

  MITK_TEST_CONDITION(baseDataImpl->GetGeometry(1)->GetOrigin() == geo3->GetOrigin(), "Testing Origin set");

  cloneBaseData = baseDataImpl->Clone();
  MITK_TEST_CONDITION(cloneBaseData->GetGeometry(1)->GetOrigin() == geo3->GetOrigin(), "Testing origin set in clone!");

  MITK_TEST_CONDITION(!baseDataImpl->IsEmptyTimeStep(1), "Is not empty before clear()!");
  baseDataImpl->Clear();
  MITK_TEST_CONDITION(baseDataImpl->IsEmptyTimeStep(1), "...but afterwards!");
  //test method Set-/GetProperty()
  baseDataImpl->SetProperty("property38", mitk::StringProperty::New("testproperty"));
  //baseDataImpl->SetProperty("visibility", mitk::BoolProperty::New());
  MITK_TEST_CONDITION(baseDataImpl->GetProperty("property38")->GetValueAsString() == "testproperty","Check if base property is set correctly!");

  cloneBaseData = baseDataImpl->Clone();
  MITK_TEST_CONDITION(cloneBaseData->GetProperty("property38")->GetValueAsString() == "testproperty", "Testing origin set in clone!");

  //test method Set-/GetPropertyList
  mitk::PropertyList::Pointer propertyList = mitk::PropertyList::New();
  propertyList->SetFloatProperty("floatProperty1", 123.45);
  propertyList->SetBoolProperty("visibility",true);
  propertyList->SetStringProperty("nameXY","propertyName");
  baseDataImpl->SetPropertyList(propertyList);
  bool value = false;
  MITK_TEST_CONDITION(baseDataImpl->GetPropertyList() == propertyList, "Check if base property list is set correctly!");
  MITK_TEST_CONDITION(baseDataImpl->GetPropertyList()->GetBoolProperty("visibility", value) == true, "Check if base property is set correctly in the property list!");

  //test method UpdateOutputInformation()
   baseDataImpl->UpdateOutputInformation();
  MITK_TEST_CONDITION(baseDataImpl->GetUpdatedTimeSlicedGeometry() == geo2, "TimeSlicedGeometry update!");
  //Test method CopyInformation()
  mitk::BaseDataTestImplementation::Pointer newBaseData =  mitk::BaseDataTestImplementation::New();
  newBaseData->CopyInformation(baseDataImpl);
  MITK_TEST_CONDITION_REQUIRED(  newBaseData->GetTimeSlicedGeometry()->GetTimeSteps() == 5, "Check copying of of Basedata Data Object!");

  MITK_TEST_END()
}
