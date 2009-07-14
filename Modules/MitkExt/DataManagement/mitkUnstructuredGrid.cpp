/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkUnstructuredGrid.h"

#include <vtkUnstructuredGrid.h>

#include <itkSmartPointerForwardReference.txx>

void mitk::UnstructuredGrid::SetVtkUnstructuredGrid( vtkUnstructuredGrid* grid, unsigned int t )
{
  this->Expand(t);

  if(m_GridSeries[ t ] != NULL)
  {
    m_GridSeries[ t ]->Delete();
  }
  
  m_GridSeries[ t ] = grid;

  // call m_VtkPolyData->Register(NULL) to tell the reference counting that we 
  // want to keep a reference on the object
  if (m_GridSeries[t] != 0)
    m_GridSeries[t]->Register(grid);
  
  this->Modified();
  m_CalculateBoundingBox = true;
}

void mitk::UnstructuredGrid::Expand(unsigned int timeSteps)
{
  // check if the vector is long enough to contain the new element
  // at the given position. If not, expand it with sufficient zero-filled elements.
  if(timeSteps > m_GridSeries.size())
  {
    Superclass::Expand(timeSteps);
    vtkUnstructuredGrid* pdnull = 0;
    m_GridSeries.resize( timeSteps, pdnull );
    m_CalculateBoundingBox = true;
  }
}

void mitk::UnstructuredGrid::ClearData()
{
  for ( VTKUnstructuredGridSeries::iterator it = m_GridSeries.begin(); it != m_GridSeries.end(); ++it )
  {
    if ( ( *it ) != 0 )
      ( *it )->Delete();
  }
  m_GridSeries.clear();

  Superclass::ClearData();
}

void mitk::UnstructuredGrid::InitializeEmpty()
{
  vtkUnstructuredGrid* pdnull = 0;
  m_GridSeries.resize( 1, pdnull );
  Superclass::InitializeTimeSlicedGeometry(1);

  m_Initialized = true;
}

vtkUnstructuredGrid* mitk::UnstructuredGrid::GetVtkUnstructuredGrid(unsigned int t)
{
  if ( t < m_GridSeries.size() )
  {
    vtkUnstructuredGrid* grid = m_GridSeries[ t ];
    if((grid == 0) && (GetSource().GetPointer() != 0))
    {
      RegionType requestedregion;
      requestedregion.SetIndex(3, t);
      requestedregion.SetSize(3, 1);
      SetRequestedRegion(&requestedregion);
      GetSource()->Update();
    }
    grid = m_GridSeries[ t ];
    return grid;
  }
  else
    return 0;
}

mitk::UnstructuredGrid::UnstructuredGrid() : m_CalculateBoundingBox( false )
{
  this->InitializeEmpty();
}

mitk::UnstructuredGrid::~UnstructuredGrid()
{
  this->ClearData();
}

void mitk::UnstructuredGrid::UpdateOutputInformation()
{
 if ( this->GetSource() )
  {
    this->GetSource()->UpdateOutputInformation();
  }
  if ( ( m_CalculateBoundingBox ) && ( m_GridSeries.size() > 0 ) )
    CalculateBoundingBox();
  else
    GetTimeSlicedGeometry()->UpdateInformation();
}

void mitk::UnstructuredGrid::CalculateBoundingBox()
{
  //
  // first make sure, that the associated time sliced geometry has
  // the same number of geometry 3d's as vtkUnstructuredGrids are present
  //
  mitk::TimeSlicedGeometry* timeGeometry = GetTimeSlicedGeometry();
  if ( timeGeometry->GetTimeSteps() != m_GridSeries.size() )
  {
    itkExceptionMacro(<<"timeGeometry->GetTimeSteps() != m_GridSeries.size() -- use Initialize(timeSteps) with correct number of timeSteps!");
  }

  //
  // Iterate over the vtkUnstructuredGrids and update the Geometry
  // information of each of the items.
  //
  for ( unsigned int i = 0 ; i < m_GridSeries.size() ; ++i )
  {
    vtkUnstructuredGrid* grid = m_GridSeries[ i ];
    vtkFloatingPointType bounds[ ] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    if ( ( grid != 0 ) && ( grid->GetNumberOfCells() > 0 ) )
    {
      grid->Update();
      grid->ComputeBounds();
      grid->GetBounds( bounds );
    }
    mitk::Geometry3D::Pointer g3d = timeGeometry->GetGeometry3D( i );
    assert( g3d.IsNotNull() );
    g3d->SetFloatBounds( bounds );
  }
  timeGeometry->UpdateInformation();

  mitk::BoundingBox::Pointer bb = const_cast<mitk::BoundingBox*>( timeGeometry->GetBoundingBox() );
  itkDebugMacro( << "boundingbox min: "<< bb->GetMinimum());
  itkDebugMacro( << "boundingbox max: "<< bb->GetMaximum());
  m_CalculateBoundingBox = false;
}


void mitk::UnstructuredGrid::SetRequestedRegionToLargestPossibleRegion()
{
  m_RequestedRegion = GetLargestPossibleRegion();
}

bool mitk::UnstructuredGrid::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  RegionType::IndexValueType end = m_RequestedRegion.GetIndex(3)+m_RequestedRegion.GetSize(3);

  if(((RegionType::IndexValueType)m_GridSeries.size()) < end)
    return true;

  for( RegionType::IndexValueType t=m_RequestedRegion.GetIndex(3); t < end; ++t )
    if(m_GridSeries[t] == 0)
      return true;

  return false;
}

bool mitk::UnstructuredGrid::VerifyRequestedRegion()
{
  if( (m_RequestedRegion.GetIndex(3)>=0) && 
      (m_RequestedRegion.GetIndex(3)+m_RequestedRegion.GetSize(3)<=m_GridSeries.size()) )
    return true;

  return false;
}

void mitk::UnstructuredGrid::SetRequestedRegion( itk::DataObject *data )
{
  mitk::UnstructuredGrid *gridData;

  gridData = dynamic_cast<mitk::UnstructuredGrid*>(data);

  if (gridData)
  {
    m_RequestedRegion = gridData->GetRequestedRegion();
  }
  else
  {
    // pointer could not be cast back down
    itkExceptionMacro( << "mitk::UnstructuredGrid::SetRequestedRegion(DataObject*) cannot cast " << typeid(data).name() << " to " << typeid(UnstructuredGrid*).name() );
  }
}

void mitk::UnstructuredGrid::SetRequestedRegion(UnstructuredGrid::RegionType *region)  //by arin
{
  if(region != 0)
  {
    m_RequestedRegion = *region;
  }
  else
  {
    // pointer could not be cast back down
    itkExceptionMacro( << "mitk::UnstructuredGrid::SetRequestedRegion(UnstructuredGrid::RegionType*) cannot cast " << typeid(region).name() << " to " << typeid(UnstructuredGrid*).name() );
  }
}

void mitk::UnstructuredGrid::CopyInformation( const itk::DataObject * data )
{
  Superclass::CopyInformation(data);
}

void mitk::UnstructuredGrid::Update()
{
  if ( GetSource() == 0 )
  {
    for ( VTKUnstructuredGridSeries::iterator it = m_GridSeries.begin() ; it != m_GridSeries.end() ; ++it )
    {
      if ( ( *it ) != 0 )
        ( *it )->Update();
    }
  }
  Superclass::Update();
}
