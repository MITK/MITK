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

#include <itksys/SystemTools.hxx>

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
  MITK_TEST(TestSetOrigin);
  CPPUNIT_TEST_SUITE_END();

  // Used Variables
private:
  DummyTestClass dummy1;
  mitk::Point3D  aPoint;

public:

  // Set up for variables
  void setUp()
  {
    mitk::FillVector3D(aPoint, 1,2,3);
  }

  // Test functions

  void TestSetOrigin()
  {
    std::cout<<"- Set Origin Test -"<<std::endl;
    dummy1.SetOrigin(aPoint);
    CPPUNIT_ASSERT(aPoint==dummy1.GetOrigin());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkBaseGeometry)
