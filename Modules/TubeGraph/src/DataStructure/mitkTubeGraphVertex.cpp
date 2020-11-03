/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTubeGraphVertex.h"

mitk::TubeGraphVertex::TubeGraphVertex() : m_element(nullptr)
{
}

mitk::TubeGraphVertex::TubeGraphVertex(mitk::TubeElement *element)
{
  m_element = element;
}

mitk::TubeGraphVertex::~TubeGraphVertex()
{
}

void mitk::TubeGraphVertex::SetTubeElement(mitk::TubeElement *element)
{
  m_element = element;
}

const mitk::TubeElement *mitk::TubeGraphVertex::GetTubeElement() const
{
  return m_element;
}

bool mitk::TubeGraphVertex::operator==(const mitk::TubeGraphVertex &right) const
{
  return (m_element == right.m_element);
}
