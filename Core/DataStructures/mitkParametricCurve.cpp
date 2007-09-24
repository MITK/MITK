/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 11989 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkParametricCurve.h"

#include "mitkOperation.h"
#include "mitkXMLWriter.h"
#include "mitkXMLReader.h"
#include "mitkRenderingManager.h"

#include <itkSmartPointerForwardReference.txx>

namespace mitk
{

ParametricCurve
::ParametricCurve()
{
  m_Initialized = false;

  this->InitializeTimeSlicedGeometry( 1 );
}


ParametricCurve
::~ParametricCurve()
{
}


void 
ParametricCurve
::InitializeTimeSlicedGeometry( unsigned int timeSteps )
{
  mitk::TimeSlicedGeometry::Pointer timeGeometry = this->GetTimeSlicedGeometry();

  mitk::Geometry3D::Pointer g3d = mitk::Geometry3D::New();
  g3d->Initialize();

  if ( timeSteps > 1 )
  {
    mitk::ScalarType timeBounds[] = {0.0, 1.0};
    g3d->SetTimeBounds( timeBounds );
  }

  //
  // The geometry is propagated automatically to the other items,
  // if EvenlyTimed is true...
  //
  timeGeometry->InitializeEvenlyTimed( g3d.GetPointer(), timeSteps );

  m_Initialized = (timeSteps>0);
}


void 
ParametricCurve
::ExecuteOperation( Operation * /*operation*/ )
{
  // Empty by default. override if needed!
}


void 
ParametricCurve
::UpdateOutputInformation()
{
  // Use methode of BaseData; override in sub-classes
  Superclass::UpdateOutputInformation();
}


void 
ParametricCurve
::SetRequestedRegionToLargestPossibleRegion()
{
  // does not apply
}


bool 
ParametricCurve
::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  // does not apply
  return false;
}


bool 
ParametricCurve
::VerifyRequestedRegion()
{
  // does not apply
  return true;
}


void 
ParametricCurve
::SetRequestedRegion( itk::DataObject * )
{
  // does not apply
}


bool 
ParametricCurve
::WriteXMLData( XMLWriter &xmlWriter )
{
  // to be implemented by sub-classes
  return false;
}


bool 
ParametricCurve
::ReadXMLData( XMLReader &xmlReader )
{
  // to be implemented by sub-classes
  return false;
}

} // namespace mitk
