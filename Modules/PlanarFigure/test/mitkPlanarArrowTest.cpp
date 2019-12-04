/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarArrow.h"
#include "mitkPlaneGeometry.h"
#include "mitkTestingMacros.h"

class mitkPlanarArrowTestClass
{
public:
  static void TestPlanarArrowPlacement(mitk::PlanarArrow::Pointer PlanarArrow)
  {
    // Test for correct minimum number of control points in cross-mode
    MITK_TEST_CONDITION(PlanarArrow->GetMinimumNumberOfControlPoints() == 2, "Minimum number of control points");

    // Test for correct maximum number of control points in cross-mode
    MITK_TEST_CONDITION(PlanarArrow->GetMaximumNumberOfControlPoints() == 2, "Maximum number of control points");

    // Initial placement of PlanarArrow
    mitk::Point2D p0;
    p0[0] = 00.0;
    p0[1] = 0.0;
    PlanarArrow->PlaceFigure(p0);

    // Add second control point
    mitk::Point2D p1;
    p1[0] = 50.0;
    p1[1] = 00.0;
    PlanarArrow->SetControlPoint(1, p1);

    // Test for number of control points
    MITK_TEST_CONDITION(PlanarArrow->GetNumberOfControlPoints() == 2, "Number of control points after placement");

    // Test for number of polylines
    const mitk::PlanarFigure::PolyLineType polyLine0 = PlanarArrow->GetPolyLine(0);
    auto iter = polyLine0.begin();
    MITK_TEST_CONDITION(PlanarArrow->GetPolyLinesSize() == 1, "Number of polylines after placement");

    // Get polylines and check if the generated coordinates are OK
    const mitk::Point2D &pp0 = *iter;
    iter++;
    const mitk::Point2D &pp1 = *iter;
    MITK_TEST_CONDITION((pp0 == p0) && (pp1 == p1), "Correct polyline 1");

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
int mitkPlanarArrowTest(int /* argc */, char * /*argv*/ [])
{
  // always start with this!
  MITK_TEST_BEGIN("PlanarArrow")

  // create PlaneGeometry on which to place the PlanarArrow
  mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane(100.0, 100.0);

  // **************************************************************************
  // 1. Instantiation and basic tests
  mitk::PlanarArrow::Pointer PlanarArrow = mitk::PlanarArrow::New();
  PlanarArrow->SetPlaneGeometry(planeGeometry);

  // first test: did this work?
  MITK_TEST_CONDITION_REQUIRED(PlanarArrow.IsNotNull(), "Testing instantiation");

  // Test placement of PlanarArrow by control points
  mitkPlanarArrowTestClass::TestPlanarArrowPlacement(PlanarArrow);
  PlanarArrow->EvaluateFeatures();

  mitk::PlanarArrow::Pointer clonedArrow = PlanarArrow->Clone();
  MITK_TEST_CONDITION_REQUIRED(clonedArrow.IsNotNull(), "Testing cloning");
  bool identical(true);
  identical &= clonedArrow->GetMinimumNumberOfControlPoints() == PlanarArrow->GetMinimumNumberOfControlPoints();
  identical &= clonedArrow->GetMaximumNumberOfControlPoints() == PlanarArrow->GetMaximumNumberOfControlPoints();
  identical &= clonedArrow->IsClosed() == PlanarArrow->IsClosed();
  identical &= clonedArrow->IsPlaced() == PlanarArrow->IsPlaced();
  identical &= clonedArrow->GetNumberOfControlPoints() == PlanarArrow->GetNumberOfControlPoints();
  identical &= clonedArrow->GetNumberOfControlPoints() == PlanarArrow->GetNumberOfControlPoints();
  identical &= clonedArrow->GetSelectedControlPoint() == PlanarArrow->GetSelectedControlPoint();
  identical &= clonedArrow->IsPreviewControlPointVisible() == PlanarArrow->IsPreviewControlPointVisible();
  identical &= clonedArrow->GetPolyLinesSize() == PlanarArrow->GetPolyLinesSize();
  identical &= clonedArrow->GetHelperPolyLinesSize() == PlanarArrow->GetHelperPolyLinesSize();
  identical &= clonedArrow->ResetOnPointSelect() == PlanarArrow->ResetOnPointSelect();

  for (unsigned int i = 0; i < clonedArrow->GetNumberOfControlPoints(); ++i)
  {
    identical &= clonedArrow->GetControlPoint(i) == PlanarArrow->GetControlPoint(i);
  }

  for (unsigned int i = 0; i < clonedArrow->GetPolyLinesSize(); ++i)
  {
    mitk::PlanarFigure::PolyLineType polyLine = clonedArrow->GetPolyLine(i);
    for (unsigned int j = 0; j < polyLine.size(); ++j)
    {
      identical &= polyLine.at(j) == PlanarArrow->GetPolyLine(i).at(j);
    }
  }

  MITK_TEST_CONDITION_REQUIRED(identical, "Cloning completely successful");

  // always end with this!
  MITK_TEST_END();
}
