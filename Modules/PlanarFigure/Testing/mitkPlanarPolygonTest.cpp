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
#include "mitkPlanarPolygon.h"
#include "mitkPlaneGeometry.h"


class mitkPlanarPolygonTestClass
{

public:


static void TestPlanarPolygonPlacement( mitk::PlanarPolygon::Pointer planarPolygon )
{
  // Test for correct minimum number of control points in cross-mode
  MITK_TEST_CONDITION( planarPolygon->GetMinimumNumberOfControlPoints() == 3, "Minimum number of control points" );

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
  const mitk::PlanarFigure::PolyLineType polyLine0 = planarPolygon->GetPolyLine( 0 );
  mitk::PlanarFigure::PolyLineType::const_iterator iter = polyLine0.begin();
  MITK_TEST_CONDITION( planarPolygon->GetPolyLinesSize() == 1, "Number of polylines after placement" );

  // Get polylines and check if the generated coordinates are OK
  const mitk::Point2D& pp0 = *iter;
  ++iter;
  const mitk::Point2D& pp1 = *iter;
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

static void TestPlanarPolygonEditing( mitk::PlanarPolygon::Pointer planarPolygon )
{
  unsigned int initialNumberOfControlPoints = planarPolygon->GetNumberOfControlPoints();

  mitk::Point2D pnt;
  pnt[0] = 75.0; pnt[1] = 25.0;
  planarPolygon->AddControlPoint( pnt);

  MITK_TEST_CONDITION( planarPolygon->GetNumberOfControlPoints() == initialNumberOfControlPoints+1, "A new control-point shall be added" );
  MITK_TEST_CONDITION( planarPolygon->GetControlPoint( planarPolygon->GetNumberOfControlPoints()-1 ) == pnt, "Control-point shall be added at the end." );


  planarPolygon->RemoveControlPoint( 3 );
  MITK_TEST_CONDITION( planarPolygon->GetNumberOfControlPoints() == initialNumberOfControlPoints, "A control-point has been removed" );
  MITK_TEST_CONDITION( planarPolygon->GetControlPoint( 3 ) == pnt, "It shall be possible to remove any control-point." );

  planarPolygon->RemoveControlPoint( 0 );
  planarPolygon->RemoveControlPoint( 0 );
  planarPolygon->RemoveControlPoint( 0 );
  MITK_TEST_CONDITION( planarPolygon->GetNumberOfControlPoints() == 3, "Control-points cannot be removed if only three points remain." );

  mitk::Point2D pnt1;
  pnt1[0] = 33.0; pnt1[1] = 33.0;
  planarPolygon->AddControlPoint( pnt1, 0 );
  MITK_TEST_CONDITION( planarPolygon->GetNumberOfControlPoints() == 4, "A control-point has been added" );
  MITK_TEST_CONDITION( planarPolygon->GetControlPoint( 0 ) == pnt1, "It shall be possible to insert a control-point at any position." );

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
  planarPolygon->SetPlaneGeometry( planeGeometry );

  // first test: did this work?
  MITK_TEST_CONDITION_REQUIRED( planarPolygon.IsNotNull(), "Testing instantiation" );

  // Test placement of planarPolygon by control points
  mitkPlanarPolygonTestClass::TestPlanarPolygonPlacement( planarPolygon );

  mitkPlanarPolygonTestClass::TestPlanarPolygonEditing( planarPolygon );

  // always end with this!
  MITK_TEST_END();
}
