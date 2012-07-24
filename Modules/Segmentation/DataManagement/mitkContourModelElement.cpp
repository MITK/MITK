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
#include <mitkContourModelElement.h>

mitk::ContourModelElement::ContourModelElement()
{

}

mitk::ContourModelElement::ContourModelElement(mitk::ContourModelElement &other)
{

}

mitk::ContourModelElement::~ContourModelElement()
{

}


void mitk::ContourModelElement::AddVertex(mitk::Point3D* vertex)
{
  
}

mitk::ContourModelElement::VertexType* mitk::ContourModelElement::GetVertexAt(int index)
{
  return NULL;
}

mitk::ContourModelElement::VertexType* mitk::ContourModelElement::GetVertexAt(mitk::Point3D* point)
{
  return NULL;
}

std::deque<mitk::ContourModelElement::VertexType*>* mitk::ContourModelElement::GetVertexList()
{
  return NULL;
}

bool mitk::ContourModelElement::IsClosed()
{
  return NULL;
}

void mitk::ContourModelElement::Concatenate(mitk::ContourModelElement* other)
{

}

bool mitk::ContourModelElement::RemoveVertex(mitk::ContourModelElement::VertexType* vertex)
{
  return NULL;
}

bool mitk::ContourModelElement::RemoveVertexAt(int index)
{
  return NULL;
}

bool mitk::ContourModelElement::RemoveVertexAt(mitk::Point3D* point)
{
  return NULL;
}