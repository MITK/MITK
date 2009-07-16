/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataToPointSetFilter.h"

#include "mitkTestingMacros.h"

#include <iostream>

/**
 *  Simple example for a test for the (non-existent) class "NavigationDataToPointSetFilter".
 *  
 *  argc and argv are the command line parameters which were passed to 
 *  the ADD_TEST command in the CMakeLists.txt file. For the automatic
 *  tests, argv is either empty for the simple tests or contains the filename
 *  of a test image for the image tests (see CMakeLists.txt).
 */
class mitkNavigationDataToPointSetFilterTestClass { 

public:

static void TestMode3D(mitk::NavigationDataToPointSetFilter::Pointer myNavigationDataToPointSetFilter)
{
  myNavigationDataToPointSetFilter->SetOperationMode(mitk::NavigationDataToPointSetFilter::Mode3D);

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

  myNavigationDataToPointSetFilter->SetInput(0, nd0);
  myNavigationDataToPointSetFilter->SetInput(1, nd1);
  myNavigationDataToPointSetFilter->SetInput(2, nd2);
  myNavigationDataToPointSetFilter->SetInput(3, nd3);

  //Process
  mitk::PointSet::Pointer pointSet0 = myNavigationDataToPointSetFilter->GetOutput(0);
  mitk::PointSet::Pointer pointSet1 = myNavigationDataToPointSetFilter->GetOutput(1);
  mitk::PointSet::Pointer pointSet2 = myNavigationDataToPointSetFilter->GetOutput(2);
  mitk::PointSet::Pointer pointSet3 = myNavigationDataToPointSetFilter->GetOutput(3);

  pointSet0->Update();

  MITK_TEST_OUTPUT(<< "Testing the conversion of navigation data object to PointSets in Mode 3D:");
  MITK_TEST_CONDITION(mitk::Equal(pointSet0->GetPoint(0), point0), "Pointset 0 correct?");
  MITK_TEST_CONDITION(mitk::Equal(pointSet1->GetPoint(0), point1), "Pointset 1 correct?");
  MITK_TEST_CONDITION(mitk::Equal(pointSet2->GetPoint(0), point2), "Pointset 2 correct?");
  MITK_TEST_CONDITION(mitk::Equal(pointSet3->GetPoint(0), point3), "Pointset 3 correct?");

    //pointSet0->GetPoint(0)[0] == 1.0  && pointSet0->GetPoint(0)[1] == 2.0  && pointSet0->GetPoint(0)[2] == 3.0 &&
    //                  pointSet1->GetPoint(0)[0] == 4.0  && pointSet1->GetPoint(0)[1] == 5.0  && pointSet1->GetPoint(0)[2] == 6.0 &&
    //                  pointSet2->GetPoint(0)[0] == 7.0  && pointSet2->GetPoint(0)[1] == 8.0  && pointSet2->GetPoint(0)[2] == 9.0 &&
    //                  pointSet3->GetPoint(0)[0] == 10.0 && pointSet3->GetPoint(0)[1] == 11.0 && pointSet3->GetPoint(0)[2] == 12.0 
    //, "Testing the conversion of navigation data object to PointSets in Mode 3D" );

}


static void TestMode4D(mitk::NavigationDataToPointSetFilter::Pointer myNavigationDataToPointSetFilter)
{
  myNavigationDataToPointSetFilter->SetOperationMode(mitk::NavigationDataToPointSetFilter::Mode4D);
  myNavigationDataToPointSetFilter->SetRingBufferSize(2);

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

  myNavigationDataToPointSetFilter->SetInput(0, nd);
  myNavigationDataToPointSetFilter->SetInput(1, nd2);

  mitk::PointSet::Pointer pointSet = myNavigationDataToPointSetFilter->GetOutput();
  pointSet->Update();

  MITK_TEST_CONDITION( pointSet->GetPoint(0,0)[0] == 1.0 && pointSet->GetPoint(0,0)[1] == 2.0 && pointSet->GetPoint(0,0)[2] == 3.0 &&
    pointSet->GetPoint(1,0)[0] == 4.0 && pointSet->GetPoint(1,0)[1] == 5.0 && pointSet->GetPoint(1,0)[2] == 6.0 
    , "Testing the conversion of navigation data object to one point set in Mode 4D in first timestep" );

  myNavigationDataToPointSetFilter->SetInput(0, nd3);
  myNavigationDataToPointSetFilter->SetInput(1, nd4);
  myNavigationDataToPointSetFilter->Update();
  pointSet = myNavigationDataToPointSetFilter->GetOutput();

  MITK_TEST_CONDITION( pointSet->GetPoint(0,0)[0] == 1.0 && pointSet->GetPoint(0,0)[1] == 2.0 && pointSet->GetPoint(0,0)[2] == 3.0 &&
    pointSet->GetPoint(1,0)[0] == 4.0 && pointSet->GetPoint(1,0)[1] == 5.0 && pointSet->GetPoint(1,0)[2] == 6.0 &&
    pointSet->GetPoint(0,1)[0] == 7.0 && pointSet->GetPoint(0,1)[1] == 8.0 && pointSet->GetPoint(0,1)[2] == 9.0 &&
    pointSet->GetPoint(1,1)[0] == 10.0 && pointSet->GetPoint(1,1)[1] == 11.0 && pointSet->GetPoint(1,1)[2] == 12.0
    , "Testing the conversion of navigation data object to one point set in Mode 4D in second timestep" );

  myNavigationDataToPointSetFilter->SetInput(0, nd3);
  //nd3->Modified(); //necessary because the generate data is only called when input has changed...
  myNavigationDataToPointSetFilter->SetInput(1, nd4);
  //nd4->Modified();
  //myNavigationDataToPointSetFilter->Update();
  pointSet = myNavigationDataToPointSetFilter->GetOutput();
  pointSet->Update();

  MITK_TEST_CONDITION( pointSet->GetPoint(0,0)[0] == 7.0 && pointSet->GetPoint(0,0)[1] == 8.0 && pointSet->GetPoint(0,0)[2] == 9.0 &&
    pointSet->GetPoint(1,0)[0] == 10.0 && pointSet->GetPoint(1,0)[1] == 11.0 && pointSet->GetPoint(1,0)[2] == 12.0 &&
    pointSet->GetPoint(0,1)[0] == 7.0 && pointSet->GetPoint(0,1)[1] == 8.0 && pointSet->GetPoint(0,1)[2] == 9.0 &&
    pointSet->GetPoint(1,1)[0] == 10.0 && pointSet->GetPoint(1,1)[1] == 11.0 && pointSet->GetPoint(1,1)[2] == 12.0
    , "Testing the correct ring buffer behavior" );
}
};


int mitkNavigationDataToPointSetFilterTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("NavigationDataToPointSetFilter");

  // let's create an object of our class  
  mitk::NavigationDataToPointSetFilter::Pointer myNavigationDataToPointSetFilter = mitk::NavigationDataToPointSetFilter::New();

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myNavigationDataToPointSetFilter.IsNotNull(),"Testing instantiation");

  // write your own tests here and use the macros from mitkTestingMacros.h !!!
  // do not write to std::cout and do not return from this function yourself!
  mitkNavigationDataToPointSetFilterTestClass::TestMode3D(myNavigationDataToPointSetFilter);
  myNavigationDataToPointSetFilter = mitk::NavigationDataToPointSetFilter::New();
  mitkNavigationDataToPointSetFilterTestClass::TestMode4D(myNavigationDataToPointSetFilter);
  // always end with this!
  MITK_TEST_END();
}
