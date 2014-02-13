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

#include "mitkTestingMacros.h"
#include <mitkTestingConfig.h>
#include <mitkTestFixture.h>

#include <mitkBaseGeometry.h>
#include <mitkImageGenerator.h>
#include <itkBoundingBox.h>

#include <itksys/SystemTools.hxx>

//typedef itk::BoundingBox<unsigned long, 3, mitk::ScalarType> BoundingBox;
//typedef BoundingBoxType::Pointer                       BoundingBoxPointer;

// Dummy instance of abstract base class
class DummyTestClass : public mitk::BaseGeometry
{
public:
  DummyTestClass():BaseGeometry(){}
  DummyTestClass(const DummyTestClass& other) : BaseGeometry(other){}
  ~DummyTestClass(){}

  void DummyTestClass::ExecuteOperation(mitk::Operation* operation){}
};

class mitkBaseGeometryTestSuite : public mitk::TestFixture
{
  // List of Tests
  CPPUNIT_TEST_SUITE(mitkBaseGeometryTestSuite);
  MITK_TEST(TestConstructor);
  MITK_TEST(TestSetOrigin);
  CPPUNIT_TEST_SUITE_END();

  // Used Variables
private:
  mitk::Point3D  aPoint;

public:

  // Set up for variables
  void setUp()
  {
    mitk::FillVector3D(aPoint, 2,3,4);
  }

  // Test functions

  void TestSetOrigin()
  {
    DummyTestClass dummy1;
    dummy1.SetOrigin(aPoint);
    CPPUNIT_ASSERT(aPoint==dummy1.GetOrigin());
  }

  void TestConstructor()
  {
    //test standard constructor
    DummyTestClass dummy1;

    CPPUNIT_ASSERT(dummy1.IsValid() == true);
    CPPUNIT_ASSERT(2==1);
    /*
    CPPUNIT_ASSERT(dummy1.m_FrameOfReferenceID==0);
    CPPUNIT_ASSERT(dummy1.m_IndexToWorldTransformLastModified==0);
    float l_FloatSpacing[3];
    mitk::FillVector3D(l_FloatSpacing, 1,1,1);
    CPPUNIT_ASSERT(dummy1.m_FloatSpacing[0]==l_FloatSpacing[0]);
    CPPUNIT_ASSERT(dummy1.m_FloatSpacing[1]==l_FloatSpacing[1]);
    CPPUNIT_ASSERT(dummy1.m_FloatSpacing[2]==l_FloatSpacing[2]);
    mitk::Vector3D l_Spacing;
    mitk::FillVector3D(l_Spacing, 1,1,1);
    CPPUNIT_ASSERT(dummy1.m_Spacing[0]==l_Spacing[0]);
    CPPUNIT_ASSERT(dummy1.m_Spacing[1]==l_Spacing[1]);
    CPPUNIT_ASSERT(dummy1.m_Spacing[2]==l_Spacing[2]);
    mitk::Point3D l_Origin;
    mitk::FillVector3D(l_Origin,0,0,0);
    CPPUNIT_ASSERT(dummy1.GetOrigin()==l_Origin);

    //xx missing: Bounding box test m_ParametricBoundingBox m_BoundingBox m_TimeBounds[0]
    //  m_IndexToWorldTransform->SetIdentity();
    //  m_VtkMatrix
    // m_VtkIndexToWorldTransform
    // m_RotationQuaternion
    // m_InvertedTransform*/

    //test BaseGeometry(const BaseGeometry& other) constructor
    //    DummyTestClass dummy2;
    //CPPUNIT_ASSERT(Equal(dummy1,dummy2,mitk::eps,true));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkBaseGeometry)
