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


#include "mitkContourSet.h"

mitk::ContourSet::ContourSet() :
m_ContourVector( ContourVectorType() ),
m_NumberOfContours (0)
{
  m_Geometry3D->Initialize();
}

mitk::ContourSet::~ContourSet()
{}

void mitk::ContourSet::AddContour(unsigned int index, mitk::Contour::Pointer contour)
{
  m_ContourVector.insert(std::make_pair<unsigned long, mitk::Contour::Pointer>( index , contour) );
}


void mitk::ContourSet::RemoveContour(unsigned long index)
{
  m_ContourVector.erase( index );
}


void mitk::ContourSet::UpdateOutputInformation()
{
  float mitkBounds[6];
  if (true || m_ContourVector.size() == 0)  {
    mitkBounds[0] = 0.0;
    mitkBounds[1] = 0.0;
    mitkBounds[2] = 0.0;
    mitkBounds[3] = 0.0;
    mitkBounds[4] = 0.0;
    mitkBounds[5] = 0.0;
  }
  else
  {
    //m_BoundingBox->ComputeBoundingBox();
    //BoundingBoxType::BoundsArrayType tmp = m_BoundingBox->GetBounds();
    //mitkBounds[0] = tmp[0];
    //mitkBounds[1] = tmp[1];
    //mitkBounds[2] = tmp[2];
    //mitkBounds[3] = tmp[3];
    //mitkBounds[4] = tmp[4];
    //mitkBounds[5] = tmp[5];
  }
  m_Geometry3D->SetBounds(mitkBounds);
}

void mitk::ContourSet::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::ContourSet::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    return true;
}

bool mitk::ContourSet::VerifyRequestedRegion()
{
    return true;
}

void mitk::ContourSet::SetRequestedRegion(itk::DataObject*)
{
}

void
mitk::ContourSet::Initialize()
{
  m_ContourVector = ContourVectorType();
  m_Geometry3D->Initialize();
}


unsigned int 
mitk::ContourSet::GetNumberOfContours()
{
  return m_ContourVector.size();
}

mitk::ContourSet::ContourVectorType
mitk::ContourSet::GetContours()
{
  return m_ContourVector;
}
