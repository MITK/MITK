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

#include "mitkPlanarFigureComposite.h"


mitk::PlanarFigureComposite::PlanarFigureComposite()
{
    mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
    this->SetGeometry(geometry);
}

mitk::PlanarFigureComposite::~PlanarFigureComposite()
{

}

void mitk::PlanarFigureComposite::setOperationType(OperationType pfcOp)
{
  this->m_compOperation = pfcOp;
}


mitk::PlanarFigureComposite::OperationType mitk::PlanarFigureComposite::getOperationType() const
{
  return this->m_compOperation;
}

/* ESSENTIAL IMPLEMENTATION OF SUPERCLASS METHODS */

void mitk::PlanarFigureComposite::SetRequestedRegionToLargestPossibleRegion()
{

}
bool mitk::PlanarFigureComposite::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return false;
}
bool mitk::PlanarFigureComposite::VerifyRequestedRegion()
{
    return true;
}
void mitk::PlanarFigureComposite::SetRequestedRegion(const itk::DataObject* )
{

}

