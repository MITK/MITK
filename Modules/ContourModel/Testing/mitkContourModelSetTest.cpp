/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <mitkContourModelSet.h>
#include <mitkTestingMacros.h>

static void TestAddVertex()
{
  mitk::ContourModelSet::Pointer contourSet = mitk::ContourModelSet::New();

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  contourSet->AddContourModel(contour);

  MITK_TEST_CONDITION(contourSet->GetSize() > 0, "Add a contour, size increased");
}

static void TestRemoveContourAtIndex()
{
  mitk::ContourModelSet::Pointer contourSet = mitk::ContourModelSet::New();

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  contourSet->AddContourModel(contour);

  contourSet->RemoveContourModelAt(0);

  MITK_TEST_CONDITION(contourSet->GetSize() == 0, "removed contour by index");

  contourSet->AddContourModel(contour);

  contourSet->RemoveContourModel(contour);

  MITK_TEST_CONDITION(contourSet->GetSize() == 0, "removed contour by object");
}

static void TestEmptyContour()
{
  mitk::ContourModelSet::Pointer contourSet = mitk::ContourModelSet::New();

  MITK_TEST_CONDITION(contourSet->Begin() == contourSet->End(), "test iterator of emtpy contour");

  MITK_TEST_CONDITION(contourSet->GetSize() == 0, "test numberof vertices of empty contour");
}

int mitkContourModelSetTest(int /*argc*/, char * /*argv*/ [])
{
  MITK_TEST_BEGIN("mitkContourModelSetTest")

  TestEmptyContour();
  TestAddVertex();
  TestRemoveContourAtIndex();

  MITK_TEST_END()
}
