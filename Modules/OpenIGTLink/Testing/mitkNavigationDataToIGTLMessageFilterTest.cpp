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

#include "mitkNavigationDataToPointSetFilter.h"
#include "mitkNavigationDataSequentialPlayer.h"
#include "mitkNavigationDataReaderXML.h"
#include <mitkIGTConfig.h>
#include <mitkTestingMacros.h>

#include <iostream>

/**
*  Simple example for a test for the (non-existent) class "NavigationDataToPointSetFilter".
*
*  argc and argv are the command line parameters which were passed to
*  the ADD_TEST command in the CMakeLists.txt file. For the automatic
*  tests, argv is either empty for the simple tests or contains the filename
*  of a test image for the image tests (see CMakeLists.txt).
*/

mitk::NavigationDataToPointSetFilter::Pointer m_NavigationDataToPointSetFilter;

static void Setup()
{
  m_NavigationDataToPointSetFilter = mitk::NavigationDataToPointSetFilter::New();
}

static void TestMode3D()
{
  Setup();
  m_NavigationDataToPointSetFilter->SetOperationMode(mitk::NavigationDataToPointSetFilter::Mode3D);

  //Build up test data
  mitk::NavigationData::Pointer nd0 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd2 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd3 = mitk::NavigationData::New();

  mitk::NavigationData::PositionType point0;
  point0[0] = 1.0;
  point0[1] = 2.0;
  point0[2] = 3.0;
  nd0->SetPosition(point0);
  nd0->SetDataValid(true);

  mitk::NavigationData::PositionType point1;
  point1[0] = 4.0;
  point1[1] = 5.0;
  point1[2] = 6.0;
  nd1->SetPosition(point1);
  nd1->SetDataValid(true);

  mitk::NavigationData::PositionType point2;
  point2[0] = 7.0;
  point2[1] = 8.0;
  point2[2] = 9.0;
  nd2->SetPosition(point2);
  nd2->SetDataValid(true);

  mitk::NavigationData::PositionType point3;
  point3[0] = 10.0;
  point3[1] = 11.0;
  point3[2] = 12.0;
  nd3->SetPosition(point3);
  nd3->SetDataValid(true);

  m_NavigationDataToPointSetFilter->SetInput(0, nd0);
  m_NavigationDataToPointSetFilter->SetInput(1, nd1);
  m_NavigationDataToPointSetFilter->SetInput(2, nd2);
  m_NavigationDataToPointSetFilter->SetInput(3, nd3);

  //Process
  mitk::PointSet::Pointer pointSet0 = m_NavigationDataToPointSetFilter->GetOutput();
  mitk::PointSet::Pointer pointSet1 = m_NavigationDataToPointSetFilter->GetOutput(1);
  mitk::PointSet::Pointer pointSet2 = m_NavigationDataToPointSetFilter->GetOutput(2);
  mitk::PointSet::Pointer pointSet3 = m_NavigationDataToPointSetFilter->GetOutput(3);

  pointSet0->Update();

  MITK_TEST_OUTPUT(<< "Testing the conversion of navigation data object to PointSets in Mode 3D:");
  MITK_TEST_CONDITION(mitk::Equal(pointSet0->GetPoint(0), point0), "Pointset 0 correct?");
  MITK_TEST_CONDITION(mitk::Equal(pointSet1->GetPoint(0), point1), "Pointset 1 correct?");
  MITK_TEST_CONDITION(mitk::Equal(pointSet2->GetPoint(0), point2), "Pointset 2 correct?");
  MITK_TEST_CONDITION(mitk::Equal(pointSet3->GetPoint(0), point3), "Pointset 3 correct?");
}

static void TestMode4D()
{
  Setup();
  m_NavigationDataToPointSetFilter->SetOperationMode(mitk::NavigationDataToPointSetFilter::Mode4D);
  m_NavigationDataToPointSetFilter->SetRingBufferSize(2);

  //Build up test data
  mitk::NavigationData::Pointer nd = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd2 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd3 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd4 = mitk::NavigationData::New();

  mitk::NavigationData::PositionType point;

  point[0] = 1.0;
  point[1] = 2.0;
  point[2] = 3.0;
  nd->SetPosition(point);

  point[0] = 4.0;
  point[1] = 5.0;
  point[2] = 6.0;
  nd2->SetPosition(point);

  point[0] = 7.0;
  point[1] = 8.0;
  point[2] = 9.0;
  nd3->SetPosition(point);

  point[0] = 10.0;
  point[1] = 11.0;
  point[2] = 12.0;
  nd4->SetPosition(point);

  m_NavigationDataToPointSetFilter->SetInput(0, nd);
  m_NavigationDataToPointSetFilter->SetInput(1, nd2);

  mitk::PointSet::Pointer pointSet = m_NavigationDataToPointSetFilter->GetOutput();
  pointSet->Update();

  MITK_TEST_CONDITION( pointSet->GetPoint(0,0)[0] == 1.0 && pointSet->GetPoint(0,0)[1] == 2.0 && pointSet->GetPoint(0,0)[2] == 3.0 &&
    pointSet->GetPoint(1,0)[0] == 4.0 && pointSet->GetPoint(1,0)[1] == 5.0 && pointSet->GetPoint(1,0)[2] == 6.0
    , "Testing the conversion of navigation data object to one point set in Mode 4D in first timestep" );

  m_NavigationDataToPointSetFilter->SetInput(0, nd3);
  m_NavigationDataToPointSetFilter->SetInput(1, nd4);
  m_NavigationDataToPointSetFilter->Update();
  pointSet = m_NavigationDataToPointSetFilter->GetOutput();

  MITK_TEST_CONDITION( pointSet->GetPoint(0,0)[0] == 1.0 && pointSet->GetPoint(0,0)[1] == 2.0 && pointSet->GetPoint(0,0)[2] == 3.0 &&
    pointSet->GetPoint(1,0)[0] == 4.0 && pointSet->GetPoint(1,0)[1] == 5.0 && pointSet->GetPoint(1,0)[2] == 6.0 &&
    pointSet->GetPoint(0,1)[0] == 7.0 && pointSet->GetPoint(0,1)[1] == 8.0 && pointSet->GetPoint(0,1)[2] == 9.0 &&
    pointSet->GetPoint(1,1)[0] == 10.0 && pointSet->GetPoint(1,1)[1] == 11.0 && pointSet->GetPoint(1,1)[2] == 12.0
    , "Testing the conversion of navigation data object to one point set in Mode 4D in second timestep" );

  m_NavigationDataToPointSetFilter->SetInput(0, nd3);
  m_NavigationDataToPointSetFilter->SetInput(1, nd4);
  pointSet = m_NavigationDataToPointSetFilter->GetOutput();
  pointSet->Update();

  MITK_TEST_CONDITION( pointSet->GetPoint(0,0)[0] == 7.0 && pointSet->GetPoint(0,0)[1] == 8.0 && pointSet->GetPoint(0,0)[2] == 9.0 &&
    pointSet->GetPoint(1,0)[0] == 10.0 && pointSet->GetPoint(1,0)[1] == 11.0 && pointSet->GetPoint(1,0)[2] == 12.0 &&
    pointSet->GetPoint(0,1)[0] == 7.0 && pointSet->GetPoint(0,1)[1] == 8.0 && pointSet->GetPoint(0,1)[2] == 9.0 &&
    pointSet->GetPoint(1,1)[0] == 10.0 && pointSet->GetPoint(1,1)[1] == 11.0 && pointSet->GetPoint(1,1)[2] == 12.0
    , "Testing the correct ring buffer behavior" );
}

static void TestMode3DMean()
{
  Setup();
  m_NavigationDataToPointSetFilter->SetOperationMode(mitk::NavigationDataToPointSetFilter::Mode3DMean);
  int numberForMean = 5;
  m_NavigationDataToPointSetFilter->SetNumberForMean(numberForMean);

  MITK_TEST_CONDITION(mitk::Equal(m_NavigationDataToPointSetFilter->GetNumberForMean(), numberForMean), "Testing get/set for numberForMean");

  mitk::NavigationDataSequentialPlayer::Pointer player = mitk::NavigationDataSequentialPlayer::New();

  std::string file(MITK_IGT_DATA_DIR);
  file.append("/NavigationDataTestData_2Tools.xml");

  mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();

  player->SetNavigationDataSet( reader->Read(file) );

  for (unsigned int i = 0; i< player->GetNumberOfOutputs(); i++)
  {
    m_NavigationDataToPointSetFilter->SetInput(i, player->GetOutput(i));
  }

  mitk::PointSet::Pointer pointSet0 = m_NavigationDataToPointSetFilter->GetOutput();
  mitk::PointSet::Pointer pointSet1 = m_NavigationDataToPointSetFilter->GetOutput(1);

  m_NavigationDataToPointSetFilter->Update();

  MITK_TEST_CONDITION(pointSet0->GetPoint(0)[0]==3.0 && pointSet0->GetPoint(0)[1]==2.0 && pointSet0->GetPoint(0)[2]==5.0,
    "Testing the average of first input");

  MITK_TEST_CONDITION(pointSet1->GetPoint(0)[0]==30.0 && pointSet1->GetPoint(0)[1]==20.0 && pointSet1->GetPoint(0)[2]==50.0,
    "Testing the average of second input");
}

static void NavigationDataToPointSetFilterContructor_DefaultCall_IsNotEmpty()
{
  Setup();
  MITK_TEST_CONDITION_REQUIRED(m_NavigationDataToPointSetFilter.IsNotNull(),"Testing instantiation");
  //I think this test is meaningless, because it will never ever fail. I keep it for know just to be save.
}

static void NavigationDataToPointSetFilterSetInput_SimplePoint_EqualsGroundTruth()
{
  Setup();

  mitk::NavigationData::Pointer nd_in = mitk::NavigationData::New();
  const mitk::NavigationData* nd_out = mitk::NavigationData::New();
  mitk::NavigationData::PositionType point;

  point[0] = 1.0;
  point[1] = 2.0;
  point[2] = 3.0;
  nd_in->SetPosition(point);

  m_NavigationDataToPointSetFilter->SetInput(nd_in);
  nd_out = m_NavigationDataToPointSetFilter->GetInput();

  MITK_TEST_CONDITION( nd_out->GetPosition() == nd_in->GetPosition(),
    "Testing get/set input" );
}

int mitkNavigationDataToPointSetFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataToPointSetFilter");

  NavigationDataToPointSetFilterContructor_DefaultCall_IsNotEmpty();
  NavigationDataToPointSetFilterSetInput_SimplePoint_EqualsGroundTruth();
  TestMode3D();
  TestMode4D();
//  TestMode3DMean(); //infinite loop in debug mode, see bug 17763

  MITK_TEST_END();
}
