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
#include <mitkTimeGeometry.h>
#include <mitkProportionalTimeGeometry.h>
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


  mitk::BaseDataTestImplementation::Pointer cloneBaseData = baseDataImpl->Clone();
  MITK_TEST_CONDITION_REQUIRED(cloneBaseData.IsNotNull(),"Testing instantiation of base data clone");
  MITK_TEST_CONDITION(cloneBaseData->IsInitialized(), "Clone of BaseDataTestImplementation is initialized");
  MITK_TEST_CONDITION(cloneBaseData->IsEmpty(), "Clone of BaseDataTestImplementation is initialized and empty");

  MITK_INFO << "Testing setter and getter for geometries...";

  //test method GetTimeGeometry()
  MITK_TEST_CONDITION(baseDataImpl->GetTimeGeometry(), "Testing creation of TimeGeometry");

  mitk::TimeGeometry* geo = NULL;
  baseDataImpl->SetTimeGeometry(geo);

  MITK_TEST_CONDITION(baseDataImpl->GetTimeGeometry() == NULL, "Reset Geometry");

  mitk::ProportionalTimeGeometry::Pointer geo2 = mitk::ProportionalTimeGeometry::New();
  baseDataImpl->SetTimeGeometry(geo2);
  geo2->Initialize(2);
  MITK_TEST_CONDITION(baseDataImpl->GetTimeGeometry() == geo2.GetPointer(), "Correct Reinit of TimeGeometry");

  //test method GetGeometry(int timeStep)
  MITK_TEST_CONDITION(baseDataImpl->GetGeometry(1) != NULL, "... and single Geometries");

  //test method Expand(unsigned int timeSteps)
  baseDataImpl->Expand(5);
  MITK_TEST_CONDITION(baseDataImpl->GetTimeSteps() == 5, "Expand the geometry to further time slices!");

  //test method GetUpdatedGeometry(int timeStep);
  mitk::Geometry3D::Pointer geometry3D = mitk::Geometry3D::New();
  mitk::BaseGeometry::Pointer geo3 = dynamic_cast<mitk::BaseGeometry*>(geometry3D.GetPointer());
  mitk::ProportionalTimeGeometry::Pointer timeGeometry = dynamic_cast<mitk::ProportionalTimeGeometry *>(baseDataImpl->GetTimeGeometry());
  if (timeGeometry.IsNotNull() )
  {
    timeGeometry->SetTimeStepGeometry(geo3,1);
  }

  MITK_TEST_CONDITION(baseDataImpl->GetUpdatedGeometry(1) == geo3, "Set Geometry for time step 1");
  MITK_TEST_CONDITION(baseDataImpl->GetMTime()!= 0, "Check if modified time is set");
  baseDataImpl->SetClonedGeometry(geo3, 1);

  mitk::ScalarType x[3];
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
  MITK_TEST_CONDITION(baseDataImpl->GetUpdatedTimeGeometry() == geo2, "TimeGeometry update!");
  //Test method CopyInformation()
  mitk::BaseDataTestImplementation::Pointer newBaseData =  mitk::BaseDataTestImplementation::New();
  newBaseData->CopyInformation(baseDataImpl);
  MITK_TEST_CONDITION_REQUIRED(  newBaseData->GetTimeGeometry()->CountTimeSteps() == 5, "Check copying of of Basedata Data Object!");

  MITK_TEST_END()
}
