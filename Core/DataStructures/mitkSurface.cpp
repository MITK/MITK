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


#include "mitkSurface.h"
#include <itkProcessObject.h>
#include "vtkPolyData.h"

//##ModelId=3E70F661009A
void mitk::Surface::SetVtkPolyData( vtkPolyData* polydata, unsigned int t )
{
    if ( polydata == NULL )
        return ;

    if ( t >= m_PolyDataSeries.size() )
    {
        m_PolyDataSeries.resize( t + 1, NULL );
        Initialize(t+1);
    }
    m_PolyDataSeries[ t ] = polydata;
    //@todo why do we have to call m_VtkPolyData->Register(NULL?)
    m_PolyDataSeries[ t ]->Register( NULL );
    this->Modified();
    m_CalculateBoundingBox = true;
}

void mitk::Surface::Initialize(unsigned int timeSteps)
{
  mitk::TimeSlicedGeometry::Pointer timeGeometry = this->GetTimeSlicedGeometry();

  mitk::Geometry3D::Pointer g3d = mitk::Geometry3D::New();
  g3d->Initialize();
  mitk::ScalarType timeBounds[] = {0.0, 1.0};
  g3d->SetTimeBoundsInMS( timeBounds );
  //
  // The geometry is propagated automatically to the other items,
  // if EvenlyTimed is true...
  //
  timeGeometry->InitializeEvenlyTimed( g3d.GetPointer(), timeSteps );
  
  vtkPolyData* pdnull = NULL;
  m_PolyDataSeries.reserve(timeSteps);
  m_PolyDataSeries.assign(timeSteps, pdnull);
}

void mitk::Surface::SetGeometry(mitk::Geometry3D* aGeometry3D)
{
  if(aGeometry3D!=NULL)
  {
    TimeSlicedGeometry::Pointer timeSlicedGeometry = dynamic_cast<TimeSlicedGeometry*>(aGeometry3D);
    if(timeSlicedGeometry.IsNull())
    {
      timeSlicedGeometry = TimeSlicedGeometry::New();
      m_Geometry3D = timeSlicedGeometry;   
      timeSlicedGeometry->InitializeEvenlyTimed(aGeometry3D, 1);
    }
    Superclass::SetGeometry(timeSlicedGeometry);
  }
  else
    Superclass::SetGeometry(NULL);
}

//##ModelId=3E70F66100A5
vtkPolyData* mitk::Surface::GetVtkPolyData( unsigned int t )
{
    if ( t < m_PolyDataSeries.size() )
        return m_PolyDataSeries[ t ];
    else
        return NULL;
}

//##ModelId=3E70F66100C4
mitk::Surface::Surface() : m_CalculateBoundingBox( false )
{
    m_Geometry3D = mitk::TimeSlicedGeometry::New();
}

//##ModelId=3E70F66100CA
mitk::Surface::~Surface()
{
    for ( VTKPolyDataSeries::iterator it = m_PolyDataSeries.begin(); it != m_PolyDataSeries.end(); ++it )
    {
        if ( ( *it ) != NULL )
            ( *it )->Delete();
    }
}

//##ModelId=3E70F66100AE
void mitk::Surface::UpdateOutputInformation()
{
    if ( this->GetSource() )
    {
        this->GetSource()->UpdateOutputInformation();
    }
    if ( ( m_CalculateBoundingBox ) && ( m_PolyDataSeries.size() > 0 ) )
    {

        //
        // first make sure, that the associated time sliced geometry has
        // the same number of geometry 3d's as vtkPolyDatas are present
        //
        mitk::TimeSlicedGeometry::Pointer timeGeometry = this->GetTimeSlicedGeometry();
        if ( timeGeometry->GetTimeSteps() != m_PolyDataSeries.size() )
        {
          itkExceptionMacro(<<"timeGeometry->GetTimeSteps() != m_PolyDataSeries.size() -- use Initialize(timeSteps) with correct number of timeSteps!");
        }

        //
        // Iterate over the vtkPolyDatas and update the Geometry
        // information of each of the items.
        //
        for ( unsigned int i = 0 ; i < m_PolyDataSeries.size() ; ++i )
        {
            vtkPolyData* polyData = m_PolyDataSeries[ i ];
            float bounds[ ] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            if ( polyData != NULL )
            {
                polyData->Update();
                polyData->ComputeBounds();
                polyData->GetBounds( bounds );
            }
            mitk::Geometry3D::Pointer g3d = timeGeometry->GetGeometry3D( i );
            assert( g3d.IsNotNull() );
            g3d->SetFloatBounds( bounds );
        }
        mitk::BoundingBox::Pointer bb = const_cast<mitk::BoundingBox*>( timeGeometry->GetBoundingBox() );
        //std::cout << "min"<< bb->GetMinimum() << std::endl;
        //std::cout << "max"<< bb->GetMaximum() << std::endl;
        m_CalculateBoundingBox = false;
    }
}

//##ModelId=3E70F66100B0
void mitk::Surface::SetRequestedRegionToLargestPossibleRegion()
{}

//##ModelId=3E70F66100B6

bool mitk::Surface::RequestedRegionIsOutsideOfTheBufferedRegion()
{
    if ( VerifyRequestedRegion() == false )
        return true;

    return false;
}

//##ModelId=3E70F66100B8
bool mitk::Surface::VerifyRequestedRegion()
{
    if ( m_PolyDataSeries.size() == 0 )
        return false;

    return true;
}

//##ModelId=3E70F66100BA
void mitk::Surface::SetRequestedRegion( itk::DataObject *data )
{}

//##ModelId=3E70F66100C1

void mitk::Surface::CopyInformation( const itk::DataObject *data )
{}

void mitk::Surface::Update()
{
    if ( GetSource() == NULL )
    {
        for ( VTKPolyDataSeries::iterator it = m_PolyDataSeries.begin() ; it != m_PolyDataSeries.end() ; ++it )
        {
            if ( ( *it ) != NULL )
                ( *it )->Update();
        }
    }
    Superclass::Update();
}


mitk::TimeSlicedGeometry* mitk::Surface::GetTimeSlicedGeometry()
{
    return dynamic_cast<mitk::TimeSlicedGeometry*>( m_Geometry3D.GetPointer() );
}

void mitk::Surface::Resize( unsigned int timeSteps )
{
    m_PolyDataSeries.resize( timeSteps, NULL );
    this->Modified();
    m_CalculateBoundingBox = true;
}

