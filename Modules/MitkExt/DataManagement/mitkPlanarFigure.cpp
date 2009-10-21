/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-08 11:04:08 +0200 (Mi, 08 Jul 2009) $
Version:   $Revision: 18029 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarFigure.h"
#include "mitkGeometry2D.h"


mitk::PlanarFigure::PlanarFigure()
{
  m_ControlPoints = VertexContainerType::New();
  m_PolyLine = VertexContainerType::New();

  // Currently only single-time-step geometries are supported
  this->InitializeTimeSlicedGeometry( 1 );
}


mitk::PlanarFigure::~PlanarFigure()
{
}


void mitk::PlanarFigure::SetGeometry2D( mitk::Geometry2D *geometry )
{
  this->SetGeometry( geometry );
}


unsigned int mitk::PlanarFigure::GetNumberOfControlPoints() const
{
  return m_ControlPoints->Size();
}


/** \brief Returns 2D control points vector. */
const mitk::PlanarFigure::VertexContainerType *
mitk::PlanarFigure::GetControlPoints() const
{
  return m_ControlPoints;
}


/** \brief Returns 2D control points vector. */
mitk::PlanarFigure::VertexContainerType *
mitk::PlanarFigure::GetControlPoints()
{
  return m_ControlPoints;
}


const mitk::PlanarFigure::VertexContainerType *
mitk::PlanarFigure::GetPolyLine()
{
  if ( m_PolyLine->GetMTime() < m_ControlPoints->GetMTime() )
  {
    this->GeneratePolyLine();
  }

  return m_PolyLine;
}


void mitk::PlanarFigure::UpdateOutputInformation()
{
  // Bounds are NOT calculated here, since the Geometry2D defines a fixed
  // frame (= bounds) for the planar figure.
  this->GetTimeSlicedGeometry()->UpdateInformation();
}


void mitk::PlanarFigure::SetRequestedRegionToLargestPossibleRegion()
{
}


bool mitk::PlanarFigure::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}


bool mitk::PlanarFigure::VerifyRequestedRegion()
{
  return true;
}


void mitk::PlanarFigure::SetRequestedRegion( itk::DataObject * /*data*/ )
{

}


void mitk::PlanarFigure::InitializeTimeSlicedGeometry( unsigned int timeSteps )
{
  mitk::TimeSlicedGeometry::Pointer timeGeometry = this->GetTimeSlicedGeometry();

  mitk::Geometry2D::Pointer geometry2D = mitk::Geometry2D::New();
  geometry2D->Initialize();

  if ( timeSteps > 1 )
  {
    mitk::ScalarType timeBounds[] = {0.0, 1.0};
    geometry2D->SetTimeBounds( timeBounds );
  }

  // The geometry is propagated automatically to all time steps,
  // if EvenlyTimed is true...
  timeGeometry->InitializeEvenlyTimed( geometry2D, timeSteps );
}


void mitk::PlanarFigure::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "Number of control points: " << this->GetNumberOfControlPoints() << std::endl;
 
  os << indent << "Control points:" << std::endl;

  mitk::PlanarFigure::VertexContainerType::ConstIterator it;
  
  unsigned int i;
  for ( it = m_ControlPoints->Begin(), i = 0;
        it != m_ControlPoints->End();
        ++it, ++i )
  {
    os << indent << indent << i << ": " << it.Value() << std::endl;
  }
}
