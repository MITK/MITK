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
#include "mitkPlanarCross.h"
#include "mitkPlaneGeometry.h"


class mitkPlanarCrossTestClass
{

public:



static void TestPlanarCrossPlacement( mitk::PlanarCross::Pointer planarCross )
{
  // Test for correct minimum number of control points in cross-mode
  MITK_TEST_CONDITION( planarCross->GetMinimumNumberOfControlPoints() == 4, "Minimum number of control points" );

  // Test for correct maximum number of control points in cross-mode
  MITK_TEST_CONDITION( planarCross->GetMaximumNumberOfControlPoints() == 4, "Maximum number of control points" );

  // Initial placement of PlanarCross
  mitk::Point2D p0;
  p0[0] = 20.0; p0[1] = 20.0;
  planarCross->PlaceFigure( p0 );

  // Add second control point
  mitk::Point2D p1;
  p1[0] = 80.0; p1[1] = 80.0;
  planarCross->SetCurrentControlPoint( p1 );

  // Add third control point
  mitk::Point2D p2;
  p2[0] = 90.0; p2[1] = 10.0;
  planarCross->AddControlPoint( p2 );

  // Test if helper polyline is generated
  const mitk::PlanarFigure::VertexContainerType* helperPolyLine = planarCross->GetHelperPolyLine( 0, 1.0, 100 );
  MITK_TEST_CONDITION( planarCross->GetHelperPolyLinesSize() == 1, "Number of helper polylines after placing 3 points" );

  // Test if helper polyline is marked as "to be painted"
  MITK_TEST_CONDITION( planarCross->IsHelperToBePainted( 0 ), "Helper line to be painted after placing 3 points" );

  // Test if helper polyline is orthogonal to first line
  mitk::Vector2D v0 = p1 - p0;
  v0.Normalize();
  mitk::Vector2D hv = helperPolyLine->ElementAt( 1 ) - helperPolyLine->ElementAt( 0 );
  hv.Normalize();
  MITK_TEST_CONDITION( fabs(v0 * hv) < mitk::eps, "Helper line is orthogonal to first line" );

  // Test if helper polyline is placed correctly
  mitk::Vector2D hv1 = helperPolyLine->ElementAt( 1 ) - p2;
  hv1.Normalize();
  MITK_TEST_CONDITION( fabs(hv * hv1 - 1.0) < mitk::eps, "Helper line is aligned to third point" );


  // Add fourth control point
  mitk::Point2D p3;
  p3[0] = 10.0; p3[1] = 90.0;
  planarCross->AddControlPoint( p3 );

  // Test for number of control points
  MITK_TEST_CONDITION( planarCross->GetNumberOfControlPoints() == 4, "Number of control points after placement" );

  // Test if PlanarFigure is closed
  MITK_TEST_CONDITION( !planarCross->IsClosed(), "Is PlanarFigure closed?" );

  // Test if helper polyline is no longer marked as "to be painted"
  MITK_TEST_CONDITION( planarCross->IsHelperToBePainted( 0 ), "Helper line no longer to be painted after placement of all 4 points" );


  // Test for number of polylines
  MITK_TEST_CONDITION( planarCross->GetPolyLinesSize() == 2, "Number of polylines after placement" );

  // Get polylines and check if the generated coordinates are OK
  const mitk::PlanarFigure::VertexContainerType* polyLine0 = planarCross->GetPolyLine( 0 );
  const mitk::Point2D& pp0 = polyLine0->ElementAt( 0 );
  const mitk::Point2D& pp1 = polyLine0->ElementAt( 1 );
  MITK_TEST_CONDITION( (pp0 == p0) && (pp1 == p1)
    || (pp0 == p1) && (pp1 == p0), "Correct polyline 1" );

  const mitk::PlanarFigure::VertexContainerType* polyLine1 = planarCross->GetPolyLine( 1 );
  const mitk::Point2D& pp2 = polyLine1->ElementAt( 0 );
  const mitk::Point2D& pp3 = polyLine1->ElementAt( 1 );
  MITK_TEST_CONDITION( (pp2 == p2) && (pp3 == p3)
    || (pp2 == p3) && (pp3 == p2), "Correct polyline 2" );

  
  // Test for number of measurement features
  planarCross->EvaluateFeatures();
  MITK_TEST_CONDITION( planarCross->GetNumberOfFeatures() == 2, "Number of measurement features" );

  // Test for correct feature evaluation
  double length0 = sqrt( 80.0 * 80.0 * 2.0 );
  MITK_TEST_CONDITION( fabs( planarCross->GetQuantity( 0 ) - length0) < mitk::eps, "Size of longest diameter" );

  double length1 = sqrt( 60.0 * 60.0 * 2.0 );
  MITK_TEST_CONDITION( fabs( planarCross->GetQuantity( 1 ) - length1) < mitk::eps, "Size of short axis diameter" );
}

static void TestPlanarCrossPlacementSingleLine(mitk::PlanarCross::Pointer planarCross)
{
  // * single line placement
  //   --> get min/max number of control points
  //   --> poly line generation
  //   --> helper poly line generation
  //   --> get number of features
  //   --> feature evaluation
}

static void TestPlanarCrossPlacementConstrained(mitk::PlanarCross::Pointer planarCross)
{
  // * cross placement with spatial constraints (both...)
}

static void TestPlanarCrossEdit(mitk::PlanarCross::Pointer planarCross)
{
  // * point move (different points)
  //   --> reset
  //   --> test which point is where / evaluation
}


};

int mitkPlanarCrossTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("PlanarCross")

  // create PlaneGeometry on which to place the PlanarCross
  mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane( 100.0, 100.0 );

  // instantiate PlanarCross
  mitk::PlanarCross::Pointer planarCross = mitk::PlanarCross::New();
  planarCross->SetGeometry2D( planeGeometry );

  // first test: did this work?
  MITK_TEST_CONDITION_REQUIRED( planarCross.IsNotNull(), "Testing instantiation" );

  // test: default cross-mode (not single-line-mode)?
  MITK_TEST_CONDITION_REQUIRED( !planarCross->GetSingleLineMode(), "Testing default cross mode" );



  // Test placement of PlanarCross by control points
  mitkPlanarCrossTestClass::TestPlanarCrossPlacement( planarCross );
  
  // always end with this!
  MITK_TEST_END()
}
