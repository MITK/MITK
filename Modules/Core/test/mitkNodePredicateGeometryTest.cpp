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
#include "mitkBaseDataTestImplementation.h"
#include "mitkNodePredicateGeometry.h"
#include "mitkDataNode.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
#include "mitkProportionalTimeGeometry.h"


class mitkNodePredicateGeometryTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNodePredicateGeometryTestSuite);
  MITK_TEST(Check_InvalidConstructor);
  MITK_TEST(Check_CloneAndOriginal);
  MITK_TEST(Check_DifferentOrigin);
  MITK_TEST(Check_DifferentIndexToWorldTransform);
  MITK_TEST(Check_DifferentSpacing);
  MITK_TEST(Check_DifferentBoundingBox);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::BaseDataTestImplementation::Pointer m_Data;
  mitk::DataNode::Pointer m_Node;

  mitk::Geometry3D::Pointer m_RefGeometry;
  mitk::TimeGeometry::Pointer m_RefTimeGeometry;

  mitk::Geometry3D::Pointer m_AnotherGeometry3D;

public:
  /**
* @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members
* for a new test case. (If the members are not used in a test, the method does not need to be called).
*/
  void setUp() override
  {
    m_RefGeometry = mitk::Geometry3D::New();
    m_RefGeometry->Initialize();

    mitk::ProportionalTimeGeometry::Pointer tgeo = mitk::ProportionalTimeGeometry::New();
    tgeo->Initialize(m_RefGeometry, 1);

    m_RefTimeGeometry = tgeo.GetPointer();

    m_Data = mitk::BaseDataTestImplementation::New();
    m_Data->SetClonedGeometry(m_RefGeometry);

    m_Node = mitk::DataNode::New();
    m_Node->SetData(m_Data);

    m_AnotherGeometry3D = m_RefGeometry->Clone();
  }

  void tearDown() override
  {
    m_RefGeometry = nullptr;
    m_RefTimeGeometry = nullptr;
    m_AnotherGeometry3D = nullptr;
    m_Data = nullptr;
  }

  void Check_InvalidConstructor()
  {
    m_RefGeometry = nullptr;
    m_RefTimeGeometry = nullptr;
    CPPUNIT_ASSERT_THROW(mitk::NodePredicateGeometry::New(m_RefGeometry, 3), mitk::Exception);
    CPPUNIT_ASSERT_THROW(mitk::NodePredicateGeometry::New(m_RefGeometry), mitk::Exception);
    CPPUNIT_ASSERT_THROW(mitk::NodePredicateGeometry::New(m_RefTimeGeometry), mitk::Exception);
  }

  void Check_CloneAndOriginal()
  {
    mitk::NodePredicateGeometry::Pointer predicate = mitk::NodePredicateGeometry::New(m_RefGeometry);
    mitk::NodePredicateGeometry::Pointer predicateTime = mitk::NodePredicateGeometry::New(m_RefTimeGeometry);

    CPPUNIT_ASSERT(predicate->CheckNode(m_Node));
    CPPUNIT_ASSERT(predicateTime->CheckNode(m_Node));
  }

  void Check_DifferentOrigin()
  {
    mitk::NodePredicateGeometry::Pointer predicate = mitk::NodePredicateGeometry::New(m_RefGeometry);
    mitk::NodePredicateGeometry::Pointer predicateTime = mitk::NodePredicateGeometry::New(m_RefTimeGeometry);

    mitk::Point3D origin;
    origin[0] = 0.0;
    origin[1] = 0.0;
    origin[2] = 1.0;
    m_AnotherGeometry3D->SetOrigin(origin);
    m_Data->SetClonedGeometry(m_AnotherGeometry3D);

    CPPUNIT_ASSERT(!predicate->CheckNode(m_Node));
    CPPUNIT_ASSERT(!predicateTime->CheckNode(m_Node));

    predicate = mitk::NodePredicateGeometry::New(m_AnotherGeometry3D);
    CPPUNIT_ASSERT(predicate->CheckNode(m_Node));
  }

  void Check_DifferentIndexToWorldTransform()
  {
    mitk::NodePredicateGeometry::Pointer predicate = mitk::NodePredicateGeometry::New(m_RefGeometry);
    mitk::NodePredicateGeometry::Pointer predicateTime = mitk::NodePredicateGeometry::New(m_RefTimeGeometry);

    mitk::AffineTransform3D::Pointer differentIndexToWorldTransform = mitk::AffineTransform3D::New();

    mitk::AffineTransform3D::MatrixType differentMatrix;
    differentMatrix.SetIdentity();
    differentMatrix(1, 1) = 2;

    differentIndexToWorldTransform->SetMatrix(differentMatrix);
    m_AnotherGeometry3D->SetIndexToWorldTransform(differentIndexToWorldTransform);
    m_Data->SetClonedGeometry(m_AnotherGeometry3D);

    CPPUNIT_ASSERT(!predicate->CheckNode(m_Node));
    CPPUNIT_ASSERT(!predicateTime->CheckNode(m_Node));

    predicate = mitk::NodePredicateGeometry::New(m_AnotherGeometry3D);
    CPPUNIT_ASSERT(predicate->CheckNode(m_Node));
  }

  void Check_DifferentSpacing()
  {
    mitk::NodePredicateGeometry::Pointer predicate = mitk::NodePredicateGeometry::New(m_RefGeometry);
    mitk::NodePredicateGeometry::Pointer predicateTime = mitk::NodePredicateGeometry::New(m_RefTimeGeometry);

    mitk::Vector3D differentSpacing;
    differentSpacing[0] = 1.0;
    differentSpacing[1] = 2.0;
    differentSpacing[2] = 3.0+3*mitk::eps;

    m_AnotherGeometry3D->SetSpacing(differentSpacing);
    m_Data->SetClonedGeometry(m_AnotherGeometry3D);

    CPPUNIT_ASSERT(!predicate->CheckNode(m_Node));
    CPPUNIT_ASSERT(!predicateTime->CheckNode(m_Node));

    //check with altered geometry as reference (sanity check).
    mitk::NodePredicateGeometry::Pointer predicate2 = mitk::NodePredicateGeometry::New(m_AnotherGeometry3D);
    CPPUNIT_ASSERT(predicate2->CheckNode(m_Node));

    //check less strict precission checkings
    differentSpacing[0] = 1.0;
    differentSpacing[1] = 1.0;
    differentSpacing[2] = 1.0 + 3 * mitk::eps;

    m_AnotherGeometry3D->SetSpacing(differentSpacing);
    m_Data->SetClonedGeometry(m_AnotherGeometry3D);

    CPPUNIT_ASSERT(!predicate->CheckNode(m_Node));
    CPPUNIT_ASSERT(!predicateTime->CheckNode(m_Node));

    predicate->SetCheckPrecision(1e-3);
    predicateTime->SetCheckPrecision(1e-3);

    CPPUNIT_ASSERT(predicate->CheckNode(m_Node));
    CPPUNIT_ASSERT(predicateTime->CheckNode(m_Node));
  }

  void Check_DifferentBoundingBox()
  {
    mitk::NodePredicateGeometry::Pointer predicate = mitk::NodePredicateGeometry::New(m_RefGeometry);
    mitk::NodePredicateGeometry::Pointer predicateTime = mitk::NodePredicateGeometry::New(m_RefTimeGeometry);

    mitk::ScalarType bounds[] = { 0.0, 0.0, 0.0, 1.0, 2.0, 3.0 };
    m_AnotherGeometry3D->SetBounds(bounds);
    m_Data->SetClonedGeometry(m_AnotherGeometry3D);

    CPPUNIT_ASSERT(!predicate->CheckNode(m_Node));
    CPPUNIT_ASSERT(!predicateTime->CheckNode(m_Node));

    predicate = mitk::NodePredicateGeometry::New(m_AnotherGeometry3D);
    CPPUNIT_ASSERT(predicate->CheckNode(m_Node));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkNodePredicateGeometry)
