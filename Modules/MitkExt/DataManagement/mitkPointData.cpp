/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include "mitkPointData.h"
#include <itkProcessObject.h>
#include <mitkBaseProcess.h>
#include <itkSmartPointerForwardReference.txx>


mitk::PointData::PointData()
{
   m_Point3D[0] = 0;
   m_Point3D[1] = 0;
   m_Point3D[2] = 0;

   m_Point2D[0] = 0;
   m_Point2D[1] = 0;
}

mitk::PointData::~PointData()
{

}


void mitk::PointData::UpdateOutputInformation()
{
    if ( this->GetSource() )
    {
        this->GetSource()->UpdateOutputInformation();
    }
}
  
void  mitk::PointData::SetRequestedRegionToLargestPossibleRegion()
{

}
  
bool  mitk::PointData::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    if ( VerifyRequestedRegion() == false )
        return true;
    else
        return false;
}
  
bool  mitk::PointData::VerifyRequestedRegion()
{
   return true;
}
 
void  mitk::PointData::SetRequestedRegion(itk::DataObject *)
{

}

void  mitk::PointData::CopyInformation(const itk::DataObject *data)
{
    Superclass::CopyInformation(data);
}

