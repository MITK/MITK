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



//Select a vertex by index. successful if the selected vertex member of the contour is no longer set to null
static void TestSelectVertexAtIndex()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  contour->SelectVertexAt(0);

  MITK_TEST_CONDITION(contour->GetSelectedVertex() != nullptr, "Vertex was selected at index");
}



//Select a vertex by worldposition. successful if the selected vertex member of the contour is no longer set to null
static void TestSelectVertexAtWorldposition()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  //same point is used here so the epsilon can be chosen very small
  contour->SelectVertexAt(p, 0.01);

  MITK_TEST_CONDITION(contour->GetSelectedVertex() != nullptr, "Vertex was selected at position");
}



//Move a vertex by a translation vector
static void TestMoveSelectedVertex()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  //Same point is used here so the epsilon can be chosen very small
  contour->SelectVertexAt(p, 0.01);

  mitk::Vector3D v;
  v[0] = 1;
  v[1] = 3;
  v[2] = -1;

  contour->ShiftSelectedVertex(v);

  const mitk::ContourModel::VertexType* vertex = contour->GetSelectedVertex();

  bool correctlyMoved = false;

  correctlyMoved = (vertex->Coordinates)[0] == (v[0]) &&
    (vertex->Coordinates)[1] == (v[1]) &&
    (vertex->Coordinates)[2] == (v[2]);

  MITK_TEST_CONDITION(correctlyMoved, "Vertex has been moved");
}



//Test to move the whole contour
/*
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

  contour->ShiftContour(v);

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
*/


//Remove a vertex by index
static void TestRemoveVertexAtIndex()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  contour->RemoveVertexAt(0);

  MITK_TEST_CONDITION(contour->GetNumberOfVertices() == 0, "removed vertex");
}



//Remove a vertex by position
static void TestRemoveVertexAtWorldPosition()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  contour->RemoveVertexAt(p, 0.01);

  MITK_TEST_CONDITION(contour->GetNumberOfVertices() == 0, "removed vertex");
}



//Check closeable contour
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

  MITK_TEST_CONDITION(contour->IsClosed(), "closed contour");

  //no vertices should be added to a closed contour
  int oldNumberOfVertices = contour->GetNumberOfVertices();

  mitk::Point3D p3;
  p3[0] = p3[1] = p3[2] = 4;

  contour->AddVertex(p3);

  int newNumberOfVertices = contour->GetNumberOfVertices();

  MITK_TEST_CONDITION(oldNumberOfVertices != newNumberOfVertices, "vertices added to closed contour");
}



//Test concatenating two contours
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



//Try to select a vertex at position (within a epsilon of course) where no vertex is.
//So the selected verted member should be null.
static void TestSelectVertexAtWrongPosition()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  MITK_TEST_CONDITION_REQUIRED(contour->GetSelectedVertex() == nullptr, "selected vertex is NULL");
  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  mitk::Point3D p2;
  p2[0] = p2[1] = p2[2] = 2;

  contour->SelectVertexAt(p2, 0.1);

  MITK_TEST_CONDITION(contour->GetSelectedVertex() == nullptr, "Vertex was not selected");
}


static void TestInsertVertex()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);


  mitk::Point3D p2;
  p2[0] = p2[1] = p2[2] = 1;

  contour->AddVertex(p2);

  mitk::Point3D pointToInsert;
  pointToInsert[0] = pointToInsert[1] = pointToInsert[2] = 10;

  contour->InsertVertexAtIndex(pointToInsert, 1);

  MITK_TEST_CONDITION(contour->GetNumberOfVertices() == 3, "test insert vertex");

  MITK_TEST_CONDITION(contour->GetVertexAt(1)->Coordinates == pointToInsert, "compare inserted vertex");

}


//try to access an invalid timestep
static void TestInvalidTimeStep()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();
  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);


  mitk::Point3D p2;
  p2[0] = p2[1] = p2[2] = 1;

  contour->AddVertex(p2);

  int invalidTimeStep = 42;

  MITK_TEST_CONDITION_REQUIRED(contour->IsEmptyTimeStep(invalidTimeStep), "invalid timestep required");

  MITK_TEST_FOR_EXCEPTION(std::exception, contour->IteratorBegin(-1));

  contour->Close(invalidTimeStep);
  MITK_TEST_CONDITION(contour->IsClosed() == false, "test close for timestep 0");
  MITK_TEST_CONDITION(contour->IsClosed(invalidTimeStep) == false, "test close at invalid timestep");

  contour->SetClosed(true, invalidTimeStep);
  MITK_TEST_CONDITION(contour->GetNumberOfVertices(invalidTimeStep) == -1, "test number of vertices at invalid timestep");

  contour->AddVertex(p2, invalidTimeStep);
  MITK_TEST_CONDITION(contour->GetNumberOfVertices(invalidTimeStep) == -1, "test add vertex at invalid timestep");

  contour->InsertVertexAtIndex(p2, 0, false, invalidTimeStep);
  MITK_TEST_CONDITION(contour->GetNumberOfVertices(invalidTimeStep) == -1, "test insert vertex at invalid timestep");

  MITK_TEST_CONDITION(contour->SelectVertexAt(0, invalidTimeStep) == false, "test select vertex at invalid timestep");

  MITK_TEST_CONDITION(contour->RemoveVertexAt(0, invalidTimeStep) == false, "test remove vertex at invalid timestep");

}


static void TestEmptyContour()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  MITK_TEST_CONDITION(contour->IteratorBegin() == contour->IteratorEnd(), "test iterator of emtpy contour");

  MITK_TEST_CONDITION(contour->GetNumberOfVertices() == 0, "test numberof vertices of empty contour");
}


static void TestSetVertices()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  mitk::Point3D newCoordinates;

  newCoordinates[0] = newCoordinates[1] = newCoordinates[2] = 1;

  contour->SetVertexAt(0, newCoordinates );

  MITK_TEST_EQUAL(contour->GetVertexAt(0)->Coordinates, newCoordinates, "set coordinates" );

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

  contour->AddVertex(p);

  contour->SetVertexAt(1, contour2->GetVertexAt(1));

  MITK_TEST_EQUAL(contour->GetVertexAt(1)->Coordinates, contour2->GetVertexAt(1)->Coordinates, "Use setter and getter combination");

}


static void TestContourModelAPI()
{
  mitk::ContourModel::Pointer contour1 = mitk::ContourModel::New();

  mitk::Point3D p1;
  p1[0] = -2;
  p1[1] = 10;
  p1[2] = 0;

  contour1->AddVertex(p1);

  //adding vertices should always copy the content and not store pointers or references.
  MITK_TEST_CONDITION( &p1 != &(contour1->GetVertexAt(0)->Coordinates), "copied point" );


  mitk::Point3D p2;
  p2[0] = -3;
  p2[1] = 6;
  p2[2] = -5;

  contour1->AddVertex(p2);

  //test use of setter and getter with const and non-const pointers
  const mitk::ContourModel::VertexType* vertex = contour1->GetVertexAt(1);

  MITK_TEST_CONDITION(contour1->GetIndex(vertex) == 1, "Get index");

  mitk::ContourModel::VertexType* nonConstVertex = const_cast<mitk::ContourModel::VertexType*>(vertex);

  MITK_TEST_CONDITION(contour1->GetIndex(nonConstVertex) == 1, "Get index non-const");


  mitk::ContourModel::Pointer contour2 = mitk::ContourModel::New();

  contour2->AddVertex(contour1->GetVertexAt(0));

  MITK_TEST_CONDITION(contour2->GetNumberOfVertices() == 1, "Add call with another contour");
}


int mitkContourModelTest(int /*argc*/, char* /*argv*/[])
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
  TestInvalidTimeStep();
  TestInsertVertex();
  TestEmptyContour();
  TestSetVertices();
  TestSelectVertexAtWrongPosition();
  TestContourModelAPI();

  MITK_TEST_END()
}
