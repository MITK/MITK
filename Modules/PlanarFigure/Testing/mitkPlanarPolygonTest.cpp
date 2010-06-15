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
  MITK_TEST_END();
} 
