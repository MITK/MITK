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
// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

// std includes
#include <string>

// MITK includes
#include "mitkBaseDataTestImplementation.h"
#include "mitkStringProperty.h"
#include <mitkProportionalTimeGeometry.h>
#include <mitkTimeGeometry.h>

//itksys
#include "itkImage.h"

// VTK includes
#include <vtkDebugLeaks.h>



class mitkBaseDataTestSuite : public mitk:: TestFixture
{
	CPPUNIT_TEST_SUITE(mitkBaseDataTestSuite);
	MITK_TEST(CreateBaseData_Success);
	MITK_TEST(TestingGetTimeGeometry_Success);
	MITK_TEST(TestingGetGeometry_Failure);
	MITK_TEST(TestingExpand_Success);
	MITK_TEST(TestingGetUpdateGeometry_Success);
	MITK_TEST(TestingSetAndGetProperty_Success);
	MITK_TEST(TestingSetAndGetPropertyList_Success);
    MITK_TEST(TestingUpdateOutputInformation_Failure);
	MITK_TEST(TestingCopyInformation_Failure);
	CPPUNIT_TEST_SUITE_END();

private:

	mitk::BaseDataTestImplementation::Pointer m_baseDataImpl;
	mitk::BaseDataTestImplementation::Pointer m_cloneBaseData;
	mitk::TimeGeometry *m_geo;
	mitk::ProportionalTimeGeometry::Pointer m_geo2;

public:

void setUp() /*override*/ {

	mitk::BaseDataTestImplementation::Pointer m_baseDataImpl = mitk::BaseDataTestImplementation::New();
	mitk::BaseDataTestImplementation::Pointer m_cloneBaseData = m_baseDataImpl->Clone();
	mitk::TimeGeometry *m_geo = nullptr;
	mitk::ProportionalTimeGeometry::Pointer m_geo2 = mitk::ProportionalTimeGeometry::New();
}

void tearDown() /*override*/ {

	mitk::BaseDataTestImplementation::Pointer m_baseDataImpl = nullptr;
	mitk::BaseDataTestImplementation::Pointer m_cloneBaseData =nullptr;
	mitk::TimeGeometry *m_geo = nullptr;
	mitk::ProportionalTimeGeometry::Pointer m_geo2 = nullptr;
}

void CreateBaseData_Success() {
// Create a BaseData implementation
	MITK_INFO << "Creating a base data instance...";
	
	m_baseDataImpl = mitk::BaseDataTestImplementation::New();
	CPPUNIT_ASSERT_MESSAGE("Testing instantiation", m_baseDataImpl.IsNotNull());
	CPPUNIT_ASSERT_MESSAGE("BaseDataTestImplementation is initialized", m_baseDataImpl->IsInitialized());
	CPPUNIT_ASSERT_MESSAGE("BaseDataTestImplementation is initialized and empty", m_baseDataImpl->IsEmpty());

	m_cloneBaseData = m_baseDataImpl->Clone();
	CPPUNIT_ASSERT_MESSAGE("Testing instantiation of base data clone", m_cloneBaseData.IsNotNull());
	CPPUNIT_ASSERT_MESSAGE("Clone of BaseDataTestImplementation is initialized", m_cloneBaseData->IsInitialized());
	CPPUNIT_ASSERT_MESSAGE("Clone of BaseDataTestImplementation is initialized and empty", m_cloneBaseData->IsEmpty());

	MITK_INFO << "Testing setter and getter for geometries...";
}

void TestingGetTimeGeometry_Success() {
// test method GetTimeGeometry()

	m_baseDataImpl = mitk::BaseDataTestImplementation::New();
	m_geo2 = mitk::ProportionalTimeGeometry::New();
    CPPUNIT_ASSERT_MESSAGE("Testing creation of TimeGeometry", m_baseDataImpl->GetTimeGeometry());

	m_baseDataImpl->SetTimeGeometry(m_geo);

    CPPUNIT_ASSERT_MESSAGE("Reset Geometry", m_baseDataImpl->GetTimeGeometry() == nullptr);

	m_baseDataImpl->SetTimeGeometry(m_geo2);
    m_geo2->Initialize(2);
    CPPUNIT_ASSERT_MESSAGE("Correct Reinit of TimeGeometry", m_baseDataImpl->GetTimeGeometry() == m_geo2.GetPointer());

}

void TestingGetGeometry_Failure() {
	// test method GetGeometry(int timeStep)
	m_baseDataImpl = mitk::BaseDataTestImplementation::New();
	CPPUNIT_ASSERT_MESSAGE("... and single Geometries", m_baseDataImpl->GetGeometry(1) == nullptr);
}

void TestingExpand_Success(){
	// test method Expand(unsigned int timeSteps)
	m_baseDataImpl = mitk::BaseDataTestImplementation::New();
	m_baseDataImpl->Expand(5);
	CPPUNIT_ASSERT_MESSAGE("Expand the geometry to further time slices!", m_baseDataImpl->GetTimeSteps() == 5);
}

void TestingGetUpdateGeometry_Success() {
	// test method GetUpdatedGeometry(int timeStep);
	m_baseDataImpl = mitk::BaseDataTestImplementation::New();
	m_baseDataImpl->Expand(5);
	mitk::Geometry3D::Pointer geometry3D = mitk::Geometry3D::New();
	mitk::BaseGeometry::Pointer geo3 = dynamic_cast<mitk::BaseGeometry *>(geometry3D.GetPointer());
	mitk::ProportionalTimeGeometry::Pointer timeGeometry =
		dynamic_cast<mitk::ProportionalTimeGeometry *>(m_baseDataImpl->GetTimeGeometry());
	if (timeGeometry.IsNotNull())
	{
	  timeGeometry->SetTimeStepGeometry(geo3, 1);
	}
	
	CPPUNIT_ASSERT_MESSAGE("Set Geometry for time step 1", m_baseDataImpl->GetUpdatedGeometry(1) == geo3);
	CPPUNIT_ASSERT_MESSAGE("Check if modified time is set", m_baseDataImpl->GetMTime() != 0);
	m_baseDataImpl->SetClonedGeometry(geo3, 1);
	
	mitk::ScalarType x[3];
	x[0] = 2;
	x[1] = 4;
	x[2] = 6;
	mitk::Point3D p3d(x);
	m_baseDataImpl->SetOrigin(p3d);
	geo3->SetOrigin(p3d);
	
	CPPUNIT_ASSERT_MESSAGE("Testing Origin set", m_baseDataImpl->GetGeometry(1)->GetOrigin() == geo3->GetOrigin());
	m_cloneBaseData = m_baseDataImpl->Clone();
	CPPUNIT_ASSERT_MESSAGE("Testing origin set in clone!", m_cloneBaseData->GetGeometry(1)->GetOrigin() == geo3->GetOrigin());
	
	CPPUNIT_ASSERT_MESSAGE("Is not empty before clear()!", !m_baseDataImpl->IsEmptyTimeStep(1));
	m_baseDataImpl->Clear();
	CPPUNIT_ASSERT_MESSAGE("...but afterwards!", m_baseDataImpl->IsEmptyTimeStep(1));
}

void TestingSetAndGetProperty_Success() {
	 //test method Set-/GetProperty()
	m_baseDataImpl = mitk::BaseDataTestImplementation::New();
	m_baseDataImpl->SetProperty("property38", mitk::StringProperty::New("testproperty"));
	 CPPUNIT_ASSERT_MESSAGE("Check if base property is set correctly!", m_baseDataImpl->GetProperty("property38")->GetValueAsString() == "testproperty");
	 
	 m_cloneBaseData = m_baseDataImpl->Clone();
	 CPPUNIT_ASSERT_MESSAGE("Testing origin set in clone!", m_cloneBaseData->GetProperty("property38")->GetValueAsString() == "testproperty");
}

void TestingSetAndGetPropertyList_Success() {
	// test method Set-/GetPropertyList
	m_baseDataImpl = mitk::BaseDataTestImplementation::New();
	 mitk::PropertyList::Pointer propertyList = mitk::PropertyList::New();
	 propertyList->SetFloatProperty("floatProperty1", 123.45);
	 propertyList->SetBoolProperty("visibility", true);
	 propertyList->SetStringProperty("nameXY", "propertyName");
	 m_baseDataImpl->SetPropertyList(propertyList);
	 bool value = false;
	 CPPUNIT_ASSERT_MESSAGE("Check if base property list is set correctly!", m_baseDataImpl->GetPropertyList() == propertyList);
	 CPPUNIT_ASSERT_MESSAGE("Check if base property is set correctly in the property list!",
		 m_baseDataImpl->GetPropertyList()->GetBoolProperty("visibility", value) == true);
}

void TestingUpdateOutputInformation_Failure() {
	// test method UpdateOutputInformation()
	//m_baseDataImpl = mitk::BaseDataTestImplementation::New();
	//m_geo2 = mitk::ProportionalTimeGeometry::New();
	m_baseDataImpl->UpdateOutputInformation();
	m_geo2->Initialize(2);
	m_geo2.GetPointer();
	CPPUNIT_ASSERT_MESSAGE("TimeGeometry update!", m_baseDataImpl->GetUpdatedTimeGeometry() != m_geo2);
}

void TestingCopyInformation_Failure() {
	// Test method CopyInformation()
	m_baseDataImpl = mitk::BaseDataTestImplementation::New();
	mitk::BaseDataTestImplementation::Pointer newBaseData = mitk::BaseDataTestImplementation::New();
	newBaseData->CopyInformation(m_baseDataImpl);
	CPPUNIT_ASSERT_MESSAGE("Check copying of of Basedata Data Object!", newBaseData->GetTimeGeometry()->CountTimeSteps() != 5);
}
};
MITK_TEST_SUITE_REGISTRATION(mitkBaseData)



