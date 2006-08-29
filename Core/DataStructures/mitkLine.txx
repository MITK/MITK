/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkLine.h"
#include <itkProcessObject.h>
#include <itkSmartPointerForwardReference.txx>


//##ModelId=3E639CD30201
mitk::Line::Line()
{
  m_SlicedGeometry = SlicedGeometry3D::New();
  SetGeometry(m_SlicedGeometry);
  m_SlicedGeometry->Initialize(1);
}

//##ModelId=3E639CD30233
mitk::Line::~Line()
{
}

//##ModelId=3E6423D2030E
void mitk::Line::SetGeometry2D(mitk::Geometry2D *geometry2d)
{
  m_SlicedGeometry->SetGeometry2D(geometry2d, 0);
  m_Geometry2D=geometry2d;
  Modified();
}

//##ModelId=3E66CC5A0295
void mitk::Line::UpdateOutputInformation()
{
  SetPipelineMTime(m_Geometry2D->GetMTime());
  if (this->GetSource())
  {
    this->GetSource()->UpdateOutputInformation();
  }
}

//##ModelId=3E66CC5A02B4
void mitk::Line::SetRequestedRegionToLargestPossibleRegion()
{

}

//##ModelId=3E66CC5A02D2
bool mitk::Line::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  if(m_Geometry2D.IsNull()) return true;

  return false;
}

//##ModelId=3E66CC5A02F0
bool mitk::Line::VerifyRequestedRegion()
{
  if(m_Geometry2D.IsNull()) return false;

  return true;
}

//##ModelId=3E66CC5A030E
void mitk::Line::SetRequestedRegion(itk::DataObject *data)
{

}

//##ModelId=3E67D85E00B7
void mitk::Line::CopyInformation(const itk::DataObject *data)
{
}

