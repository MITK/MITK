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

#include <iostream>

#include "mitkClippedSurfaceBoundsCalculator.h"
#include "mitkGeometry3D.h"
#include "mitkPlaneGeometry.h"
#include "mitkNumericTypes.h"

static void CheckPlanesInsideBoundingBoxOnlyOnOneSlice(mitk::BaseGeometry::Pointer geometry3D)
{
  //Check planes which are inside the bounding box

  mitk::ClippedSurfaceBoundsCalculator* calculator = new mitk::ClippedSurfaceBoundsCalculator();
  mitk::Image::Pointer image = mitk::Image::New();
  image->Initialize( mitk::MakePixelType<int, int, 1>(), *(geometry3D.GetPointer()) );

  //Check planes which are only on one slice:

  //Slice 0
  mitk::Point3D origin;
  origin[0] = 511;
  origin[1] = 0;
  origin[2] = 0;

  mitk::Vector3D normal;
  mitk::FillVector3D(normal, 0, 0, 1);

  mitk::PlaneGeometry::Pointer planeOnSliceZero = mitk::PlaneGeometry::New();
  planeOnSliceZero->InitializePlane(origin, normal);

  calculator->SetInput( planeOnSliceZero , image);
  calculator->Update();

  mitk::ClippedSurfaceBoundsCalculator::OutputType minMax = calculator->GetMinMaxSpatialDirectionZ();

  MITK_TEST_CONDITION(minMax.first == minMax.second, "Check if plane is only on one slice");
  MITK_TEST_CONDITION(minMax.first == 0 && minMax.second == 0, "Check if plane is on slice 0");


  //Slice 3
  origin[2] = 3;

  mitk::PlaneGeometry::Pointer planeOnSliceThree = mitk::PlaneGeometry::New();
  planeOnSliceThree->InitializePlane(origin, normal);
  planeOnSliceThree->SetImageGeometry(false);

  calculator->SetInput( planeOnSliceThree , image);
  calculator->Update();

  minMax = calculator->GetMinMaxSpatialDirectionZ();

  MITK_TEST_CONDITION(minMax.first == minMax.second, "Check if plane is only on one slice");
  MITK_TEST_CONDITION(minMax.first == 3 && minMax.second == 3, "Check if plane is on slice 3");

  //Slice 17
  origin[2] = 17;

  mitk::PlaneGeometry::Pointer planeOnSliceSeventeen = mitk::PlaneGeometry::New();
  planeOnSliceSeventeen->InitializePlane(origin, normal);

  calculator->SetInput( planeOnSliceSeventeen , image);
  calculator->Update();

  minMax = calculator->GetMinMaxSpatialDirectionZ();

  MITK_TEST_CONDITION(minMax.first == minMax.second, "Check if plane is only on one slice");
  MITK_TEST_CONDITION(minMax.first == 17 && minMax.second == 17, "Check if plane is on slice 17");


  //Slice 20
  origin[2] = 19;

  mitk::PlaneGeometry::Pointer planeOnSliceTwenty = mitk::PlaneGeometry::New();
  planeOnSliceTwenty->InitializePlane(origin, normal);

  calculator->SetInput( planeOnSliceTwenty , image);
  calculator->Update();

  minMax = calculator->GetMinMaxSpatialDirectionZ();

  MITK_TEST_CONDITION(minMax.first == minMax.second, "Check if plane is only on one slice");
  MITK_TEST_CONDITION(minMax.first == 19 && minMax.second == 19, "Check if plane is on slice 19");

  delete calculator;
}

static void CheckPlanesInsideBoundingBox(mitk::BaseGeometry::Pointer geometry3D)
{
  //Check planes which are inside the bounding box

  mitk::ClippedSurfaceBoundsCalculator* calculator = new mitk::ClippedSurfaceBoundsCalculator();
  mitk::Image::Pointer image = mitk::Image::New();
  image->Initialize( mitk::MakePixelType<int, int, 1>(), *(geometry3D.GetPointer()) );

  //Check planes which are only on one slice:
  //Slice 0
  mitk::Point3D origin;
  origin[0] = 511; // Set to 511.9 so that the intersection point is inside the bounding box
  origin[1] = 0;
  origin[2] = 0;

  mitk::Vector3D normal;
  mitk::FillVector3D(normal, 1, 0, 0);

  mitk::PlaneGeometry::Pointer planeSagittalOne = mitk::PlaneGeometry::New();
  planeSagittalOne->InitializePlane(origin, normal);

  calculator->SetInput( planeSagittalOne , image);
  calculator->Update();

  mitk::ClippedSurfaceBoundsCalculator::OutputType minMax = calculator->GetMinMaxSpatialDirectionZ();
  MITK_TEST_CONDITION(minMax.first == 0 && minMax.second == 19, "Check if plane is from slice 0 to slice 19");


  //Slice 3
  origin[0] = 256;

  MITK_INFO << "Case1 origin: " << origin;

  mitk::PlaneGeometry::Pointer planeSagittalTwo = mitk::PlaneGeometry::New();
  planeSagittalTwo->InitializePlane(origin, normal);

  MITK_INFO << "PlaneNormal: " << planeSagittalTwo->GetNormal();
  MITK_INFO << "PlaneOrigin: " << planeSagittalTwo->GetOrigin();

  calculator->SetInput( planeSagittalTwo , image);
  calculator->Update();

  minMax = calculator->GetMinMaxSpatialDirectionZ();
  MITK_INFO << "min: " << minMax.first << " max: " << minMax.second;

  MITK_TEST_CONDITION(minMax.first == 0 && minMax.second == 19, "Check if plane is from slice 0 to slice 19");

  //Slice 17
  origin[0] = 0;  // Set to 0.1 so that the intersection point is inside the bounding box

  mitk::PlaneGeometry::Pointer planeOnSliceSeventeen = mitk::PlaneGeometry::New();
  planeOnSliceSeventeen->InitializePlane(origin, normal);

  calculator->SetInput( planeOnSliceSeventeen , image);
  calculator->Update();

  minMax = calculator->GetMinMaxSpatialDirectionZ();
  MITK_TEST_CONDITION(minMax.first == 0 && minMax.second == 19, "Check if plane is from slice 0 to slice 19");

  //Crooked planes:
  origin[0] = 0;
  origin[1] = 507;
  origin[2] = 0;

  normal[0] = 1;
  normal[1] = -1;
  normal[2] = 1;

  mitk::PlaneGeometry::Pointer planeCrookedOne = mitk::PlaneGeometry::New();
  planeCrookedOne->InitializePlane(origin, normal);

  calculator->SetInput( planeCrookedOne , image);
  calculator->Update();

  minMax = calculator->GetMinMaxSpatialDirectionZ();
  MITK_INFO << "min: " << minMax.first << " max: " << minMax.second;

  MITK_TEST_CONDITION(minMax.first == 0 && minMax.second == 4, "Check if plane is from slice 0 to slice 4 with inclined plane");

  origin[0] = 512;
  origin[1] = 0;
  origin[2] = 16;

  mitk::PlaneGeometry::Pointer planeCrookedTwo = mitk::PlaneGeometry::New();
  planeCrookedTwo->InitializePlane(origin, normal);

  calculator->SetInput( planeCrookedTwo , image);
  calculator->Update();

  minMax = calculator->GetMinMaxSpatialDirectionZ();
  MITK_INFO << "min: " << minMax.first << " max: " << minMax.second;

  MITK_TEST_CONDITION(minMax.first == 17 && minMax.second == 19, "Check if plane is from slice 17 to slice 19 with inclined plane");


  origin[0] = 511;
  origin[1] = 0;
  origin[2] = 0;

  normal[1] = 0;
  normal[2] = 0.04;

  mitk::PlaneGeometry::Pointer planeCrookedThree = mitk::PlaneGeometry::New();
  planeCrookedThree->InitializePlane(origin, normal);

  calculator->SetInput( planeCrookedThree , image);
  calculator->Update();

  minMax = calculator->GetMinMaxSpatialDirectionZ();
  MITK_INFO << "min: " << minMax.first << " max: " << minMax.second;

  MITK_TEST_CONDITION(minMax.first == 0 && minMax.second == 19, "Check if plane is from slice 0 to slice 19 with inclined plane");


  delete calculator;
}

static void CheckPlanesOutsideOfBoundingBox(mitk::BaseGeometry::Pointer geometry3D)
{
  //Check planes which are outside of the bounding box

  mitk::ClippedSurfaceBoundsCalculator* calculator = new mitk::ClippedSurfaceBoundsCalculator();
  mitk::Image::Pointer image = mitk::Image::New();
  image->Initialize( mitk::MakePixelType<int, int, 1>(), *(geometry3D.GetPointer()) );

  //In front of the bounding box
  mitk::Point3D origin;
  origin[0] = 511;
  origin[1] = 0;
  origin[2] = -5;

  mitk::Vector3D normal;
  mitk::FillVector3D(normal, 0, 0, 1);

  mitk::PlaneGeometry::Pointer planeInFront = mitk::PlaneGeometry::New();
  planeInFront->InitializePlane(origin, normal);

  calculator->SetInput( planeInFront , image);
  calculator->Update();

  mitk::ClippedSurfaceBoundsCalculator::OutputType minMax = calculator->GetMinMaxSpatialDirectionZ();

  MITK_TEST_CONDITION(minMax.first == std::numeric_limits<int>::max(), "Check if min value hasn't been set");
  MITK_TEST_CONDITION(minMax.second == std::numeric_limits<int>::min(), "Check if max value hasn't been set");

  //Behind the bounding box
  origin[2] = 515;

  mitk::PlaneGeometry::Pointer planeBehind = mitk::PlaneGeometry::New();
  planeBehind->InitializePlane(origin, normal);

  calculator->SetInput( planeBehind , image);
  calculator->Update();

  minMax = calculator->GetMinMaxSpatialDirectionZ();

  MITK_TEST_CONDITION(minMax.first == std::numeric_limits<int>::max(), "Check if min value hasn't been set");
  MITK_TEST_CONDITION(minMax.second == std::numeric_limits<int>::min(), "Check if max value hasn't been set");

  //Above
  origin[1] = 515;

  mitk::FillVector3D(normal, 0, 1, 0);

  mitk::PlaneGeometry::Pointer planeAbove = mitk::PlaneGeometry::New();
  planeAbove->InitializePlane(origin, normal);

  calculator->SetInput( planeAbove , image);
  calculator->Update();

  minMax = calculator->GetMinMaxSpatialDirectionZ();

  MITK_TEST_CONDITION(minMax.first == std::numeric_limits<int>::max(), "Check if min value hasn't been set");
  MITK_TEST_CONDITION(minMax.second == std::numeric_limits<int>::min(), "Check if max value hasn't been set");

  //Below
  origin[1] = -5;

  mitk::PlaneGeometry::Pointer planeBelow = mitk::PlaneGeometry::New();
  planeBelow->InitializePlane(origin, normal);

  calculator->SetInput( planeBelow , image);
  calculator->Update();

  minMax = calculator->GetMinMaxSpatialDirectionZ();

  MITK_TEST_CONDITION(minMax.first == std::numeric_limits<int>::max(), "Check if min value hasn't been set");
  MITK_TEST_CONDITION(minMax.second == std::numeric_limits<int>::min(), "Check if max value hasn't been set");

  //Left side
  origin[0] = -5;


  mitk::FillVector3D(normal, 1, 0, 0);

  mitk::PlaneGeometry::Pointer planeLeftSide = mitk::PlaneGeometry::New();
  planeLeftSide->InitializePlane(origin, normal);

  calculator->SetInput( planeLeftSide , image);
  calculator->Update();

  minMax = calculator->GetMinMaxSpatialDirectionZ();

  MITK_TEST_CONDITION(minMax.first == std::numeric_limits<int>::max(), "Check if min value hasn't been set");
  MITK_TEST_CONDITION(minMax.second == std::numeric_limits<int>::min(), "Check if max value hasn't been set");

  //Right side
  origin[1] = 515;

  mitk::PlaneGeometry::Pointer planeRightSide = mitk::PlaneGeometry::New();
  planeRightSide->InitializePlane(origin, normal);

  calculator->SetInput( planeRightSide , image);
  calculator->Update();

  minMax = calculator->GetMinMaxSpatialDirectionZ();

  MITK_TEST_CONDITION(minMax.first == std::numeric_limits<int>::max(), "Check if min value hasn't been set");
  MITK_TEST_CONDITION(minMax.second == std::numeric_limits<int>::min(), "Check if max value hasn't been set");

  delete calculator;
}


static void CheckIntersectionPointsOfTwoGeometry3D(mitk::BaseGeometry::Pointer firstGeometry3D, mitk::BaseGeometry::Pointer secondGeometry3D)
{
  mitk::ClippedSurfaceBoundsCalculator* calculator = new mitk::ClippedSurfaceBoundsCalculator();
  mitk::Image::Pointer firstImage = mitk::Image::New();
  firstImage->Initialize( mitk::MakePixelType<int, int, 1>(), *(firstGeometry3D.GetPointer()) );

  calculator->SetInput( secondGeometry3D, firstImage);
  calculator->Update();

  mitk::ClippedSurfaceBoundsCalculator::OutputType minMax = calculator->GetMinMaxSpatialDirectionZ();
  minMax = calculator->GetMinMaxSpatialDirectionZ();
  MITK_INFO << "min: " << minMax.first << " max: " << minMax.second;

  MITK_TEST_CONDITION(minMax.first == 0 && minMax.second == 19, "Check if plane is from slice 0 to slice 19");
  delete calculator;
}


static void CheckIntersectionWithPointCloud( mitk::BaseGeometry::Pointer geometry3D )
{
  //Check planes which are inside the bounding box

  mitk::Image::Pointer image = mitk::Image::New();
  image->Initialize( mitk::MakePixelType<int, int, 1>(), *(geometry3D.GetPointer()) );

  {
    mitk::Point3D pnt1, pnt2;
    pnt1[0] = 3;
    pnt1[1] = 5;
    pnt1[2] = 3;
    pnt2[0] = 8;
    pnt2[1] = 3;
    pnt2[2] = 8;

    mitk::ClippedSurfaceBoundsCalculator::PointListType pointlist;
    pointlist.push_back( pnt1 );
    pointlist.push_back( pnt2 );

    mitk::ClippedSurfaceBoundsCalculator calculator;
    calculator.SetInput( pointlist, image );
    calculator.Update();

    mitk::ClippedSurfaceBoundsCalculator::OutputType minMaxZ = calculator.GetMinMaxSpatialDirectionZ();
    MITK_TEST_CONDITION(minMaxZ.first == 3 && minMaxZ.second == 8, "Check if points span from slice 3 to slice 8 in axial");

    mitk::ClippedSurfaceBoundsCalculator::OutputType minMaxX = calculator.GetMinMaxSpatialDirectionX();
    MITK_TEST_CONDITION(minMaxX.first == 3 && minMaxX.second == 5, "Check if points span from slice 3 to slice 5 in sagittal");
  }

  {
    mitk::Point3D pnt1, pnt2;
    pnt1.Fill( -3 );
    pnt2.Fill( 600 );

    mitk::ClippedSurfaceBoundsCalculator::PointListType pointlist;
    pointlist.push_back( pnt1 );
    pointlist.push_back( pnt2 );

    mitk::ClippedSurfaceBoundsCalculator calculator;
    calculator.SetInput( pointlist, image );
    calculator.Update();

    mitk::ClippedSurfaceBoundsCalculator::OutputType minMaxZ = calculator.GetMinMaxSpatialDirectionZ();
    MITK_TEST_CONDITION(minMaxZ.first == 0 && minMaxZ.second == 19, "Check if points are correctly clipped to slice 0 and slice 19 in axial");

    mitk::ClippedSurfaceBoundsCalculator::OutputType minMaxX = calculator.GetMinMaxSpatialDirectionX();
    MITK_TEST_CONDITION(minMaxX.first == 0 && minMaxX.second == 511, "Check if points are correctly clipped to slice 0 and slice 511 in sagittal");
  }



}


int mitkClippedSurfaceBoundsCalculatorTest(int, char* [])
{
  // always start with this!
  MITK_TEST_BEGIN("ClippedSurfaceBoundsCalculator");

  /** The class mitkClippedSurfaceBoundsCalculator calculates the intersection points of a PlaneGeometry and a Geometry3D.
  *   This unittest checks if the correct min and max values for the three spatial directions (x, y, z)
  *   are calculated. To test this we define artifical PlaneGeometries and Geometry3Ds and test different
  *   scenarios:
  *
  *   1. planes which are inside the bounding box of a 3D geometry but only on one slice
  *   2. planes which are outside of the bounding box
  *   3. planes which are inside the bounding box but over more than one slice
  *
  *   Note: Currently rotated geometries are not tested!
  */

  /********************* Define Geometry3D ***********************/
  //Define origin:
  mitk::Point3D origin;
  origin[0] = 511;
  origin[1] = 0;
  origin[2] = 0;

  //Define normal:
  mitk::Vector3D normal;
  mitk::FillVector3D(normal, 0, 0, 1);

  //Initialize PlaneGeometry:
  mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializePlane(origin, normal);

  //Set Bounds:
  mitk::BoundingBox::BoundsArrayType bounds = planeGeometry->GetBounds();
  bounds[0] = 0;
  bounds[1] = 512;
  bounds[2] = 0;
  bounds[3] = 512;
  bounds[4] = 0;
  bounds[5] = 1;
  planeGeometry->SetBounds(bounds);

  //Initialize SlicedGeometry3D:
  mitk::SlicedGeometry3D::Pointer slicedGeometry3D = mitk::SlicedGeometry3D::New();
  slicedGeometry3D->InitializeEvenlySpaced(dynamic_cast<mitk::PlaneGeometry*>(planeGeometry.GetPointer()), 20);
  mitk::BaseGeometry::Pointer geometry3D = dynamic_cast< mitk::BaseGeometry* > ( slicedGeometry3D.GetPointer() );
  geometry3D->SetImageGeometry(true);

  //Define origin for second Geometry3D;
  mitk::Point3D origin2;
  origin2[0] = 511;
  origin2[1] = 60;
  origin2[2] = 0;

 //Define normal:
  mitk::Vector3D normal2;
  mitk::FillVector3D(normal2, 0, 1, 0);

  //Initialize PlaneGeometry:
  mitk::PlaneGeometry::Pointer planeGeometry2 = mitk::PlaneGeometry::New();
  planeGeometry2->InitializePlane(origin2, normal2);

  //Initialize SlicedGeometry3D:
  mitk::SlicedGeometry3D::Pointer secondSlicedGeometry3D = mitk::SlicedGeometry3D::New();
  secondSlicedGeometry3D->InitializeEvenlySpaced(dynamic_cast<mitk::PlaneGeometry*>(planeGeometry2.GetPointer()), 20);
  mitk::BaseGeometry::Pointer secondGeometry3D = dynamic_cast< mitk::BaseGeometry* > ( secondSlicedGeometry3D.GetPointer() );
  secondGeometry3D->SetImageGeometry(true);


  /***************************************************************/

  CheckPlanesInsideBoundingBoxOnlyOnOneSlice(geometry3D);
  CheckPlanesOutsideOfBoundingBox(geometry3D);
  CheckPlanesInsideBoundingBox(geometry3D);
  CheckIntersectionPointsOfTwoGeometry3D(geometry3D, secondGeometry3D);
  CheckIntersectionWithPointCloud( geometry3D );


  /** ToDo:
  *  test also rotated 3D geometry!
  */

  MITK_TEST_END();
}
