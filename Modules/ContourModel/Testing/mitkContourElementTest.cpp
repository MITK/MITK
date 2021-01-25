/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourElement.h"

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
#include <limits>

class mitkContourElementTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkContourElementTestSuite);
  // Test the append method
  MITK_TEST(Iterator);
  MITK_TEST(AddVertex);
  MITK_TEST(AddVertexAtFront);
  MITK_TEST(InsertVertexAtIndex);
  MITK_TEST(GetSetVertexAt);
  MITK_TEST(OpenAndClose);
  MITK_TEST(OtherGetters);
  MITK_TEST(Concatenate);
  MITK_TEST(RemoveVertex);
  MITK_TEST(Clear);
  MITK_TEST(GetControlVertices);
  MITK_TEST(RedistributeControlVertices);
  MITK_TEST(Others);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::ContourElement::Pointer m_Contour1to4;
  mitk::ContourElement::Pointer m_Contour5to6;
  mitk::ContourElement::Pointer m_Contour_empty;
  mitk::Point3D m_p1;
  mitk::Point3D m_p2;
  mitk::Point3D m_p3;
  mitk::Point3D m_p4;
  mitk::Point3D m_p5;
  mitk::Point3D m_p6;
  mitk::Point3D m_p7;

public:
  static mitk::Point3D GeneratePoint(double val)
  {
    return mitk::Point3D(val);
  }

  void setUp() override
  {
    m_p1 = GeneratePoint(1);
    m_p2 = GeneratePoint(2);
    m_p3 = GeneratePoint(3);
    m_p4 = GeneratePoint(4);
    m_p5 = GeneratePoint(5);
    m_p6 = GeneratePoint(6);
    m_p7 = GeneratePoint(7);

    m_Contour1to4 = mitk::ContourElement::New();
    m_Contour5to6 = mitk::ContourElement::New();
    m_Contour_empty = mitk::ContourElement::New();

    m_Contour1to4->AddVertex(m_p1, true);
    m_Contour1to4->AddVertex(m_p2, false);
    m_Contour1to4->AddVertex(m_p3, true);
    m_Contour1to4->AddVertex(m_p4, false);

    m_Contour5to6->AddVertex(m_p5, false);
    m_Contour5to6->AddVertex(m_p6, true);

  }

  void tearDown() override {}

  void Iterator()
  {
    mitk::ContourElement::ConstPointer constcontour = m_Contour5to6.GetPointer();
    CPPUNIT_ASSERT_MESSAGE("Begin does not point to correct element", m_Contour1to4->IteratorBegin().operator*()->Coordinates == m_p1);
    CPPUNIT_ASSERT_MESSAGE("Begin does not point to correct element", m_Contour1to4->ConstIteratorBegin().operator*()->Coordinates == m_p1);
    CPPUNIT_ASSERT_MESSAGE("Begin does not point to correct element", m_Contour5to6->begin().operator*()->Coordinates == m_p5);
    CPPUNIT_ASSERT_MESSAGE("Begin does not point to correct element", constcontour->begin().operator*()->Coordinates == m_p5);

    CPPUNIT_ASSERT_MESSAGE("End does not point to correct element", m_Contour_empty->ConstIteratorBegin() == m_Contour_empty->ConstIteratorEnd());
    CPPUNIT_ASSERT_MESSAGE("End does not point to correct element", m_Contour_empty->IteratorBegin() == m_Contour_empty->IteratorEnd());
    CPPUNIT_ASSERT_MESSAGE("End does not point to correct element", m_Contour_empty->begin() == m_Contour_empty->end());
  }

  void AddVertex()
  {
    m_Contour_empty->AddVertex(m_p1, false);
    CPPUNIT_ASSERT(m_Contour_empty->GetVertexAt(0)->Coordinates == m_p1);
    CPPUNIT_ASSERT(m_Contour_empty->GetVertexAt(0)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour_empty->GetSize() == 1);

    m_Contour_empty->AddVertex(m_p7, true);
    CPPUNIT_ASSERT(m_Contour_empty->GetVertexAt(0)->Coordinates == m_p1);
    CPPUNIT_ASSERT(m_Contour_empty->GetVertexAt(0)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour_empty->GetVertexAt(1)->Coordinates == m_p7);
    CPPUNIT_ASSERT(m_Contour_empty->GetVertexAt(1)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour_empty->GetSize() == 2);
  }

  void AddVertexAtFront()
  {
    m_Contour_empty->AddVertexAtFront(m_p1, false);
    CPPUNIT_ASSERT(m_Contour_empty->GetVertexAt(0)->Coordinates == m_p1);
    CPPUNIT_ASSERT(m_Contour_empty->GetVertexAt(0)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour_empty->GetSize() == 1);

    m_Contour_empty->AddVertexAtFront(m_p7, true);
    CPPUNIT_ASSERT(m_Contour_empty->GetVertexAt(1)->Coordinates == m_p1);
    CPPUNIT_ASSERT(m_Contour_empty->GetVertexAt(1)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour_empty->GetVertexAt(0)->Coordinates == m_p7);
    CPPUNIT_ASSERT(m_Contour_empty->GetVertexAt(0)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour_empty->GetSize() == 2);
  }

  void InsertVertexAtIndex()
  {
    mitk::Point3D outOfBountPoint;
    m_Contour1to4->InsertVertexAtIndex(m_p5, false, 0);
    m_Contour1to4->InsertVertexAtIndex(m_p7, true, 2);
    m_Contour1to4->InsertVertexAtIndex(outOfBountPoint, true, 6);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(0)->Coordinates == m_p5);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(0)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(1)->Coordinates == m_p1);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(1)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(2)->Coordinates == m_p7);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(2)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(3)->Coordinates == m_p2);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(3)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetSize() == 6);
  }

  void GetSetVertexAt()
  {
    mitk::ContourElement::ConstPointer constcontour = m_Contour1to4.GetPointer();

    auto v0 = m_Contour1to4->GetVertexAt(0);
    auto v1 = m_Contour1to4->GetVertexAt(1);

    CPPUNIT_ASSERT(v0->Coordinates == m_p1);
    CPPUNIT_ASSERT(v0->IsControlPoint == true);
    CPPUNIT_ASSERT(v1->Coordinates == m_p2);
    CPPUNIT_ASSERT(v1->IsControlPoint == false);
    CPPUNIT_ASSERT(constcontour->GetVertexAt(0)->Coordinates == m_p1);
    CPPUNIT_ASSERT(constcontour->GetVertexAt(0)->IsControlPoint == true);
    CPPUNIT_ASSERT(constcontour->GetVertexAt(1)->Coordinates == m_p2);
    CPPUNIT_ASSERT(constcontour->GetVertexAt(1)->IsControlPoint == false);

    m_Contour1to4->SetVertexAt(0, m_p7);
    CPPUNIT_ASSERT(v0->Coordinates == m_p7);
    CPPUNIT_ASSERT(v0->IsControlPoint == true);

    m_Contour1to4->SetVertexAt(1, m_Contour5to6->GetVertexAt(1));
    CPPUNIT_ASSERT(v1->Coordinates == m_Contour5to6->GetVertexAt(1)->Coordinates);
    CPPUNIT_ASSERT(v1->IsControlPoint == m_Contour5to6->GetVertexAt(1)->IsControlPoint);
    CPPUNIT_ASSERT(v1 != m_Contour5to6->GetVertexAt(1));

    mitk::Point3D search = GeneratePoint(6.05);

    auto finding = m_Contour1to4->GetVertexAt(search, 0.);
    CPPUNIT_ASSERT(nullptr == finding);

    finding = m_Contour1to4->GetVertexAt(search, 0.1);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(1) == finding);
  }

  void OpenAndClose()
  {
    CPPUNIT_ASSERT(!m_Contour1to4->IsClosed());
    CPPUNIT_ASSERT(!m_Contour_empty->IsClosed());
    m_Contour1to4->Close();
    m_Contour_empty->SetClosed(true);
    CPPUNIT_ASSERT(m_Contour1to4->IsClosed());
    CPPUNIT_ASSERT(m_Contour_empty->IsClosed());
    m_Contour1to4->Open();
    m_Contour_empty->SetClosed(false);
    CPPUNIT_ASSERT(!m_Contour1to4->IsClosed());
    CPPUNIT_ASSERT(!m_Contour_empty->IsClosed());
  }

  void OtherGetters()
  {
    CPPUNIT_ASSERT(m_Contour1to4->GetIndex(m_Contour1to4->GetVertexAt(0)) == 0);
    CPPUNIT_ASSERT(m_Contour1to4->GetIndex(m_Contour1to4->GetVertexAt(1)) == 1);
    CPPUNIT_ASSERT(m_Contour1to4->GetIndex(m_Contour1to4->GetVertexAt(3)) == 3);
    CPPUNIT_ASSERT(m_Contour1to4->GetIndex(m_Contour5to6->GetVertexAt(0)) == mitk::ContourElement::NPOS);
  }

  void Concatenate()
  {
    m_Contour5to6->Concatenate(m_Contour1to4, true);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(0)->Coordinates == m_p5);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(0)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(1)->Coordinates == m_p6);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(1)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(2)->Coordinates == m_p1);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(2)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(3)->Coordinates == m_p2);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(3)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(4)->Coordinates == m_p3);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(4)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(5)->Coordinates == m_p4);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(5)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour5to6->GetSize() == 6);

    m_Contour_empty->AddVertex(m_p1, false);
    m_Contour5to6->Concatenate(m_Contour_empty, true);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(0)->Coordinates == m_p5);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(0)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(1)->Coordinates == m_p6);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(1)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(2)->Coordinates == m_p1);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(2)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(3)->Coordinates == m_p2);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(3)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(4)->Coordinates == m_p3);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(4)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(5)->Coordinates == m_p4);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(5)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour5to6->GetSize() == 6);

    m_Contour5to6->Concatenate(m_Contour_empty, false);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(0)->Coordinates == m_p5);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(0)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(1)->Coordinates == m_p6);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(1)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(2)->Coordinates == m_p1);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(2)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(3)->Coordinates == m_p2);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(3)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(4)->Coordinates == m_p3);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(4)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(5)->Coordinates == m_p4);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(5)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(6)->Coordinates == m_p1);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(6)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour5to6->GetSize() == 7);
  }
  
  void RemoveVertex()
  {
    CPPUNIT_ASSERT(m_Contour1to4->RemoveVertex(m_Contour1to4->GetVertexAt(0)));
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(0)->Coordinates == m_p2);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(0)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetSize() == 3);
    CPPUNIT_ASSERT(!m_Contour1to4->RemoveVertex(m_Contour5to6->GetVertexAt(0)));
    CPPUNIT_ASSERT(m_Contour1to4->GetSize() == 3);

    CPPUNIT_ASSERT(m_Contour1to4->RemoveVertexAt(1));
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(0)->Coordinates == m_p2);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(0)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(1)->Coordinates == m_p4);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(1)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetSize() == 2);

    CPPUNIT_ASSERT(!m_Contour1to4->RemoveVertexAt(2));
    CPPUNIT_ASSERT(m_Contour1to4->GetSize() == 2);

    mitk::Point3D search = GeneratePoint(6.05);

    CPPUNIT_ASSERT(!m_Contour1to4->RemoveVertexAt(search,0.1));
    CPPUNIT_ASSERT(m_Contour1to4->GetSize() == 2);

    CPPUNIT_ASSERT(m_Contour5to6->RemoveVertexAt(search, 0.1));
    CPPUNIT_ASSERT(m_Contour5to6->GetSize() == 1);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(0)->Coordinates == m_p5);
    CPPUNIT_ASSERT(m_Contour5to6->GetVertexAt(0)->IsControlPoint == false);
  }

  void Clear()
  {
    m_Contour1to4->Clear();
    CPPUNIT_ASSERT(m_Contour1to4->IsEmpty());
    m_Contour_empty->Clear();
    CPPUNIT_ASSERT(m_Contour_empty->IsEmpty());
  }
  
  void GetControlVertices()
  {
    auto controlVs = m_Contour1to4->GetControlVertices();
    CPPUNIT_ASSERT(controlVs.size() == 2);
    CPPUNIT_ASSERT(controlVs[0] == m_Contour1to4->GetVertexAt(0));
    CPPUNIT_ASSERT(controlVs[1] == m_Contour1to4->GetVertexAt(2));

    controlVs = m_Contour_empty->GetControlVertices();
    CPPUNIT_ASSERT(controlVs.empty());
  }
  
  void RedistributeControlVertices()
  {
    //just adding more nodes to better check the redistribution
    m_Contour1to4->Concatenate(m_Contour1to4, false);
    m_Contour1to4->Concatenate(m_Contour1to4, false);

    m_Contour1to4->RedistributeControlVertices(nullptr, 3);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(0)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(1)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(2)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(3)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(4)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(5)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(6)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(7)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(8)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(9)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(10)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(11)->IsControlPoint == false);

    m_Contour1to4->RedistributeControlVertices(m_Contour1to4->GetVertexAt(4), 4);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(0)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(1)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(2)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(3)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(4)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(5)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(6)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(7)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(8)->IsControlPoint == true);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(9)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(10)->IsControlPoint == false);
    CPPUNIT_ASSERT(m_Contour1to4->GetVertexAt(11)->IsControlPoint == false);
  }
  
  void Others()
  {
    CPPUNIT_ASSERT(m_Contour1to4->GetSize() == 4);
    CPPUNIT_ASSERT(m_Contour_empty->GetSize() == 0);
    CPPUNIT_ASSERT(!m_Contour1to4->IsEmpty());
    CPPUNIT_ASSERT(m_Contour_empty->IsEmpty());

    mitk::ContourElement::Pointer copyConstructed = m_Contour5to6->Clone();
    CPPUNIT_ASSERT(*(m_Contour5to6->GetVertexAt(0)) == *(copyConstructed->GetVertexAt(0)));
    CPPUNIT_ASSERT(*(m_Contour5to6->GetVertexAt(1)) == *(copyConstructed->GetVertexAt(1)));
    CPPUNIT_ASSERT(m_Contour5to6->GetSize() == copyConstructed->GetSize());
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkContourElement)
