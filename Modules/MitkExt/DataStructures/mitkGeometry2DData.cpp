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


#include "mitkGeometry2DData.h"
#include "mitkBaseProcess.h"

mitk::Geometry2DData::Geometry2DData()
{
}

mitk::Geometry2DData::~Geometry2DData()
{
}

void mitk::Geometry2DData::SetGeometry(mitk::Geometry3D *geometry)
{
  if(geometry==NULL)
    SetGeometry2D(NULL);
  else
  {
    Geometry2D* geometry2d = dynamic_cast<Geometry2D*>(geometry);
    if(geometry2d==NULL)
      itkExceptionMacro(<<"Trying to set a geometry which is not a Geometry2D into Geometry2DData.");
    SetGeometry2D(geometry2d);
  }
}

void mitk::Geometry2DData::SetGeometry2D(mitk::Geometry2D *geometry2d)
{
  if(geometry2d != NULL)
  {
    TimeSlicedGeometry* timeSlicedGeometry = GetTimeSlicedGeometry();
    if(timeSlicedGeometry == NULL)
    {
      Superclass::SetGeometry(geometry2d);
      return;
    }
    timeSlicedGeometry->InitializeEvenlyTimed(geometry2d, 1);
    Modified();
  }
  else
    Superclass::SetGeometry(geometry2d);
}

void mitk::Geometry2DData::UpdateOutputInformation()
{
  Superclass::UpdateOutputInformation();
}

void mitk::Geometry2DData::SetRequestedRegionToLargestPossibleRegion()
{

}

bool mitk::Geometry2DData::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  if(GetGeometry2D()==NULL) return true;

  return false;
}

bool mitk::Geometry2DData::VerifyRequestedRegion()
{
  if(GetGeometry2D()==NULL) return false;

  return true;
}

void mitk::Geometry2DData::SetRequestedRegion(itk::DataObject *)
{

}

void mitk::Geometry2DData::CopyInformation(const itk::DataObject *)
{
}
