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

#include "mitkTubeGraphVertex.h"

mitk::TubeGraphVertex::TubeGraphVertex()
: m_element(nullptr)
{
}

mitk::TubeGraphVertex::TubeGraphVertex( mitk::TubeElement* element)
{
  m_element = element;
}

mitk::TubeGraphVertex::~TubeGraphVertex()
{
}


void mitk::TubeGraphVertex::SetTubeElement(mitk::TubeElement* element)
{
  m_element = element;
}

const mitk::TubeElement* mitk::TubeGraphVertex::GetTubeElement() const
{
  return m_element;
}

bool mitk::TubeGraphVertex::operator==(const mitk::TubeGraphVertex& right) const
{
  return (m_element == right.m_element);
}
