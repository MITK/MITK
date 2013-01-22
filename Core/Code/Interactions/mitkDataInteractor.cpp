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

#include "mitkDataInteractor.h"
#include "mitkInteractor.h"
#include "mitkPointSet.h"
#include "mitkDataNode.h"
#include "mitkStateMachineState.h"

mitk::DataInteractor::DataInteractor()
{
  m_DataNode = NULL;
  m_SelectionAccuracy = 3.5;
}

mitk::DataNode::Pointer mitk::DataInteractor::GetDataNode()
{
  return m_DataNode;
}

void mitk::DataInteractor::SetDataNode(DataNode::Pointer dataNode)
{
  mitk::DataNode::Pointer tmpDN = m_DataNode;
  m_DataNode = dataNode;
  if (dataNode.IsNotNull())
  {
    dataNode->SetDataInteractor(this);
  }
  else if (tmpDN.IsNotNull())
  { // if DataInteractors' DataNode is set to null, the "old" DataNode has to be notified (else the Dispatcher won't be notified either)
    tmpDN->SetDataInteractor(NULL);
  }
  // notify implementations ...
  DataNodeChanged();
}

int mitk::DataInteractor::GetLayer()
{
  int layer = -1;
  if (m_DataNode.IsNotNull())
  {
    m_DataNode->GetIntProperty("layer", layer);
  }
  return layer;
}

mitk::DataInteractor::~DataInteractor()
{
  if (m_DataNode.IsNotNull())
  {
    m_DataNode->SetInteractor(NULL);
    m_DataNode = NULL;
  }
}

void mitk::DataInteractor::SetAccuracy(float accuracy)
{
  m_SelectionAccuracy = accuracy;
}

void mitk::DataInteractor::ConnectActionsAndFunctions()
{
  MITK_WARN<< "ConnectActionsAndFunctions in DataInteractor not implemented.\n DataInteractor will not be able to process any events.";
}

std::string mitk::DataInteractor::GetMode()
{
  return GetCurrentState()->GetMode();
}

int mitk::DataInteractor::GetPointIndexByPosition(Point3D position, int time)
{

  // iterate over point set and check if it contains a point close enough to the pointer to be selected
  PointSet* points = dynamic_cast<PointSet*>(GetDataNode()->GetData());
  int index = -1;
  if (points == NULL)
  {
    return index;
  }
  PointSet::PointsContainer* pointsContainer = points->GetPointSet(time)->GetPoints();

  float minDistance = m_SelectionAccuracy;
  for (PointSet::PointsIterator it = pointsContainer->Begin(); it != pointsContainer->End(); it++)
  {
    float distance = sqrt(position.SquaredEuclideanDistanceTo(points->GetPoint(it->Index(), time)));  // TODO: support time!
    if (distance < minDistance) // if several points fall within the margin, choose the one with minimal distance to position
    { // TODO: does this make sense, which unit is it?
      index = it->Index();
    }
  }
  return index;
}

void mitk::DataInteractor::DataNodeChanged()
{
}
