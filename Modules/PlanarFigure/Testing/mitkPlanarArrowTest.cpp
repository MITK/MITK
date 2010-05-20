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
#include "mitkPlanarArrow.h"
#include "mitkPlaneGeometry.h"


class mitkPlanarArrowTestClass
{

public:


static void TestPlanarArrowPlacement( mitk::PlanarArrow::Pointer PlanarArrow )
{
  // Test for correct minimum number of control points in cross-mode
  MITK_TEST_CONDITION( PlanarArrow->GetMinimumNumberOfControlPoints() == 2, "Minimum number of control points" );

  // Test for correct maximum number of control points in cross-mode
  MITK_TEST_CONDITION( PlanarArrow->GetMaximumNumberOfControlPoints() == 2, "Maximum number of control points" );

  // Initial placement of PlanarArrow
  mitk::Point2D p0;
  p0[0] = 00.0; p0[1] = 0.0;
  PlanarArrow->PlaceFigure( p0 );

  // Add second control point
  mitk::Point2D p1;
  p1[0] = 50.0; p1[1] = 00.0;
  PlanarArrow->SetControlPoint(1, p1 );

  // Test for number of control points
  MITK_TEST_CONDITION( PlanarArrow->GetNumberOfControlPoints() == 2, "Number of control points after placement" );

  // Test for number of polylines
  const mitk::PlanarFigure::VertexContainerType* polyLine0 = PlanarArrow->GetPolyLine( 0 );
  MITK_TEST_CONDITION( PlanarArrow->GetPolyLinesSize() == 1, "Number of polylines after placement" );

  // Get polylines and check if the generated coordinates are OK
  const mitk::Point2D& pp0 = polyLine0->ElementAt( 0 );
  const mitk::Point2D& pp1 = polyLine0->ElementAt( 1 );
  MITK_TEST_CONDITION( ((pp0 == p0) && (pp1 == p1))
    || ((pp0 == p1) && (pp1 == p0)), "Correct polyline 1" );

    
  // Test for number of measurement features
  // none yet 

}
};
/**
 * mitkPlanarArrowTest tests the methods and behavior of mitk::PlanarArrow with sub-tests:
 *
 * 1. Instantiation and basic tests
 * 
 */
int mitkPlanarArrowTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("PlanarArrow")

  // create PlaneGeometry on which to place the PlanarArrow
  mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane( 100.0, 100.0 );

  // **************************************************************************
  // 1. Instantiation and basic tests
  mitk::PlanarArrow::Pointer PlanarArrow = mitk::PlanarArrow::New();
  PlanarArrow->SetGeometry2D( planeGeometry );

  // first test: did this work?
  MITK_TEST_CONDITION_REQUIRED( PlanarArrow.IsNotNull(), "Testing instantiation" );

  // Test placement of PlanarArrow by control points
  mitkPlanarArrowTestClass::TestPlanarArrowPlacement( PlanarArrow );

  // always end with this!
  MITK_TEST_END();
} 
