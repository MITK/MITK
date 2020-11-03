/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTubeGraphEdge.h"

mitk::TubeGraphEdge::TubeGraphEdge() : m_ElementVector(0)
{
}

mitk::TubeGraphEdge::~TubeGraphEdge()
{
  /*while(!m_ElementVector.empty())
  {
  delete m_ElementVector.back();
  m_ElementVector.pop_back();
  }*/
  m_ElementVector.clear();
}

std::vector<mitk::TubeElement *> mitk::TubeGraphEdge::GetElementVector()
{
  return m_ElementVector;
}

void mitk::TubeGraphEdge::SetElementVector(std::vector<mitk::TubeElement *> elementVector)
{
  m_ElementVector = elementVector;
}

unsigned int mitk::TubeGraphEdge::GetNumberOfElements() const
{
  return m_ElementVector.size();
}

void mitk::TubeGraphEdge::AddTubeElement(mitk::TubeElement *element)
{
  m_ElementVector.push_back(element);
}

void mitk::TubeGraphEdge::AddTubeElementAt(unsigned int position, TubeElement *element)
{
  m_ElementVector.insert(m_ElementVector.begin() + position, element);
}

mitk::TubeElement *mitk::TubeGraphEdge::GetTubeElement(unsigned int position)
{
  if (position >= GetNumberOfElements())
    throw std::runtime_error("Exception thrown in TubeGraphEdge::getTubeElement(uint): getTubeElement(uint)");
  else
    return m_ElementVector[position];
}

float mitk::TubeGraphEdge::GetEdgeLength(mitk::TubeGraphVertex &source, mitk::TubeGraphVertex &target)
{
  float length = 0.0f;
  mitk::Point3D pos1, pos2;
  std::vector<float> diff;
  diff.resize(3);
  unsigned int numberOfElements = this->GetNumberOfElements();

  // if there are no elements return 0
  if (numberOfElements == 0)
  {
    // If the edge has no elements, then take the difference between the vertices directly
    pos1 = (source.GetTubeElement())->GetCoordinates();
    pos2 = (target.GetTubeElement())->GetCoordinates();

    diff[0] = pos1[0] - pos2[0];
    diff[1] = pos1[1] - pos2[1];
    diff[2] = pos1[2] - pos2[2];

    float sum = diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2];
    length = length + std::sqrt(sum);
  }
  else
  {
    // difference between source and first element of the edge
    pos1 = (source.GetTubeElement())->GetCoordinates();
    pos2 = (this->GetTubeElement(0))->GetCoordinates();

    diff[0] = pos1[0] - pos2[0];
    diff[1] = pos1[1] - pos2[1];
    diff[2] = pos1[2] - pos2[2];

    float sum = diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2];
    length = length + std::sqrt(sum);

    // Differences within the edge
    for (unsigned int i = 0; i < numberOfElements - 1; ++i)
    {
      pos1 = (this->GetTubeElement(i))->GetCoordinates();
      pos2 = (this->GetTubeElement(i + 1))->GetCoordinates();

      diff[0] = pos1[0] - pos2[0];
      diff[1] = pos1[1] - pos2[1];
      diff[2] = pos1[2] - pos2[2];

      sum = diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2];
      length = length + std::sqrt(sum);
    }

    // difference between last element and target of the edge
    pos1 = (this->GetTubeElement(numberOfElements - 1))->GetCoordinates();
    pos2 = (target.GetTubeElement())->GetCoordinates();

    diff[0] = pos1[0] - pos2[0];
    diff[1] = pos1[1] - pos2[1];
    diff[2] = pos1[2] - pos2[2];

    sum = diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2];
    length = length + std::sqrt(sum);
  }

  return length;
}

float mitk::TubeGraphEdge::GetEdgeAverageDiameter(mitk::TubeGraphVertex &source, mitk::TubeGraphVertex &target)
{
  float diameterSum = 0.0;
  unsigned int numberOfElements = this->GetNumberOfElements();

  // add source diameter
  if (dynamic_cast<const mitk::CircularProfileTubeElement *>(source.GetTubeElement()))
    diameterSum += (dynamic_cast<const mitk::CircularProfileTubeElement *>(source.GetTubeElement()))->GetDiameter();
  ;

  // add all edge element diameter
  for (unsigned int i = 0; i < numberOfElements; i++)
  {
    if (dynamic_cast<const mitk::CircularProfileTubeElement *>(this->GetTubeElement(i)))
      diameterSum += (dynamic_cast<const mitk::CircularProfileTubeElement *>(this->GetTubeElement(i)))->GetDiameter();
  }

  // add target diameter
  if (dynamic_cast<const mitk::CircularProfileTubeElement *>(target.GetTubeElement()))
    diameterSum += (dynamic_cast<const mitk::CircularProfileTubeElement *>(target.GetTubeElement()))->GetDiameter();

  return diameterSum / (numberOfElements + 2);
}

bool mitk::TubeGraphEdge::operator==(const mitk::TubeGraphEdge &right) const
{
  return (m_ElementVector == right.m_ElementVector);
}
