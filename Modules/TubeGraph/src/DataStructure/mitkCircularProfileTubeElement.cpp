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

#include "mitkCircularProfileTubeElement.h"

mitk::CircularProfileTubeElement::CircularProfileTubeElement()
: m_coordinates()
, m_diameter(0.0f)
{
}

mitk::CircularProfileTubeElement::CircularProfileTubeElement(float x, float y, float z, float d)
:m_diameter(d)
{
  mitk::FillVector3D(m_coordinates, x, y, z);
}

mitk::CircularProfileTubeElement::CircularProfileTubeElement(const mitk::Point3D c, float d)
: m_coordinates(c)
, m_diameter(d)
{
}

mitk::CircularProfileTubeElement::~CircularProfileTubeElement()
{
}

const mitk::Point3D& mitk::CircularProfileTubeElement::GetCoordinates() const
{
  return m_coordinates;
}

void mitk::CircularProfileTubeElement::SetCoordinates(mitk::Point3D coordinates)
{
  m_coordinates = coordinates;
}

const float mitk::CircularProfileTubeElement::GetDiameter() const
{
  return m_diameter;
}

void mitk::CircularProfileTubeElement::SetDiameter(float d)
{
  m_diameter = d;
}

bool mitk::CircularProfileTubeElement::operator== (const mitk::TubeElement& right)const
{
  //Check if given TubeElement is CircularProfileTubeElement
  if (dynamic_cast<const mitk::CircularProfileTubeElement*>(&right))
  {
    const mitk::CircularProfileTubeElement* element = dynamic_cast< const mitk::CircularProfileTubeElement*>(&right);
    return ((m_diameter == element->m_diameter) && (m_coordinates == element->m_coordinates));
  }
  else
    return false;
}
