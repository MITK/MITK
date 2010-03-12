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
#include "mitkStringProperty.h"
#include "mitkTestingMacros.h"
#include "itkImage.h"


int mitkBaseDataTest(int /*argc*/, char* /*argv*/[])
{

  MITK_TEST_BEGIN("BaseData")

  //Create a BaseData implementation
  std::cout << "Creating a base data instance..." << std::endl;
  mitk::BaseDataImplementation::Pointer baseDataImpl = mitk::BaseDataImplementation::New();

  MITK_TEST_CONDITION_REQUIRED(baseDataImpl.IsNotNull(),"Testing instantiation");
  MITK_TEST_CONDITION(baseDataImpl->IsInitialized(), "BaseDataImplementation is initialized");
  MITK_TEST_CONDITION(baseDataImpl->IsEmpty(), "BaseDataImplementation is initialized and empty");
  MITK_TEST_CONDITION(baseDataImpl->GetExternalReferenceCount()== baseDataImpl->GetReferenceCount(), "Checks external reference count!");
  


  std::cout << "Testing setter and getter for geometries..." << std::endl;

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
  baseDataImpl->SetGeometry(geo3, 1);

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

  MITK_TEST_CONDITION(!baseDataImpl->IsEmpty(1), "Is not empty before clear()!");
  baseDataImpl->Clear();
  MITK_TEST_CONDITION(baseDataImpl->IsEmpty(1), "...but afterwards!");
  //test method Set-/GetProperty()
  baseDataImpl->SetProperty("property38", mitk::StringProperty::New("testproperty"));
  //baseDataImpl->SetProperty("visibility", mitk::BoolProperty::New());
  MITK_TEST_CONDITION(baseDataImpl->GetProperty("property38")->GetValueAsString() == "testproperty","Check if base property is set correctly!");

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
  mitk::BaseDataImplementation::Pointer newBaseData =  mitk::BaseDataImplementation::New();
  newBaseData->CopyInformation(baseDataImpl);
  MITK_TEST_CONDITION(  newBaseData->GetTimeSlicedGeometry()->GetTimeSteps() == 5, "Check copying of of Basedata Data Object!");
 
  MITK_TEST_END()
}
