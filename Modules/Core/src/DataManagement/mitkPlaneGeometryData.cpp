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


#include "mitkPlaneGeometryData.h"
#include "mitkBaseProcess.h"
#include <mitkProportionalTimeGeometry.h>
#include "mitkAbstractTransformGeometry.h"

mitk::PlaneGeometryData::PlaneGeometryData()
{
}

mitk::PlaneGeometryData::~PlaneGeometryData()
{
}

void mitk::PlaneGeometryData::SetGeometry(mitk::BaseGeometry *geometry)
{
  if(geometry==nullptr)
    SetPlaneGeometry(nullptr);
  else
  {
    PlaneGeometry* geometry2d = dynamic_cast<PlaneGeometry*>(geometry);
    if(geometry2d==nullptr || dynamic_cast<AbstractTransformGeometry*>(geometry)!=nullptr)
      itkExceptionMacro(<<"Trying to set a geometry which is not a PlaneGeometry into PlaneGeometryData.");
    SetPlaneGeometry(geometry2d);
  }
}

void mitk::PlaneGeometryData::SetPlaneGeometry(mitk::PlaneGeometry *geometry2d)
{
  if(geometry2d != nullptr)
  {
    ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
    timeGeometry->Initialize(geometry2d, 1);
    SetTimeGeometry(timeGeometry);
    Modified();
  }
  else
    Superclass::SetGeometry(geometry2d);
}

void mitk::PlaneGeometryData::UpdateOutputInformation()
{
  Superclass::UpdateOutputInformation();
}

void mitk::PlaneGeometryData::SetRequestedRegionToLargestPossibleRegion()
{

}

bool mitk::PlaneGeometryData::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  if(GetPlaneGeometry()==nullptr) return true;

  return false;
}

bool mitk::PlaneGeometryData::VerifyRequestedRegion()
{
  if(GetPlaneGeometry()==nullptr) return false;

  return true;
}

void mitk::PlaneGeometryData::SetRequestedRegion( const itk::DataObject *)
{

}

void mitk::PlaneGeometryData::CopyInformation(const itk::DataObject *)
{
}
