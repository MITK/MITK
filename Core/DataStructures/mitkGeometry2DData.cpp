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


#include "mitkGeometry2DData.h"
#include "mitkBaseProcess.h"

//##ModelId=3E639CD30201
mitk::Geometry2DData::Geometry2DData()
{
}

//##ModelId=3E639CD30233
mitk::Geometry2DData::~Geometry2DData()
{
}

//##ModelId=3E6423D2030E
void mitk::Geometry2DData::SetGeometry2D(mitk::Geometry2D *geometry2d)
{
  m_Geometry3D=geometry2d;
  Modified();
}

//##ModelId=3E66CC5A0295
void mitk::Geometry2DData::UpdateOutputInformation()
{
  if(m_Geometry3D.IsNotNull())
    SetPipelineMTime(m_Geometry3D->GetMTime());
  if (this->GetSource())
  {
    this->GetSource()->UpdateOutputInformation();
  }
}

//##ModelId=3E66CC5A02B4
void mitk::Geometry2DData::SetRequestedRegionToLargestPossibleRegion()
{

}

//##ModelId=3E66CC5A02D2
bool mitk::Geometry2DData::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  if(m_Geometry3D.IsNull()) return true;

  return false;
}

//##ModelId=3E66CC5A02F0
bool mitk::Geometry2DData::VerifyRequestedRegion()
{
  if(m_Geometry3D.IsNull()) return false;

  return true;
}

//##ModelId=3E66CC5A030E
void mitk::Geometry2DData::SetRequestedRegion(itk::DataObject *data)
{

}

//##ModelId=3E67D85E00B7
void mitk::Geometry2DData::CopyInformation(const itk::DataObject *data)
{
}
