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

void mitk::PlanarFigureComposite::addDataNode(mitk::DataNode::Pointer dnode)
{
 
  m_DNVector->InsertElement(m_DNVector->Size(), dnode);
  
}


void mitk::PlanarFigureComposite::addPlanarFigure(PlanarFigure::Pointer pf)
{
  
  MITK_INFO << "addPlanarFigure: size before: " << this->getNumberOfChildren();
  m_PFVector->InsertElement(m_PFVector->Size(), pf);
  MITK_INFO << "addPlanarFigure: size after: " << this->getNumberOfChildren();

  
  
  
}

void mitk::PlanarFigureComposite::replaceDataNodeAt(int idx, mitk::DataNode::Pointer dn)
{
  MITK_INFO << "replace: size before: " << this->getNumberOfChildren();
  m_DNVector->SetElement( idx, dn );
   MITK_INFO << "replace: size after: " << this->getNumberOfChildren();

}

void mitk::PlanarFigureComposite::setOperationType(PFCompositionOperation pfcOp)
{
  this->m_compOperation = pfcOp;
  MITK_INFO << "Composition set to: " << this->getOperationType();
  
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
