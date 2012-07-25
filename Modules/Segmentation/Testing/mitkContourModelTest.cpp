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



//select a vertex by index. successful if the selected vertex member of the contour is no longer set to null
static void TestSelectVertexAtIndex()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  contour->SelectVertexAt(0);

  MITK_TEST_CONDITION(contour->GetSelectedVertex() != NULL, "Vertex was selected at index");
}



//select a vertex by worldposition. successful if the selected vertex member of the contour is no longer set to null
static void TestSelectVertexAtWorldposition()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  //same point is used here so the epsilon can be chosen very small
  contour->SelectVertexAt(p, 0.01);

  MITK_TEST_CONDITION(contour->GetSelectedVertex() != NULL, "Vertex was selected at index");
}



//move a vertex by a translation vector
static void TestMoveSelectedVertex()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  //same point is used here so the epsilon can be chosen very small
  contour->SelectVertexAt(p, 0.01);

  mitk::Vector3D v;
  v[0] = 1;
  v[1] = 3;
  v[2] = -1;

  contour->MoveSelectedVertex(v);

  const mitk::ContourModel::VertexType* vertex = contour->GetSelectedVertex();

  bool correctlyMoved = false;

  correctlyMoved = (vertex->Coordinates)[0] == (v[0]) &&
                   (vertex->Coordinates)[1] == (v[1]) &&
                   (vertex->Coordinates)[2] == (v[2]);

  MITK_TEST_CONDITION(correctlyMoved, "Vertex has been moved");
}



//test to move the whole contour
static void TestMoveContour()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);


  mitk::Point3D p2;
  p2[0] = p2[1] = p2[2] = 0;

  contour->AddVertex(p2);


  mitk::Vector3D v;
  v[0] = 1;
  v[1] = 3;
  v[2] = -1;

  contour->MoveContour(v);

  mitk::ContourModel::VertexIterator it = contour->IteratorBegin();
  mitk::ContourModel::VertexIterator end = contour->IteratorEnd();

  bool correctlyMoved = false;

  while(it != end)
  {

    correctlyMoved &= (*it)->Coordinates[0] == (v[0]) &&
      (*it)->Coordinates[1] == (v[1]) &&
      (*it)->Coordinates[2] == (v[2]);
  }

  MITK_TEST_CONDITION(correctlyMoved, "Contour has been moved");
}



//remove a vertex by index
static void TestRemoveVertexAtIndex()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  contour->RemoveVertexAt(0);

  MITK_TEST_CONDITION(contour->GetNumberOfVertices() == 0, "removed vertex");
}



//remove a vertex by position
static void TestRemoveVertexAtWorldPosition()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  contour->RemoveVertexAt(p, 0.01);

  MITK_TEST_CONDITION(contour->GetNumberOfVertices() == 0, "removed vertex");
}



//check closeable contour
static void TestIsclosed()
{
    mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);


  mitk::Point3D p2;
  p2[0] = p2[1] = p2[2] = 1;

  contour->AddVertex(p2);

  contour->Close();

  MITK_TEST_CONDITION(contour->IsClosed(), "close contour");
}



//test concatenating two contours
static void TestConcatenate()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);


  mitk::Point3D p2;
  p2[0] = p2[1] = p2[2] = 1;

  contour->AddVertex(p2);


  mitk::ContourModel::Pointer contour2 = mitk::ContourModel::New();

  mitk::Point3D p3;
  p3[0] = -2;
  p3[1] = 10;
  p3[2] = 0;

  contour2->AddVertex(p3);


  mitk::Point3D p4;
  p4[0] = -3;
  p4[1] = 6;
  p4[2] = -5;

  contour2->AddVertex(p4);

  contour->Concatenate(contour2);

  MITK_TEST_CONDITION(contour->GetNumberOfVertices() == 4, "two contours were concatenated");
}




int mitkContourModelTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkContourModelTest")

    TestAddVertex();
  TestSelectVertexAtIndex();
  TestSelectVertexAtWorldposition();
  TestMoveSelectedVertex();
  TestRemoveVertexAtIndex();
  TestRemoveVertexAtWorldPosition();
  TestIsclosed();
  TestConcatenate();

  MITK_TEST_END()
}