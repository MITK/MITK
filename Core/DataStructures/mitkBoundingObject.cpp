/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkBoundingObject.h"
#include "mitkBaseProcess.h"

mitk::BoundingObject::BoundingObject()
  : Surface(), m_Positive(true)
{
  m_Geometry3D->Initialize();
}

mitk::BoundingObject::~BoundingObject() 
{
} 

//void mitk::BoundingObject::SetRequestedRegionToLargestPossibleRegion()
//{
//}
//
//bool mitk::BoundingObject::RequestedRegionIsOutsideOfTheBufferedRegion()
//{
//  return ! VerifyRequestedRegion();
//}
//
//bool mitk::BoundingObject::VerifyRequestedRegion()
//{
//  assert(m_Geometry3D.IsNotNull());
//  return true;
//}

void mitk::BoundingObject::UpdateOutputInformation()
{  
  if ( this->GetSource() )
  {
    this->GetSource()->UpdateOutputInformation();
  }
  ScalarType bounds[6]={0,1,0,1,0,1};  //{xmin,x_max, ymin,y_max,zmin,z_max}
  /* bounding box around the unscaled bounding object */ 
  bounds[0] = - 1;
  bounds[1] = + 1;
  bounds[2] = - 1;
  bounds[3] = + 1;
  bounds[4] = - 1;
  bounds[5] = + 1; 
  m_Geometry3D->SetBounds(bounds);
}

mitk::ScalarType mitk::BoundingObject::GetVolume()
{
  return 0.0;
}
