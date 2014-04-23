/*
 * mitkPointSetTestOnEmpty.cpp
 *
 *  Created on: Apr 23, 2014
 *      Author: wirkert
 */
/**
 * TestSuite for PointSet which tests properties on an empty PointSet
 */

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"


#include <mitkPointSet.h>
#include <mitkVector.h>
#include <mitkPointOperation.h>
#include <mitkInteractionConst.h>

#include <fstream>

class mitkPointSetOnEmptyTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkPointSetOnEmptyTestSuite);

  MITK_TEST(TestInstantiation);
  MITK_TEST(TestIsEmpty);
  MITK_TEST(TestGetITKPointSet);
  MITK_TEST(TestGetSizeIsZero);
  MITK_TEST(TestAddPointDirectly);
  MITK_TEST(TestPointSetClone);
  MITK_TEST(TestPointDataContainerCorrectAfterPointSetManipulation);

  CPPUNIT_TEST_SUITE_END();

private:

  mitk::PointSet::Pointer pointSet;

  void TestPointContainerPointDataContainer(mitk::PointSet* ps)
  {
    mitk::PointSet::PointsContainer* pc = ps->GetPointSet()->GetPoints();
    mitk::PointSet::PointDataContainer* pd = ps->GetPointSet()->GetPointData();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("PointContainer and PointDataContainer have same size",
        true, pc->Size() == pd->Size());
    mitk::PointSet::PointsContainer::ConstIterator pIt = pc->Begin();
    mitk::PointSet::PointDataContainer::ConstIterator dIt = pd->Begin();
    bool failed = false;
    for (; pIt != pc->End(); ++pIt, ++dIt)
      if (pIt->Index() != dIt->Index())
      {
        failed = true;
        break;
      }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Indices in PointContainer and PointDataContainer are equal",
        false, failed);
  }

public:

  void setUp()
  {
    //Create PointSet
    pointSet = mitk::PointSet::New();
  }

  void tearDown()
  {
    pointSet = NULL;
  }

  void TestInstantiation()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing instantiation",
        true, pointSet.IsNotNull());
  }

  void TestGetITKPointSet()
  {
    //try to get the itkPointSet
    mitk::PointSet::DataType::Pointer itkdata = NULL;
    itkdata = pointSet->GetPointSet();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("try to get the itkPointSet from a newly created PointSet",
        true,  itkdata.IsNotNull());
  }

  void TestGetSizeIsZero()
  {
    //fresh PointSet has to be empty!
    CPPUNIT_ASSERT_EQUAL_MESSAGE("check if the PointSet size is 0 ",
        true, pointSet->GetSize() == 0);
  }

  void TestIsEmpty()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("check if the PointSet is empty",
        true, pointSet->IsEmptyTimeStep(0));
  }

  void TestAddPointDirectly()
  {
    //add a point directly
    int id=0;
    mitk::Point3D point;
    mitk::FillVector3D(point, 1.0, 2.0, 3.0);
    ++id;
    pointSet->GetPointSet()->GetPoints()->InsertElement(id, point);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check if added points exists",
        true, pointSet->GetSize()==1 ||pointSet->IndexExists(id));

    mitk::Point3D tempPoint;

    tempPoint.Fill(0);
    tempPoint = pointSet->GetPoint(id);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check if added point contains real value",
        true, point == tempPoint);
  }

  void TestPointSetClone(void)
  {
    pointSet->Expand(3);
    mitk::Point3D new0, new1, new2;
    new0.Fill(0);
    new1.Fill(1);
    new2.Fill(2);
    pointSet->InsertPoint(5,new0, mitk::PTCORNER,0);
    pointSet->InsertPoint(112,new1,0);
    pointSet->InsertPoint(2,new2,0);
    pointSet->InsertPoint(2,new0,1);
    pointSet->InsertPoint(1,new1,1);
    pointSet->InsertPoint(0,new2,1);
    pointSet->InsertPoint(0,new0,2);
    pointSet->InsertPoint(2,new1,2);
    pointSet->InsertPoint(1,new2,2);

    MITK_TEST_OUTPUT( << "... pointset ts: " << pointSet->GetTimeSteps() )
    mitk::PointSet::Pointer clonePS = pointSet->Clone();
    MITK_TEST_OUTPUT( << "... clone pointset ts: " << clonePS->GetTimeSteps() )

    for (unsigned int t=0; t< pointSet->GetTimeSteps(); t++)
      {
        MITK_TEST_OUTPUT( << "testing timestep: " << t )
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Clone has same size",
            true, pointSet->GetSize(t) == clonePS->GetSize(t));

          // test for equal point coordinates
          for (mitk::PointSet::PointsConstIterator i = pointSet->Begin(), j = clonePS->Begin();
              i != pointSet->End() && j != clonePS->End(); ++i, ++j)
            {
              CPPUNIT_ASSERT_EQUAL_MESSAGE("Cloned PS and PS have same points",
                  true, i.Index() == j.Index() && mitk::Equal(i.Value(),j.Value()));
            }

        // test for equal point data
        mitk::PointSet::PointDataContainer* pointDataCont = pointSet->GetPointSet(t)->GetPointData();
        mitk::PointSet::PointDataContainer* clonePointDataCont = clonePS->GetPointSet(t)->GetPointData();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Valid point data container",
            true, pointDataCont && clonePointDataCont);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Cloned point data container has same size",
            true, pointDataCont->Size() == clonePointDataCont->Size());
        for (mitk::PointSet::PointDataConstIterator i = pointDataCont->Begin(), j = clonePointDataCont->Begin();
            i != pointDataCont->End() && j != clonePointDataCont->End(); ++i, ++j)
          {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Cloned PS and PS have same point data",
                true, i.Index() == j.Index() && i.Value() == j.Value());
          }
      }

    CPPUNIT_ASSERT_EQUAL_MESSAGE("check if the PointSet is not empty",
        false, clonePS->IsEmptyTimeStep(0));

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing cloned point set's size!",
        true, clonePS->GetPointSetSeriesSize() == pointSet->GetPointSetSeriesSize());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing that the clone is not the source PS!",
        true, clonePS.GetPointer() != pointSet.GetPointer());
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "Testing if the geometry is cloned correctly!",
        true, clonePS->GetGeometry()->GetCenter() == pointSet->GetGeometry()->GetCenter());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing if the property list is cloned correctly!",
        true, clonePS->GetPropertyList()->GetMap()->size() == pointSet->GetPropertyList()->GetMap()->size());
    // Also testing, that clone is independent from original
    mitk::Point3D p, p2;
    p.Fill(42);
    p2.Fill(84);
    clonePS->InsertPoint(0,p);
    pointSet->InsertPoint(0,p2);
    p = clonePS->GetPoint(0);
    p2 = pointSet->GetPoint(0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing that the clone is independent from source!",
        true, p != p2);

  }


  void TestPointDataContainerCorrectAfterPointSetManipulation(void)
  {
    /* call everything that might modify PointContainer and PointDataContainer */
    MITK_TEST_OUTPUT(<< "Test InsertPoint(), SetPoint() and SwapPointPosition()");
    mitk::PointSet::PointType point;
    mitk::FillVector3D(point, 2.2, 3.3, -4.4);
    pointSet->InsertPoint(17, point);
    pointSet->SetPoint(4, point);
    pointSet->SetPoint(7, point);
    pointSet->SetPoint(2, point);
    pointSet->SwapPointPosition(7, true);
    pointSet->SwapPointPosition(3, true);
    pointSet->SwapPointPosition(2, false);
    TestPointContainerPointDataContainer(pointSet);

    MITK_TEST_OUTPUT(<< "Test OpREMOVE");
    mitk::PointOperation op1(mitk::OpREMOVE, mitk::Point3D(), 2); // existing index
    pointSet->ExecuteOperation(&op1);
    mitk::PointOperation op1b(mitk::OpREMOVE, mitk::Point3D(), 112); // non existing index
    pointSet->ExecuteOperation(&op1b);
    TestPointContainerPointDataContainer(pointSet);

    MITK_TEST_OUTPUT(<< "Test OpMove");
    mitk::PointOperation op2(mitk::OpMOVE, mitk::Point3D(), 4);  // existing index
    pointSet->ExecuteOperation(&op2);
    mitk::PointOperation op3(mitk::OpMOVE, mitk::Point3D(), 34);  // non existing index
    pointSet->ExecuteOperation(&op3);
    TestPointContainerPointDataContainer(pointSet);

    MITK_TEST_OUTPUT(<< "Test OpINSERT");
    mitk::PointOperation op4(mitk::OpINSERT, mitk::Point3D(), 38);  // non existing index
    pointSet->ExecuteOperation(&op4);
    mitk::PointOperation op5(mitk::OpINSERT, mitk::Point3D(), 17);  // existing index
    pointSet->ExecuteOperation(&op5);
    TestPointContainerPointDataContainer(pointSet);
  }




};

MITK_TEST_SUITE_REGISTRATION(mitkPointSetOnEmpty)





