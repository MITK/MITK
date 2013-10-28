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
/*
 *  mitkPlanarFigureComposite.cpp
 *  mitk-all
 *
 *  Created by HAL9000 on 2/4/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "mitkPlanarFigureComposite.h"


mitk::PlanarFigureComposite::PlanarFigureComposite()

{
  m_PFVector = CompositionContainer::New();
  m_DNVector = DataNodeContainer::New();
}

mitk::PlanarFigureComposite::~PlanarFigureComposite()
{

}

mitk::PlanarFigureComposite::PlanarFigureComposite(const Self& other)
  : PlanarFigure(other),
    m_PFVector(other.m_PFVector->Clone()),
    m_compOperation(other.m_compOperation),
    m_DNVector(other.m_DNVector->Clone()),
    m_name(other.m_name)
{
}

void mitk::PlanarFigureComposite::addDataNode(mitk::DataNode::Pointer dnode)
{

  m_DNVector->InsertElement(m_DNVector->Size(), dnode);

}


void mitk::PlanarFigureComposite::addPlanarFigure(PlanarFigure::Pointer pf)
{
  m_PFVector->InsertElement(m_PFVector->Size(), pf);
}

void mitk::PlanarFigureComposite::replaceDataNodeAt(int idx, mitk::DataNode::Pointer dn)
{
  m_DNVector->SetElement( idx, dn );
}

void mitk::PlanarFigureComposite::setOperationType(PFCompositionOperation pfcOp)
{
  this->m_compOperation = pfcOp;
}


mitk::PFCompositionOperation mitk::PlanarFigureComposite::getOperationType()
{
  return this->m_compOperation;


}

void mitk::PlanarFigureComposite::setDisplayName(std::string displName)
{

 m_name = displName;

}


std::string mitk::PlanarFigureComposite::getDisplayName()
{
 return m_name;
}


int mitk::PlanarFigureComposite::getNumberOfChildren()
{
  return m_PFVector->Size();

}

mitk::PlanarFigure::Pointer mitk::PlanarFigureComposite::getChildAt(int idx)
{

  return m_PFVector->ElementAt(idx);
}


mitk::DataNode::Pointer mitk::PlanarFigureComposite::getDataNodeAt(int idx)
{

  return m_DNVector->ElementAt(idx);

}






//musthave implementations from superclass.... not sure if return true makes sense
bool mitk::PlanarFigureComposite::SetControlPoint( unsigned int index, const Point2D &point, bool createIfDoesNotExist )
{
  return true;
}

void mitk::PlanarFigureComposite::GeneratePolyLine()
{

}

void mitk::PlanarFigureComposite::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
  // A circle does not require a helper object
}

void mitk::PlanarFigureComposite::EvaluateFeaturesInternal()
{

}

void mitk::PlanarFigureComposite::PrintSelf( std::ostream& os, itk::Indent indent) const
{

}
