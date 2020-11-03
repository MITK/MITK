/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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

#include "itkImage.h"

// VTK includes
#include <vtkDebugLeaks.h>

class mitkBaseDataTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkBaseDataTestSuite);

  MITK_TEST(CreateBaseData_Success);
  MITK_TEST(InitializationOfBaseData_Success);

  MITK_TEST(CreateCloneBaseData_Success);
  MITK_TEST(InitializationOfCloneBaseData_Success);

  MITK_TEST(GetAndSetTimeGeometry_Success);
  MITK_TEST(ResetTimeGeometry_Success);
  MITK_TEST(ReinitOfTimeGeometry_Success);

  MITK_TEST(GetGeometryForSingleTimeGeometries_Failure);

  MITK_TEST(TestingExpand_Success);

  MITK_TEST(TestingGetUpdateGeometry_Success);

  MITK_TEST(GetOriginOfBaseData_Success);
  MITK_TEST(GetOriginOfCloneBaseData);

  MITK_TEST(ClearATimeStep);

  MITK_TEST(BaseDataSetAndGetProperty_Success);
  MITK_TEST(CloneBaseDataSetAndGetProperty_Success);

  MITK_TEST(BasePropertyListIsSet_Success);
  MITK_TEST(BasePorpertyIsSetInPropertyList_Success);

  MITK_TEST(UpdateOutputInformationOfBaseData_Failure);
  MITK_TEST(CopyingInformationOfBaseData_Failure);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::BaseDataTestImplementation::Pointer m_BaseDataImpl;
  mitk::BaseDataTestImplementation::Pointer m_CloneBaseData;

  mitk::TimeGeometry *m_Geo;
  mitk::ProportionalTimeGeometry::Pointer m_Geo2;

  mitk::Geometry3D::Pointer m_Geometry3D;
  mitk::BaseGeometry::Pointer m_Geo3;

  mitk::ScalarType m_X[3];
  mitk::PropertyList::Pointer m_PropertyList;

public:
  void setUp() override
  {
    m_BaseDataImpl = mitk::BaseDataTestImplementation::New();
    m_CloneBaseData = m_BaseDataImpl->Clone();

    m_Geo = nullptr;
    m_Geo2 = mitk::ProportionalTimeGeometry::New();

    m_Geometry3D = mitk::Geometry3D::New();
    m_Geo3 = dynamic_cast<mitk::BaseGeometry *>(m_Geometry3D.GetPointer());

    m_X[0] = 2;
    m_X[1] = 4;
    m_X[2] = 6;

    m_PropertyList = mitk::PropertyList::New();
  }

  void tearDown() override
  {
    m_BaseDataImpl = nullptr;
    m_CloneBaseData = nullptr;

    m_Geo = nullptr;
    m_Geo2 = nullptr;

    m_Geometry3D = nullptr;
    m_Geo3 = nullptr;

    m_X[0] = 0;
    m_X[1] = 0;
    m_X[2] = 0;

    m_PropertyList = nullptr;
  }

  void CreateBaseData_Success()
  {
    // Create a BaseData implementation
    MITK_INFO << "Creating a base data instance...";
    CPPUNIT_ASSERT_MESSAGE("Testing instantiation", m_BaseDataImpl.IsNotNull());
  }

  void InitializationOfBaseData_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("BaseDataTestImplementation is initialized", m_BaseDataImpl->IsInitialized());
    CPPUNIT_ASSERT_MESSAGE("BaseDataTestImplementation is initialized and empty", m_BaseDataImpl->IsEmpty());
  }

  void CreateCloneBaseData_Success()
  {
    // Create CloneBaseData implementation
    MITK_INFO << "Creating a clone base data instance...";
    CPPUNIT_ASSERT_MESSAGE("Testing instantiation of base data clone", m_CloneBaseData.IsNotNull());
  }

  void InitializationOfCloneBaseData_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Clone of BaseDataTestImplementation is initialized", m_CloneBaseData->IsInitialized());
    CPPUNIT_ASSERT_MESSAGE("Clone of BaseDataTestImplementation is initialized and empty", m_CloneBaseData->IsEmpty());
  }

  void GetAndSetTimeGeometry_Success()
  {
    // test method GetTimeGeometry()
    MITK_INFO << "Testing setter and getter for geometries...";
    CPPUNIT_ASSERT_MESSAGE("Testing creation of TimeGeometry", m_BaseDataImpl->GetTimeGeometry());
  }

  void ResetTimeGeometry_Success()
  {
    m_BaseDataImpl->SetTimeGeometry(m_Geo);
    CPPUNIT_ASSERT_MESSAGE("Reset Geometry", m_BaseDataImpl->GetTimeGeometry() == nullptr);
  }

  void ReinitOfTimeGeometry_Success()
  {
    m_BaseDataImpl->SetTimeGeometry(m_Geo2);
    m_Geo2->Initialize(2);
    CPPUNIT_ASSERT_MESSAGE("Correct Reinit of TimeGeometry", m_BaseDataImpl->GetTimeGeometry() == m_Geo2.GetPointer());
  }

  void GetGeometryForSingleTimeGeometries_Failure()
  {
    // test method GetGeometry(int timeStep)
    CPPUNIT_ASSERT_MESSAGE("Testing Creation of single TimeGeometries", m_BaseDataImpl->GetGeometry(1) == nullptr);
  }

  void TestingExpand_Success()
  {
    // test method Expand(unsigned int timeSteps)
    m_BaseDataImpl->Expand(5);
    CPPUNIT_ASSERT_MESSAGE("Expand the geometry to further time slices!", m_BaseDataImpl->GetTimeSteps() == 5);
  }

  void TestingGetUpdateGeometry_Success()
  {
    // test method GetUpdatedGeometry(int timeStep);
    m_BaseDataImpl->Expand(5);
    mitk::ProportionalTimeGeometry::Pointer timeGeometry =
      dynamic_cast<mitk::ProportionalTimeGeometry *>(m_BaseDataImpl->GetTimeGeometry());
    if (timeGeometry.IsNotNull())
    {
      timeGeometry->SetTimeStepGeometry(m_Geo3, 1);
    }

    CPPUNIT_ASSERT_MESSAGE("Set Geometry for time step 1", m_BaseDataImpl->GetUpdatedGeometry(1) == m_Geo3);
    CPPUNIT_ASSERT_MESSAGE("Check if modified time is set", m_BaseDataImpl->GetMTime() != 0);
  }

  void GetOriginOfBaseData_Success()
  {
    m_BaseDataImpl->Expand(5);
    m_BaseDataImpl->SetClonedGeometry(m_Geo3, 1);

    mitk::Point3D p3d(m_X);
    m_BaseDataImpl->SetOrigin(p3d);
    m_Geo3->SetOrigin(p3d);
    CPPUNIT_ASSERT_MESSAGE("Testing Origin set", m_BaseDataImpl->GetGeometry(1)->GetOrigin() == m_Geo3->GetOrigin());
  }
  void GetOriginOfCloneBaseData()
  {
    m_BaseDataImpl->Expand(5);
    m_BaseDataImpl->SetClonedGeometry(m_Geo3, 1);

    mitk::Point3D p3d(m_X);
    m_BaseDataImpl->SetOrigin(p3d);
    m_Geo3->SetOrigin(p3d);

    m_CloneBaseData = m_BaseDataImpl->Clone();
    CPPUNIT_ASSERT_MESSAGE("Testing origin set in clone!",
                           m_CloneBaseData->GetGeometry(1)->GetOrigin() == m_Geo3->GetOrigin());
  }

  void ClearATimeStep()
  {
    CPPUNIT_ASSERT_MESSAGE("Is not empty before clear()!", !m_BaseDataImpl->IsEmptyTimeStep(1));
    m_BaseDataImpl->Clear();
    CPPUNIT_ASSERT_MESSAGE("...but afterwards!", m_BaseDataImpl->IsEmptyTimeStep(1));
  }

  void BaseDataSetAndGetProperty_Success()
  {
    // test method Set-/GetProperty()
    m_BaseDataImpl->SetProperty("property38", mitk::StringProperty::New("testproperty"));
    CPPUNIT_ASSERT_MESSAGE("Check if base property is set correctly!",
                           m_BaseDataImpl->GetProperty("property38")->GetValueAsString() == "testproperty");
  }

  void CloneBaseDataSetAndGetProperty_Success()
  {
    m_BaseDataImpl->SetProperty("property38", mitk::StringProperty::New("testproperty"));
    m_CloneBaseData = m_BaseDataImpl->Clone();
    CPPUNIT_ASSERT_MESSAGE("Testing origin set in clone!",
                           m_CloneBaseData->GetProperty("property38")->GetValueAsString() == "testproperty");
  }

  void BasePropertyListIsSet_Success()
  {
    // test method Set-/GetPropertyList
    m_PropertyList->SetFloatProperty("floatProperty1", 123.45);
    m_PropertyList->SetBoolProperty("visibility", true);
    m_PropertyList->SetStringProperty("nameXY", "propertyName");
    m_BaseDataImpl->SetPropertyList(m_PropertyList);

    CPPUNIT_ASSERT_MESSAGE("Check if base property list is set correctly!",
                           m_BaseDataImpl->GetPropertyList() == m_PropertyList);
  }

  void BasePorpertyIsSetInPropertyList_Success()
  {
    m_PropertyList->SetFloatProperty("floatProperty1", 123.45);
    m_PropertyList->SetBoolProperty("visibility", true);
    m_PropertyList->SetStringProperty("nameXY", "propertyName");
    m_BaseDataImpl->SetPropertyList(m_PropertyList);
    bool value = false;
    CPPUNIT_ASSERT_MESSAGE("Check if base property is set correctly in the property list!",
                           m_BaseDataImpl->GetPropertyList()->GetBoolProperty("visibility", value) == true);
  }

  void UpdateOutputInformationOfBaseData_Failure()
  {
    // test method UpdateOutputInformation()
    m_BaseDataImpl->UpdateOutputInformation();
    m_Geo2->Initialize(2);
    m_Geo2.GetPointer();
    CPPUNIT_ASSERT_MESSAGE("TimeGeometry update!", m_BaseDataImpl->GetUpdatedTimeGeometry() != m_Geo2);
  }

  void CopyingInformationOfBaseData_Failure()
  {
    // Test method CopyInformation()
    mitk::BaseDataTestImplementation::Pointer newBaseData = mitk::BaseDataTestImplementation::New();
    newBaseData->CopyInformation(m_BaseDataImpl);
    CPPUNIT_ASSERT_MESSAGE("Check copying of Basedata Data Object!",
                           newBaseData->GetTimeGeometry()->CountTimeSteps() != 5);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkBaseData)
