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

#include "mitkTubeGraphGeometry3D.h"

mitk::TubeGraphGeometry3D::TubeGraphGeometry3D()
{
}

void mitk::TubeGraphGeometry3D::Initialize( mitk::TubeGraph* graph )
{
  if (!graph)
  {
    itkWarningMacro("The passed graph is empty. Trying to continue...");
    return;
  }

  m_TubeGraph = graph;

  BoundingBox::ConstPointer        bb = this->GetBoundingBox();
  BoundingBox::BoundsArrayType bounds = bb->GetBounds();

  Superclass::Initialize();
}


const mitk::BoundingBox* mitk::TubeGraphGeometry3D::GetBoundingBox( ) const
{
  if (!m_TubeGraph)
  {
    itkWarningMacro("Graph is empty! Trying to continue...");
  }

  BoundingBox::Pointer boundingBox = BoundingBox::New();
  BoundingBox::PointIdentifier pointid = 0;
  BoundingBox::PointsContainer::Pointer pointscontainer = BoundingBox::PointsContainer::New();

  ScalarType nullpoint[] = { 0,0,0 };
  BoundingBox::PointType p(nullpoint);

  //traverse the tree and add each point to the pointscontainer

  mitk::Point3D pos;

  std::vector<mitk::TubeGraphVertex> vertexVector = m_TubeGraph->GetVectorOfAllVertices();
  for(std::vector<mitk::TubeGraphVertex>::iterator vertex = vertexVector.begin(); vertex != vertexVector.end(); ++vertex)
  {
    pos =  vertex->GetTubeElement()->GetCoordinates();
    p[0] = pos[0];
    p[1] = pos[1];
    p[2] = pos[2];
    pointscontainer->InsertElement(pointid++, p);

  }

  std::vector<mitk::TubeGraphEdge> edgeVector = m_TubeGraph->GetVectorOfAllEdges();

  for(std::vector<mitk::TubeGraphEdge>::iterator edge = edgeVector.begin(); edge != edgeVector.end(); ++edge)
  {
    std::vector<mitk::TubeElement*> allElements = edge->GetElementVector();
    for(unsigned int index =0; index < edge->GetNumberOfElements(); index++)
    {
      pos = allElements[index]->GetCoordinates();
      p[0] = pos[0];
      p[1] = pos[1];
      p[2] = pos[2];
      pointscontainer->InsertElement(pointid++, p);

    }
  }

  boundingBox->SetPoints(pointscontainer);
  boundingBox->ComputeBoundingBox();

  m_BoundingBox = boundingBox;

  return boundingBox.GetPointer();
}
