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
  const mitk::PlanarFigure::PolyLineType helperPolyLine = planarCross->GetHelperPolyLine( 0, 1.0, 100 );
  MITK_TEST_CONDITION( planarCross->GetHelperPolyLinesSize() == 1, "Number of helper polylines after placing 3 points" );

  // Test if helper polyline is marked as "to be painted"
  MITK_TEST_CONDITION( planarCross->IsHelperToBePainted( 0 ), "Helper line to be painted after placing 3 points" );

  // Test if helper polyline is orthogonal to first line
  mitk::Vector2D v0 = p1 - p0;
  v0.Normalize();

  // TODO: make it work again

  //mitk::Vector2D hv = helperPolyLine->ElementAt( 1 ) - helperPolyLine->ElementAt( 0 );
  //hv.Normalize();
  //MITK_TEST_CONDITION( fabs(v0 * hv) < mitk::eps, "Helper line is orthogonal to first line" );

  //// Test if helper polyline is placed correctly
  //mitk::Vector2D hv1 = helperPolyLine->ElementAt( 1 ) - p2;
  //hv1.Normalize();
  //MITK_TEST_CONDITION( fabs(hv * hv1 - 1.0) < mitk::eps, "Helper line is aligned to third point" );


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
  const mitk::PlanarFigure::PolyLineType polyLine0 = planarCross->GetPolyLine( 0 );
  const mitk::PlanarFigure::PolyLineType polyLine1 = planarCross->GetPolyLine( 1 );
  MITK_TEST_CONDITION( planarCross->GetPolyLinesSize() == 2, "Number of polylines after placement" );

  mitk::PlanarFigure::PolyLineType::const_iterator iter0 = polyLine0.begin();
  mitk::PlanarFigure::PolyLineType::const_iterator iter1 = polyLine1.begin();

  // Get polylines and check if the generated coordinates are OK
  const mitk::Point2D& pp0 = *iter0;
  iter0++;
  const mitk::Point2D& pp1 = *iter0;
  MITK_TEST_CONDITION( ((pp0 == p0) && (pp1 == p1))
    || ((pp0 == p1) && (pp1 == p0)), "Correct polyline 1" );

  const mitk::Point2D& pp2 = *iter1;
  iter1++;
  const mitk::Point2D& pp3 = *iter1;
  MITK_TEST_CONDITION( ((pp2 == p2) && (pp3 == p3))
    || ((pp2 == p3) && (pp3 == p2)), "Correct polyline 2" );


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
  // Test for correct minimum number of control points in cross-mode
  MITK_TEST_CONDITION( planarCross->GetMinimumNumberOfControlPoints() == 2, "Minimum number of control points" );

  // Test for correct maximum number of control points in cross-mode
  MITK_TEST_CONDITION( planarCross->GetMaximumNumberOfControlPoints() == 2, "Maximum number of control points" );

  // Initial placement of PlanarCross
  mitk::Point2D p0;
  p0[0] = 25.0; p0[1] = 10.0;
  planarCross->PlaceFigure( p0 );

  // Add second control point
  mitk::Point2D p1;
  p1[0] = 30.0; p1[1] = 60.0;
  planarCross->SetCurrentControlPoint( p1 );

  // Verify that no helper line is drawn
  MITK_TEST_CONDITION( planarCross->IsHelperToBePainted( 0 ) == false, "No helper line to be painted in single-line mode" );

  // Test for number of control points
  MITK_TEST_CONDITION( planarCross->GetNumberOfControlPoints() == 2, "Number of control points after placement" );

  // Test if PlanarFigure is closed
  MITK_TEST_CONDITION( !planarCross->IsClosed(), "Is PlanarFigure closed?" );


  // Test for number of polylines
  const mitk::PlanarFigure::PolyLineType polyLine0 = planarCross->GetPolyLine( 0 );
  mitk::PlanarFigure::PolyLineType::const_iterator iter = polyLine0.begin();
  MITK_TEST_CONDITION( planarCross->GetPolyLinesSize() == 1, "Number of polylines after placement" );

  // Get polylines and check if the generated coordinates are OK
  const mitk::Point2D& pp0 = *iter;
  iter++;
  const mitk::Point2D& pp1 = *iter;
  MITK_TEST_CONDITION( ((pp0 == p0) && (pp1 == p1))
    || ((pp0 == p1) && (pp1 == p0)), "Correct polyline 1" );


  // Test for number of measurement features
  planarCross->EvaluateFeatures();
  MITK_TEST_CONDITION( planarCross->GetNumberOfFeatures() == 1, "Number of measurement features" );

  // Test for correct feature evaluation
  double length0 = sqrt( 5.0 * 5.0 + 50.0 * 50.0 );
  MITK_TEST_CONDITION( fabs( planarCross->GetQuantity( 0 ) - length0) < mitk::eps, "Size of diameter" );

  // Test if reset called on single-line PlanarCross returns false (nothing to reset)
  MITK_TEST_CONDITION( planarCross->ResetOnPointSelect() == false, "Single-line PlanarCross should not be reset on point edit" );
}


static void TestPlanarCrossPlacementConstrained(mitk::PlanarCross::Pointer planarCross)
{
  // **************************************************************************
  // Place first control point out of bounds (to the left of the image bounds)
  mitk::Point2D p0;
  p0[0] = -20.0; p0[1] = 20.0;
  planarCross->PlaceFigure( p0 );

  // Test if constraint has been applied correctly
  mitk::Point2D cp0 = planarCross->GetControlPoint( 0 );
  MITK_TEST_CONDITION(
       (fabs(cp0[0]) < mitk::eps)
    && (fabs(cp0[1] - 20.0) < mitk::eps), "Point1 placed and constrained correctly" );

  // **************************************************************************
  // Add second control point out of bounds (to the top of the image bounds)
  mitk::Point2D p1;
  p1[0] = 80.0; p1[1] = 120.0;
  planarCross->SetCurrentControlPoint( p1 );

  // Test if constraint has been applied correctly
  mitk::Point2D cp1 = planarCross->GetControlPoint( 1 );
  MITK_TEST_CONDITION(
    (fabs(cp1[0] - 80.0) < mitk::eps)
    && (fabs(cp1[1] - 100.0) < mitk::eps), "Point2 placed and constrained correctly" );

  // **************************************************************************
  // Add third control point out of bounds (outside of channel defined by first line)
  mitk::Point2D p2;
  p2[0] = 100.0; p2[1] = 100.0;
  planarCross->AddControlPoint( p2 );

  // Test if constraint has been applied correctly (100.0, 100.0) must be projected to (90.0, 90.0)
  mitk::Point2D cp2 = planarCross->GetControlPoint( 2 );
  MITK_TEST_CONDITION(
    (fabs(cp2[0] - 90.0) < mitk::eps)
    && (fabs(cp2[1] - 90.0) < mitk::eps), "Point3 placed and constrained correctly" );

  // Move third control point (within channel defined by first line)
  p2[0] = 40.0; p2[1] = 20.0;
  planarCross->SetControlPoint( 2, p2 );

  // Test if point is still at this position (no constrained should be applied)
  cp2 = planarCross->GetControlPoint( 2 );
  MITK_TEST_CONDITION(
    (fabs(cp2[0] - 40.0) < mitk::eps)
    && (fabs(cp2[1] - 20.0) < mitk::eps), "Point3 moved correctly" );

  // **************************************************************************
  // Add fourth control point out of bounds (outside of line defined by first line and third point)
  mitk::Point2D p3;
  p3[0] = 20.0; p3[1] = 60.0;
  planarCross->AddControlPoint( p3 );

  // Test if constraint has been applied correctly (20.0, 60.0) must be projected to (10.0, 50.0)
  mitk::Point2D cp3 = planarCross->GetControlPoint( 3 );
  MITK_TEST_CONDITION(
    (fabs(cp3[0] - 10.0) < mitk::eps)
    && (fabs(cp3[1] - 50.0) < mitk::eps), "Point4 placed and constrained correctly" );

  // Move fourth control point (to a position which would result in two non-intersecting line
  // without the constraint that lines have to intersect)
  p3[0] = 40.0; p3[1] = 30.0;
  planarCross->SetControlPoint( 3, p3 );

  // Test if constrained point is on the projected intersection point of both lines (20.0/40.0)
  cp3 = planarCross->GetControlPoint( 3 );
  MITK_TEST_CONDITION(
    (fabs(cp3[0] - 20.0) < mitk::eps)
    && (fabs(cp3[1] - 40.0) < mitk::eps), "Point4 placed and constrained correctly" );
}


static void TestPlanarCrossEdit(mitk::PlanarCross::Pointer planarCross)
{
  // * point move (different points)
  //   --> reset
  //   --> test which point is where / evaluation
  mitk::Point2D p0 = planarCross->GetControlPoint( 0 );
  mitk::Point2D p1 = planarCross->GetControlPoint( 1 );
  mitk::Point2D p2 = planarCross->GetControlPoint( 2 );
  mitk::Point2D p3 = planarCross->GetControlPoint( 3 );

  // **************************************************************************
  // Edit control point 0
  planarCross->SelectControlPoint( 0 );

  // Request reset and check if it is done
  MITK_TEST_CONDITION( planarCross->ResetOnPointSelect(), "Editing control point 0: Double-line PlanarCross should be reset" );

  // Check number of control points
  MITK_TEST_CONDITION( planarCross->GetNumberOfControlPoints() == 2, "Two control points are left" );

  // Check if correct control points have been left
  MITK_TEST_CONDITION(
       (planarCross->GetControlPoint( 0 ).EuclideanDistanceTo( p1 ) < mitk::eps)
    && (planarCross->GetControlPoint( 1 ).EuclideanDistanceTo( p0 ) < mitk::eps),
    "Reset to expected control points (p1, p0)" );

  // Reset planar cross to original values
  ResetPlanarCross( planarCross, p0, p1, p2, p3 );


  // **************************************************************************
  // Edit control point 1
  planarCross->SelectControlPoint( 1 );

  // Request reset and check if it is done
  MITK_TEST_CONDITION( planarCross->ResetOnPointSelect(), "Editing control point 1: Double-line PlanarCross should be reset" );

  // Check number of control points
  MITK_TEST_CONDITION( planarCross->GetNumberOfControlPoints() == 2, "Two control points are left" );

  // Check if correct control points have been left
  MITK_TEST_CONDITION(
    (planarCross->GetControlPoint( 0 ).EuclideanDistanceTo( p0 ) < mitk::eps)
    && (planarCross->GetControlPoint( 1 ).EuclideanDistanceTo( p1 ) < mitk::eps),
    "Reset to expected control points (p0, p1)" );

  // Reset planar cross to original values
  ResetPlanarCross( planarCross, p0, p1, p2, p3 );


  // **************************************************************************
  // Edit control point 2
  planarCross->SelectControlPoint( 2 );

  // Request reset and check if it is done
  MITK_TEST_CONDITION( planarCross->ResetOnPointSelect(), "Editing control point 2: Double-line PlanarCross should be reset" );

  // Check number of control points
  MITK_TEST_CONDITION( planarCross->GetNumberOfControlPoints() == 2, "Two control points are left" );

  // Check if correct control points have been left
  MITK_TEST_CONDITION(
    (planarCross->GetControlPoint( 0 ).EuclideanDistanceTo( p3 ) < mitk::eps)
    && (planarCross->GetControlPoint( 1 ).EuclideanDistanceTo( p2 ) < mitk::eps),
    "Reset to expected control points (p3, p2)" );

  // Reset planar cross to original values
  ResetPlanarCross( planarCross, p0, p1, p2, p3 );


  // **************************************************************************
  // Edit control point 3
  planarCross->SelectControlPoint( 3 );

  // Request reset and check if it is done
  MITK_TEST_CONDITION( planarCross->ResetOnPointSelect(), "Editing control point 3: Double-line PlanarCross should be reset" );

  // Check number of control points
  MITK_TEST_CONDITION( planarCross->GetNumberOfControlPoints() == 2, "Two control points are left" );

  // Check if correct control points have been left
  MITK_TEST_CONDITION(
    (planarCross->GetControlPoint( 0 ).EuclideanDistanceTo( p2 ) < mitk::eps)
    && (planarCross->GetControlPoint( 1 ).EuclideanDistanceTo( p3 ) < mitk::eps),
    "Reset to expected control points (p2, p3)" );
}


static void ResetPlanarCross( mitk::PlanarCross::Pointer planarCross,
  mitk::Point2D p0, mitk::Point2D p1, mitk::Point2D p2, mitk::Point2D p3 )
{
  planarCross->SetControlPoint( 0, p0, true );
  planarCross->SetControlPoint( 1, p1, true );
  planarCross->SetControlPoint( 2, p2, true );
  planarCross->SetControlPoint( 3, p3, true );
}


};


/**
 * mitkPlanarCrossTest tests the methods and behavior of mitk::PlanarCross with four sub-tests:
 *
 * 1. Double-line mode instantiation and basic tests
 * 2. Single-line mode instantiation and basic tests
 * 3. Tests of application of spatial constraints for double-line mode
 * 4. Tests if editing of PlanarCross works as intended
 *
 */
int mitkPlanarCrossTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("PlanarCross")

  // create PlaneGeometry on which to place the PlanarCross
  mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane( 100.0, 100.0 );

  // **************************************************************************
  // 1. Double-line mode instantiation and basic tests
  mitk::PlanarCross::Pointer planarCross = mitk::PlanarCross::New();
  planarCross->SetPlaneGeometry( planeGeometry );

  // first test: did this work?
  MITK_TEST_CONDITION_REQUIRED( planarCross.IsNotNull(), "Testing instantiation" );

  // test: default cross-mode (not single-line-mode)?
  MITK_TEST_CONDITION_REQUIRED( !planarCross->GetSingleLineMode(), "Testing default cross mode" );


  // Test placement of PlanarCross by control points
  mitkPlanarCrossTestClass::TestPlanarCrossPlacement( planarCross );


  // **************************************************************************
  // 2. Single-line mode instantiation and basic tests
  planarCross = mitk::PlanarCross::New();
  planarCross->SingleLineModeOn();
  planarCross->SetPlaneGeometry( planeGeometry );

  // test: single-line mode?
  MITK_TEST_CONDITION_REQUIRED( planarCross->GetSingleLineMode(), "Testing activation of single-line mode" );

  // Test placement of single-line PlanarCross by control points
  mitkPlanarCrossTestClass::TestPlanarCrossPlacementSingleLine( planarCross );


  // **************************************************************************
  // 3. Tests of application of spatial constraints for double-line mode
  planarCross = mitk::PlanarCross::New();
  planarCross->SetPlaneGeometry( planeGeometry );

  // Test placement with various out-of-bounds control points (automatic application of
  // constraints expected)
  mitkPlanarCrossTestClass::TestPlanarCrossPlacementConstrained( planarCross );


  // **************************************************************************
  // 4. Tests if editing of PlanarCross works as intended
  mitkPlanarCrossTestClass::TestPlanarCrossEdit( planarCross );



  // always end with this!
  MITK_TEST_END()
}
