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

#include "mitkPlanesPerpendicularToLinesFilter.h"

#include <vnl/vnl_cross.h>
#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.txx>

mitk::PlanesPerpendicularToLinesFilter::PlanesPerpendicularToLinesFilter()
  : m_Plane(NULL), m_CreatedGeometries(NULL)
{
}

mitk::PlanesPerpendicularToLinesFilter::~PlanesPerpendicularToLinesFilter()
{
}

void mitk::PlanesPerpendicularToLinesFilter::GenerateOutputInformation()
{
  mitk::Mesh::ConstPointer input  = this->GetInput();
  mitk::GeometryData::Pointer output = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  if(input.IsNull()) return;

  output->SetGeometry(static_cast<Geometry3D*>(input->GetGeometry()->Clone().GetPointer()));
}

void mitk::PlanesPerpendicularToLinesFilter::GenerateData()
{
  mitk::Mesh::ConstPointer input  = this->GetInput();
  mitk::GeometryData::Pointer output = this->GetOutput();

  mitk::Mesh::PointType thisPoint;

  //iterate through all cells and build tubes
  Mesh::CellIterator cellIt, cellEnd;
  cellEnd = input->GetMesh()->GetCells()->End();
  for( cellIt = input->GetMesh()->GetCells()->Begin(); cellIt != cellEnd; ++cellIt )
  {
  }

  output->SetGeometry(m_CreatedGeometries);
}

void mitk::PlanesPerpendicularToLinesFilter::SetPlane(const mitk::PlaneGeometry* aPlane)
{
  if(aPlane!=NULL)
  {
    m_Plane = static_cast<mitk::PlaneGeometry*>(aPlane->Clone().GetPointer());
  }
  else
  {
    if(m_Plane.IsNull())
      return;
    m_Plane=NULL;
  }
  Modified();
}


