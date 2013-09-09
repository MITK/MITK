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
#include <mitkTestingMacros.h>
#include <mitkContourModel.h>


//Add a vertex to the contour and see if size changed
static void TestAddVertex()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  MITK_TEST_CONDITION(contour->GetNumberOfVertices() > 0, "Add a Vertex, size increased");

}



//Remove a vertex by index
static void TestRemoveContourAtIndex()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  contour->RemoveVertexAt(0);

  MITK_TEST_CONDITION(contour->GetNumberOfVertices() == 0, "removed vertex");
}



static void TestEmptyContour()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  MITK_TEST_CONDITION(contour->IteratorBegin() == contour->IteratorEnd(), "test iterator of emtpy contour");

  MITK_TEST_CONDITION(contour->GetNumberOfVertices() == 0, "test numberof vertices of empty contour");
}



int mitkContourModelSetTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkContourModelTest")

    TestEmptyContour();
    TestAddVertex();
    TestRemoveContourAtIndex();

  MITK_TEST_END()
}
