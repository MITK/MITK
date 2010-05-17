/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-15 11:12:36 +0100 (Mo, 15 Mrz 2010) $
Version:   $Revision: 21745 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTestingMacros.h"
#include "mitkPlanarPolygon.h"
#include "mitkPlaneGeometry.h"


class mitkPlanarPolygonTestClass
{

public:


static void TestPlanarPolygonPlacement( mitk::PlanarPolygon::Pointer planarPolygon )
{
  // Test for correct minimum number of control points in cross-mode
  MITK_TEST_CONDITION( planarPolygon->GetMinimumNumberOfControlPoints() == 2, "Minimum number of control points" );

  // Test for correct maximum number of control points in cross-mode
  MITK_TEST_CONDITION( planarPolygon->GetMaximumNumberOfControlPoints() == 1000, "Maximum number of control points" );

  // Initial placement of PlanarPolygon
  mitk::Point2D p0;
  p0[0] = 00.0; p0[1] = 0.0;
  planarPolygon->PlaceFigure( p0 );

  // Add second control point
  mitk::Point2D p1;
  p1[0] = 50.0; p1[1] = 00.0;
  planarPolygon->SetControlPoint(1, p1 );

  // Add third control point
  mitk::Point2D p2;
  p2[0] = 50.0; p2[1] = 50.0;
  planarPolygon->AddControlPoint( p2 );

  // Add fourth control point
  mitk::Point2D p3;
  p3[0] = 0.0; p3[1] = 50.0;
  planarPolygon->AddControlPoint( p3 );

  // Test for number of control points
  MITK_TEST_CONDITION( planarPolygon->GetNumberOfControlPoints() == 4, "Number of control points after placement" );

  // Test if PlanarFigure is closed
  MITK_TEST_CONDITION( planarPolygon->IsClosed(), "planar polygon should not be closed, yet, right?" );
  
  planarPolygon->SetClosed(true);

  MITK_TEST_CONDITION( planarPolygon->IsClosed(), "planar polygon should be closed after function call, right?" );
  
  // Test for number of polylines
  const mitk::PlanarFigure::VertexContainerType* polyLine0 = planarPolygon->GetPolyLine( 0 );
  MITK_TEST_CONDITION( planarPolygon->GetPolyLinesSize() == 1, "Number of polylines after placement" );

  // Get polylines and check if the generated coordinates are OK
  const mitk::Point2D& pp0 = polyLine0->ElementAt( 0 );
  const mitk::Point2D& pp1 = polyLine0->ElementAt( 1 );
  MITK_TEST_CONDITION( ((pp0 == p0) && (pp1 == p1))
    || ((pp0 == p1) && (pp1 == p0)), "Correct polyline 1" );

    
  // Test for number of measurement features
  planarPolygon->EvaluateFeatures();
  MITK_TEST_CONDITION( planarPolygon->GetNumberOfFeatures() == 2, "Number of measurement features" );

  // Test for correct feature evaluation
  double length0 = 4 * 50.0; // circumference
  MITK_TEST_CONDITION( fabs( planarPolygon->GetQuantity( 0 ) - length0) < mitk::eps, "Size of longest diameter" );

  double length1 = 50.0 * 50.0 ; // area
  MITK_TEST_CONDITION( fabs( planarPolygon->GetQuantity( 1 ) - length1) < mitk::eps, "Size of short axis diameter" );
}

static void TestplanarPolygonPlacementConstrained(mitk::PlanarPolygon::Pointer planarPolygon)
{
  // **************************************************************************
  // Place first control point out of bounds (to the left of the image bounds)
  mitk::Point2D p0;
  p0[0] = -20.0; p0[1] = 20.0;
  planarPolygon->PlaceFigure( p0 );

  // Test if constraint has been applied correctly
  mitk::Point2D cp0 = planarPolygon->GetControlPoint( 0 );
  MITK_TEST_CONDITION( 
       (fabs(cp0[0]) < mitk::eps)
    && (fabs(cp0[1] - 20.0) < mitk::eps), "Point1 placed and constrained correctly" );

  // **************************************************************************
  // Add second control point out of bounds (to the top of the image bounds)
  mitk::Point2D p1;
  p1[0] = 80.0; p1[1] = 120.0;
  planarPolygon->SetCurrentControlPoint( p1 );

  // Test if constraint has been applied correctly
  mitk::Point2D cp1 = planarPolygon->GetControlPoint( 1 );
  MITK_TEST_CONDITION( 
    (fabs(cp1[0] - 80.0) < mitk::eps)
    && (fabs(cp1[1] - 100.0) < mitk::eps), "Point2 placed and constrained correctly" );

  // **************************************************************************
  // Add third control point out of bounds (outside of channel defined by first line)
  mitk::Point2D p2;
  p2[0] = 100.0; p2[1] = 100.0;
  planarPolygon->AddControlPoint( p2 );

  // Test if constraint has been applied correctly (100.0, 100.0) must be projected to (90.0, 90.0)
  mitk::Point2D cp2 = planarPolygon->GetControlPoint( 2 );
  MITK_TEST_CONDITION( 
    (fabs(cp2[0] - 90.0) < mitk::eps)
    && (fabs(cp2[1] - 90.0) < mitk::eps), "Point3 placed and constrained correctly" );

  // Move third control point (within channel defined by first line)
  p2[0] = 40.0; p2[1] = 20.0;
  planarPolygon->SetControlPoint( 2, p2 );

  // Test if point is still at this position (no constrained should be applied)
  cp2 = planarPolygon->GetControlPoint( 2 );
  MITK_TEST_CONDITION( 
    (fabs(cp2[0] - 40.0) < mitk::eps)
    && (fabs(cp2[1] - 20.0) < mitk::eps), "Point3 moved correctly" );

  // **************************************************************************
  // Add fourth control point out of bounds (outside of line defined by first line and third point)
  mitk::Point2D p3;
  p3[0] = 20.0; p3[1] = 60.0;
  planarPolygon->AddControlPoint( p3 );

  // Test if constraint has been applied correctly (20.0, 60.0) must be projected to (10.0, 50.0)
  mitk::Point2D cp3 = planarPolygon->GetControlPoint( 3 );
  MITK_TEST_CONDITION( 
    (fabs(cp3[0] - 10.0) < mitk::eps)
    && (fabs(cp3[1] - 50.0) < mitk::eps), "Point4 placed and constrained correctly" );
}





};


/**
 * mitkplanarPolygonTest tests the methods and behavior of mitk::PlanarPolygon with sub-tests:
 *
 * 1. Instantiation and basic tests, including feature evaluation
 * 
 */
int mitkPlanarPolygonTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("planarPolygon")

  // create PlaneGeometry on which to place the planarPolygon
  mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane( 100.0, 100.0 );

  // **************************************************************************
  // 1. Instantiation and basic tests, including feature evaluation
  mitk::PlanarPolygon::Pointer planarPolygon = mitk::PlanarPolygon::New();
  planarPolygon->SetGeometry2D( planeGeometry );

  // first test: did this work?
  MITK_TEST_CONDITION_REQUIRED( planarPolygon.IsNotNull(), "Testing instantiation" );

  // Test placement of planarPolygon by control points
  mitkPlanarPolygonTestClass::TestPlanarPolygonPlacement( planarPolygon );

  // always end with this!
  MITK_TEST_END()
}
