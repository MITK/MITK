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

  mitk::Point3D point;
  point[0] = point[1] = point[2] = 0;

  contour->AddVertex(point);

  MITK_TEST_CONDITION(contour->GetNumberOfVertices() > 0, "Add a Vertex, size increased");

}



//select a vertex by index. successful if the selected vertex member of the contour is no longer set to null
static void TestSelectVertexAtIndex()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D point;
  point[0] = point[1] = point[2] = 0;

  contour->AddVertex(point);

  contour->SelectVertexAt(0);

  MITK_TEST_CONDITION(contour->GetSelectedVertex() != NULL, "Vertex was selected at index");
}



//select a vertex by worldposition. successful if the selected vertex member of the contour is no longer set to null
static void TestSelectVertexAtWorldposition()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D point;
  point[0] = point[1] = point[2] = 0;

  contour->AddVertex(point);

  //same point is used here so the epsilon can be chosen very small
  contour->SelectVertexAt(point, 0.01);

  MITK_TEST_CONDITION(contour->GetSelectedVertex() != NULL, "Vertex was selected at index");
}



//move a vertex by a translation vector
static void TestMoveSelectedVertex()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D point;
  point[0] = point[1] = point[2] = 0;

  contour->AddVertex(point);

  //same point is used here so the epsilon can be chosen very small
  contour->SelectVertexAt(point, 0.01);

  mitk::Vector3D v;
  v[0] = 1;
  v[1] = 3;
  v[2] = -1;

  contour->MoveSelectedVertex(v);

  const mitk::ContourModel::VertexType* vertex = contour->GetSelectedVertex();

  bool correctlyMoved = false;

  correctlyMoved = (*(vertex->Coordinates))[0] == (v[0]) &&
                   (*(vertex->Coordinates))[1] == (v[1]) &&
                   (*(vertex->Coordinates))[2] == (v[2]);

  MITK_TEST_CONDITION(correctlyMoved, "Vertex has been moved");
}



int mitkContourModelTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkContourModelTest")

    TestAddVertex();
  TestSelectVertexAtIndex();
  TestSelectVertexAtWorldposition();
  MITK_TEST_END()
}