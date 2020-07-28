/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGeometry3D.h"
#include "mitkBaseDataTestImplementation.h"
#include "mitkNodePredicateSubGeometry.h"
#include "mitkNodePredicateGeometry.h"
#include "mitkDataNode.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

class mitkNodePredicateSubGeometryTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNodePredicateSubGeometryTestSuite);
  MITK_TEST(Check_InvalidConstructor);
  MITK_TEST(Check_Spacing);
  MITK_TEST(Check_TransformMatrix);
  MITK_TEST(Check_Bounds);
  MITK_TEST(Check_Grid);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::BaseDataTestImplementation::Pointer m_Data;
  mitk::DataNode::Pointer m_Node;

  mitk::Geometry3D::Pointer m_RefGeometry;

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

    m_Data = mitk::BaseDataTestImplementation::New();
    m_Data->SetClonedGeometry(m_RefGeometry);

    m_Node = mitk::DataNode::New();
    m_Node->SetData(m_Data);

    m_AnotherGeometry3D = m_RefGeometry->Clone();
  }

  void tearDown() override
  {
    m_RefGeometry = nullptr;
    m_AnotherGeometry3D = nullptr;
    m_Data = nullptr;
  }

  void Check_InvalidConstructor()
  {
    m_RefGeometry = nullptr;
    CPPUNIT_ASSERT_THROW(mitk::NodePredicateSubGeometry::New(m_RefGeometry, 3), mitk::Exception);
    CPPUNIT_ASSERT_THROW(mitk::NodePredicateSubGeometry::New(m_RefGeometry), mitk::Exception);
  }

  void Check_CloneAndOriginal()
  {
    mitk::NodePredicateSubGeometry::Pointer predicate = mitk::NodePredicateSubGeometry::New(m_RefGeometry);

    CPPUNIT_ASSERT(predicate->CheckNode(m_Node));
  }

  void Check_Spacing()
  {
    mitk::NodePredicateSubGeometry::Pointer predicate = mitk::NodePredicateSubGeometry::New(m_RefGeometry);

    for (unsigned int i = 0; i < 3; ++i)
    {
      mitk::Vector3D wrongSpacing = m_RefGeometry->GetSpacing();
      wrongSpacing[i] += mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_PRECISION * 2;
      auto wrongGeometry = m_RefGeometry->Clone();
      wrongGeometry->SetSpacing(wrongSpacing);
      m_Node->GetData()->SetGeometry(wrongGeometry);

      CPPUNIT_ASSERT(!predicate->CheckNode(m_Node));
    }
    for (unsigned int i = 0; i < 3; ++i)
    {
      mitk::Vector3D wrongSpacing = m_RefGeometry->GetSpacing();
      wrongSpacing[i] -= mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_PRECISION * 2;
      auto wrongGeometry = m_RefGeometry->Clone();
      wrongGeometry->SetSpacing(wrongSpacing);
      m_Node->GetData()->SetGeometry(wrongGeometry);

      CPPUNIT_ASSERT(!predicate->CheckNode(m_Node));
    }
  }

  void Check_TransformMatrix()
  {
    mitk::NodePredicateSubGeometry::Pointer predicate = mitk::NodePredicateSubGeometry::New(m_RefGeometry);

    for (unsigned int i = 0; i < 3; ++i)
    {
      for (unsigned int j = 0; j < 3; ++j)
      {
        itk::Matrix<mitk::ScalarType, 3, 3> wrongMatrix = m_RefGeometry->GetIndexToWorldTransform()->GetMatrix();
        wrongMatrix[i][j] += mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_PRECISION * 2;
        auto wrongGeometry = m_RefGeometry->Clone();
        wrongGeometry->GetIndexToWorldTransform()->SetMatrix(wrongMatrix);
        m_Node->GetData()->SetGeometry(wrongGeometry);

        CPPUNIT_ASSERT(!predicate->CheckNode(m_Node));
      }
    }
  }

  void Check_Bounds()
  {
    auto newBounds = m_RefGeometry->GetBounds();
    newBounds[0] = 10;
    newBounds[1] = 20;
    newBounds[2] = 10;
    newBounds[3] = 20;
    newBounds[4] = 10;
    newBounds[5] = 20;
    m_RefGeometry->SetBounds(newBounds);
    mitk::NodePredicateSubGeometry::Pointer predicate = mitk::NodePredicateSubGeometry::New(m_RefGeometry);

    for (unsigned int i = 0; i < 6; ++i)
    {
      auto legalBounds = newBounds;
      if (i % 2 == 0)
      {
        legalBounds[i] += 1;
      }
      else
      {
        legalBounds[i] -= 1;
      }
      auto legalGeometry = m_RefGeometry->Clone();
      legalGeometry->SetBounds(legalBounds);
      m_Node->GetData()->SetGeometry(legalGeometry);

      CPPUNIT_ASSERT(predicate->CheckNode(m_Node));
    }

    for (unsigned int i = 0; i < 6; ++i)
    {
      auto wrongBounds = m_RefGeometry->GetBounds();
      if (i % 2 == 0)
      {
        wrongBounds[i] -= 1;
      }
      else
      {
        wrongBounds[i] += 1;
      }
      auto wrongGeometry = m_RefGeometry->Clone();
      wrongGeometry->SetBounds(wrongBounds);
      m_Node->GetData()->SetGeometry(wrongGeometry);

      CPPUNIT_ASSERT(!predicate->CheckNode(m_Node));
    }
  }

  void Check_Grid()
  {
    auto newBounds = m_RefGeometry->GetBounds();
    newBounds[0] = 0;
    newBounds[1] = 20;
    newBounds[2] = 0;
    newBounds[3] = 20;
    newBounds[4] = 0;
    newBounds[5] = 20;
    m_RefGeometry->SetBounds(newBounds);
    mitk::NodePredicateSubGeometry::Pointer predicate = mitk::NodePredicateSubGeometry::New(m_RefGeometry);

    auto smallerGeometry = m_RefGeometry->Clone();
    newBounds[0] = 5;
    newBounds[1] = 10;
    newBounds[2] = 5;
    newBounds[3] = 10;
    newBounds[4] = 5;
    newBounds[5] = 10;
    smallerGeometry->SetBounds(newBounds);

    //legal negative shift
    for (unsigned int i = 0; i < 3; ++i)
    {
      auto legalOrigin = smallerGeometry->GetOrigin();
      legalOrigin[i] -= smallerGeometry->GetSpacing()[i];
      auto legalGeometry = smallerGeometry->Clone();
      legalGeometry->SetOrigin(legalOrigin);
      m_Node->GetData()->SetGeometry(legalGeometry);

      CPPUNIT_ASSERT(predicate->CheckNode(m_Node));
    }

    //legal positive shift
    for (unsigned int i = 0; i < 3; ++i)
    {
      auto legalOrigin = smallerGeometry->GetOrigin();
      legalOrigin[i] += smallerGeometry->GetSpacing()[i];
      auto legalGeometry = smallerGeometry->Clone();
      legalGeometry->SetOrigin(legalOrigin);
      m_Node->GetData()->SetGeometry(legalGeometry);

      CPPUNIT_ASSERT(predicate->CheckNode(m_Node));
    }

    //wrong negative shift
    for (unsigned int i = 0; i < 3; ++i)
    {
      auto wrongOrigin = smallerGeometry->GetOrigin();
      wrongOrigin[i] -= 2 * mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_PRECISION;
      auto wrongGeometry = smallerGeometry->Clone();
      wrongGeometry->SetOrigin(wrongOrigin);
      m_Node->GetData()->SetGeometry(wrongGeometry);

      CPPUNIT_ASSERT(!predicate->CheckNode(m_Node));
    }

    //wrong positive shift
    for (unsigned int i = 0; i < 3; ++i)
    {
      auto wrongOrigin = smallerGeometry->GetOrigin();
      wrongOrigin[i] += 2 * mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_PRECISION;
      auto wrongGeometry = smallerGeometry->Clone();
      wrongGeometry->SetOrigin(wrongOrigin);
      m_Node->GetData()->SetGeometry(wrongGeometry);

      CPPUNIT_ASSERT(!predicate->CheckNode(m_Node));
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkNodePredicateSubGeometry)
