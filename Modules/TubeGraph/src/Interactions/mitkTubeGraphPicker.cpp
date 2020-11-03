/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTubeGraphPicker.h"

mitk::TubeGraphPicker::TubeGraphPicker()
{
  m_WorldPosition.Fill(0.0);
}

mitk::TubeGraphPicker::~TubeGraphPicker() {}

void mitk::TubeGraphPicker::SetTubeGraph(const mitk::TubeGraph *tubeGraph)
{
  m_TubeGraph = tubeGraph;
  m_TubeGraphProperty =
    dynamic_cast<TubeGraphProperty *>(m_TubeGraph->GetProperty("Tube Graph.Visualization Information").GetPointer());
}

/**
 * Implements the picking process
 */
std::pair<mitk::TubeGraph::TubeDescriptorType, mitk::TubeElement *> mitk::TubeGraphPicker::GetPickedTube(
  const Point3D pickedPosition)
{
  if (!m_TubeGraph)
  {
    MITK_ERROR << "mitk::TubeGraphPicker: No tube graph available. Please set an input!" << std::endl;
    mitk::TubeElement *nullPointer = nullptr;
    return std::pair<mitk::TubeGraph::TubeDescriptorType, mitk::TubeElement *>(TubeGraph::ErrorId, nullPointer);
  }
  m_WorldPosition = pickedPosition;

  Point3D currentPosition;
  ScalarType closestDistance = itk::NumericTraits<ScalarType>::max();
  ScalarType currentDistance = itk::NumericTraits<ScalarType>::max();
  float currentRadius = 0;

  TubeGraph::TubeDescriptorType currentTubeId(TubeGraph::ErrorId);
  TubeGraph::TubeDescriptorType tubeId(TubeGraph::ErrorId);
  TubeElement *tubeElement;

  // iterate over all edges and find the edge, which element is near by the clicked point
  std::vector<mitk::TubeGraphEdge> allEdges = m_TubeGraph->GetVectorOfAllEdges();
  for (auto edge = allEdges.begin(); edge != allEdges.end(); ++edge)
  {
    std::pair<mitk::TubeGraphVertex, mitk::TubeGraphVertex> soureTargetPair =
      m_TubeGraph->GetVerticesOfAnEdge(m_TubeGraph->GetEdgeDescriptor(*edge));

    currentTubeId = TubeGraph::TubeDescriptorType(m_TubeGraph->GetVertexDescriptor(soureTargetPair.first),
                                                  m_TubeGraph->GetVertexDescriptor(soureTargetPair.second));
    // check if the tube is visible, if not pass this tube. User can not choose a tube, which he can't see
    if (m_TubeGraphProperty->IsTubeVisible(currentTubeId))
    {
      std::vector<mitk::TubeElement *> allElements = edge->GetElementVector();
      for (unsigned int index = 0; index < edge->GetNumberOfElements(); index++)
      {
        currentPosition = allElements[index]->GetCoordinates();
        if (dynamic_cast<mitk::CircularProfileTubeElement *>(allElements[index]))
          currentRadius = ((dynamic_cast<mitk::CircularProfileTubeElement *>(allElements[index]))->GetDiameter()) / 2;
        else
          currentRadius = 0;

        // calculate point->point distance
        itk::Index<3> worldIndex;
        m_TubeGraph->GetGeometry()->WorldToIndex(pickedPosition, worldIndex);

        m_WorldPosition[0] = worldIndex[0];
        m_WorldPosition[1] = worldIndex[1];
        m_WorldPosition[2] = worldIndex[2];

        currentDistance = m_WorldPosition.EuclideanDistanceTo(currentPosition);
        if (currentDistance < closestDistance && (currentDistance - currentRadius) < 1.0)
        {
          closestDistance = currentDistance;
          tubeId = currentTubeId;
          tubeElement = allElements[index];
        }
      }
    }
  }

  return std::make_pair(tubeId, tubeElement);
}
