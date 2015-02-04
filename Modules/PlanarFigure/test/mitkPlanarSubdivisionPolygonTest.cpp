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
#include "mitkPlanarSubdivisionPolygon.h"
#include "mitkPlaneGeometry.h"
#include "mitkProperties.h"

class mitkPlanarSubdivisionPolygonTestClass
{

public:


static void TestPlanarSubdivisionPolygonPlacement( mitk::PlanarSubdivisionPolygon::Pointer planarSubdivisionPolygon )
{
  // Test for correct minimum number of control points in cross-mode
  MITK_TEST_CONDITION( planarSubdivisionPolygon->GetMinimumNumberOfControlPoints() == 3, "Minimum number of control points" );

  // Test for correct maximum number of control points in cross-mode
  MITK_TEST_CONDITION( planarSubdivisionPolygon->GetMaximumNumberOfControlPoints() == 1000, "Maximum number of control points" );

  // Test for correct rounds of subdivisionPoints
  MITK_TEST_CONDITION( planarSubdivisionPolygon->GetSubdivisionRounds() == 5, "Subdivision point generation depth" );

  // Test for correct tension parameter
  MITK_TEST_CONDITION( planarSubdivisionPolygon->GetTensionParameter() == 0.0625, "Tension parameter" );

  planarSubdivisionPolygon->SetProperty( "initiallyplaced", mitk::BoolProperty::New( true ) );

  // Initial placement of planarSubdivisionPolygon
  mitk::Point2D p0;
  p0[0] = 25.0; p0[1] = 25.0;
  planarSubdivisionPolygon->PlaceFigure( p0 );

  // Add second control point
  mitk::Point2D p1;
  p1[0] = 75.0; p1[1] = 25.0;
  planarSubdivisionPolygon->SetControlPoint(1, p1 );

  // Add third control point
  mitk::Point2D p2;
  p2[0] = 75.0; p2[1] = 75.0;
  planarSubdivisionPolygon->AddControlPoint( p2 );

  // Add fourth control point
  mitk::Point2D p3;
  p3[0] = 25.0; p3[1] = 75.0;
  planarSubdivisionPolygon->AddControlPoint( p3 );

  // Test for number of control points
  MITK_TEST_CONDITION( planarSubdivisionPolygon->GetNumberOfControlPoints() == 4, "Number of control points after placement" );

  // Test if PlanarFigure is closed
  MITK_TEST_CONDITION( planarSubdivisionPolygon->IsClosed(), "Test if property 'closed' is set by default" );

  // Test for number of polylines
  const mitk::PlanarFigure::PolyLineType polyLine0 = planarSubdivisionPolygon->GetPolyLine( 0 );
  mitk::PlanarFigure::PolyLineType::const_iterator iter = polyLine0.begin();
  MITK_TEST_CONDITION( planarSubdivisionPolygon->GetPolyLinesSize() == 1, "Number of polylines after placement" );

  // Test if subdivision point count is correct
  MITK_TEST_CONDITION( polyLine0.size() == 128, "correct number of subdivision points for this depth level" );

  // Test if control points are in correct order between subdivision points
  bool correctPoint = true;
  iter = polyLine0.begin();

  if( static_cast<mitk::Point2D>(*iter) != p0 ){ correctPoint = false; }
  advance(iter, 32);
  if( static_cast<mitk::Point2D>(*iter) != p1 ){ correctPoint = false; }
  advance(iter, 32);
  if( static_cast<mitk::Point2D>(*iter) != p2 ){ correctPoint = false; }
  advance(iter, 32);
  if( static_cast<mitk::Point2D>(*iter) != p3 ){ correctPoint = false; }
  MITK_TEST_CONDITION( correctPoint, "Test if control points are in correct order in polyline" );


  // Test if a picked point has the correct coordinates
  correctPoint = true;

  mitk::Point2D testPoint;
  testPoint[0] = 81.25;
  testPoint[1] = 48.243;
  iter = polyLine0.begin();
  advance(iter, 47);

  mitk::ScalarType testEps = 1E-5;

  if( (static_cast<mitk::Point2D>(*iter)[0] - testPoint[0]) + (static_cast<mitk::Point2D>(*iter)[1] - testPoint[1]) > testEps ){ correctPoint = false; }

  testPoint[0] = 39.624;
  testPoint[1] = 19.3268;
  iter = polyLine0.begin();
  advance(iter, 10);
  if( (static_cast<mitk::Point2D>(*iter)[0] - testPoint[0]) + (static_cast<mitk::Point2D>(*iter)[1] - testPoint[1]) > testEps ){ correctPoint = false; }

  testPoint[0] = 71.2887;
  testPoint[1] = 77.5248;
  iter = polyLine0.begin();
  advance(iter, 67);
  if( (static_cast<mitk::Point2D>(*iter)[0] - testPoint[0]) + (static_cast<mitk::Point2D>(*iter)[1] - testPoint[1]) > testEps ){ correctPoint = false; }

  MITK_TEST_CONDITION( correctPoint, "Test if subdivision points are calculated correctly" )

  // Test for number of measurement features
  /*
  Does not work yet

  planarSubdivisionPolygon->EvaluateFeatures();
  MITK_TEST_CONDITION( planarSubdivisionPolygon->GetNumberOfFeatures() == 2, "Number of measurement features" );

  // Test for correct feature evaluation
  double length0 = 4 * 50.0; // circumference
  MITK_TEST_CONDITION( fabs( planarSubdivisionPolygon->GetQuantity( 0 ) - length0) < mitk::eps, "Size of longest diameter" );

  double length1 = 50.0 * 50.0 ; // area
  MITK_TEST_CONDITION( fabs( planarSubdivisionPolygon->GetQuantity( 1 ) - length1) < mitk::eps, "Size of short axis diameter" );
  */
}

static void TestPlanarSubdivisionPolygonEditing( mitk::PlanarSubdivisionPolygon::Pointer planarSubdivisionPolygon )
{
  unsigned int initialNumberOfControlPoints = planarSubdivisionPolygon->GetNumberOfControlPoints();

  mitk::Point2D pnt;
  pnt[0] = 75.0; pnt[1] = 25.0;
  planarSubdivisionPolygon->AddControlPoint( pnt);

  MITK_TEST_CONDITION( planarSubdivisionPolygon->GetNumberOfControlPoints() == initialNumberOfControlPoints+1, "A new control-point shall be added" );
  MITK_TEST_CONDITION( planarSubdivisionPolygon->GetControlPoint( planarSubdivisionPolygon->GetNumberOfControlPoints()-1 ) == pnt, "Control-point shall be added at the end." );


  planarSubdivisionPolygon->RemoveControlPoint( 3 );
  MITK_TEST_CONDITION( planarSubdivisionPolygon->GetNumberOfControlPoints() == initialNumberOfControlPoints, "A control-point has been removed" );
  MITK_TEST_CONDITION( planarSubdivisionPolygon->GetControlPoint( 3 ) == pnt, "It shall be possible to remove any control-point." );

  planarSubdivisionPolygon->RemoveControlPoint( 0 );
  planarSubdivisionPolygon->RemoveControlPoint( 0 );
  planarSubdivisionPolygon->RemoveControlPoint( 0 );
  MITK_TEST_CONDITION( planarSubdivisionPolygon->GetNumberOfControlPoints() == 3, "Control-points cannot be removed if only three points remain." );

  mitk::Point2D pnt1;
  pnt1[0] = 33.0; pnt1[1] = 33.0;
  planarSubdivisionPolygon->AddControlPoint( pnt1, 0 );
  MITK_TEST_CONDITION( planarSubdivisionPolygon->GetNumberOfControlPoints() == 4, "A control-point has been added" );
  MITK_TEST_CONDITION( planarSubdivisionPolygon->GetControlPoint( 0 ) == pnt1, "It shall be possible to insert a control-point at any position." );

}

};
/**
 * mitkplanarSubdivisionPolygonTest tests the methods and behavior of mitk::planarSubdivisionPolygon with sub-tests:
 *
 * 1. Instantiation and basic tests, including feature evaluation
 *
 */
int mitkPlanarSubdivisionPolygonTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("planarSubdivisionPolygon")

  // create PlaneGeometry on which to place the planarSubdivisionPolygon
  mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane( 100.0, 100.0 );

  // **************************************************************************
  // 1. Instantiation and basic tests, including feature evaluation
  mitk::PlanarSubdivisionPolygon::Pointer planarSubdivisionPolygon = mitk::PlanarSubdivisionPolygon::New();
  planarSubdivisionPolygon->SetPlaneGeometry( planeGeometry );

  // first test: did this work?
  MITK_TEST_CONDITION_REQUIRED( planarSubdivisionPolygon.IsNotNull(), "Testing instantiation" );

  // Test placement of planarSubdivisionPolygon by control points
  mitkPlanarSubdivisionPolygonTestClass::TestPlanarSubdivisionPolygonPlacement( planarSubdivisionPolygon );

  mitkPlanarSubdivisionPolygonTestClass::TestPlanarSubdivisionPolygonEditing( planarSubdivisionPolygon );

  // always end with this!
  MITK_TEST_END();
}
