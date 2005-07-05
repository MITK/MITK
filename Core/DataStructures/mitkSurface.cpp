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
#include "mitkBaseProcess.h"
#include "vtkPolyData.h"
#include "mitkXMLWriter.h"

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
  mitk::TimeSlicedGeometry::Pointer timeGeometry = const_cast< mitk::TimeSlicedGeometry *>(this->GetTimeSlicedGeometry());

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

vtkPolyData* mitk::Surface::GetVtkPolyData( unsigned int t )
{

  if ( t < m_PolyDataSeries.size() )
  {
    vtkPolyData* polydata = m_PolyDataSeries[ t ];
    if((polydata==NULL) && (GetSource().GetPointer()!=NULL))
    {
      RegionType requestedregion;
      requestedregion.SetIndex(3, t);
      requestedregion.SetSize(3, 1);
      SetRequestedRegion(&requestedregion);
      GetSource()->Update();
    }
    polydata = m_PolyDataSeries[ t ];
    return polydata;
  }
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
    CalculateBoundingBox();
}

void mitk::Surface::CalculateBoundingBox()
{
  //
  // first make sure, that the associated time sliced geometry has
  // the same number of geometry 3d's as vtkPolyDatas are present
  //
  mitk::TimeSlicedGeometry::Pointer timeGeometry = GetTimeSlicedGeometry();
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
#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
    double bounds[ ] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
#else
    float bounds[ ] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
#endif
    if ( ( polyData != NULL ) && ( polyData->GetNumberOfPoints() > 0 ) )
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

//##ModelId=3E70F66100B0
void mitk::Surface::SetRequestedRegionToLargestPossibleRegion()
{
  m_RequestedRegion = GetLargestPossibleRegion();
}

//##ModelId=3E70F66100B6

bool mitk::Surface::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  RegionType::IndexValueType end = m_RequestedRegion.GetIndex(3)+m_RequestedRegion.GetSize(3);

  if(((RegionType::IndexValueType)m_PolyDataSeries.size())<end)
    return true;

  for( RegionType::IndexValueType t=m_RequestedRegion.GetIndex(3); t<end; ++t )
    if(m_PolyDataSeries[t]==NULL)
      return true;

  return false;
}

//##ModelId=3E70F66100B8
bool mitk::Surface::VerifyRequestedRegion()
{
  if( (m_RequestedRegion.GetIndex(3)>=0) && 
      (m_RequestedRegion.GetIndex(3)+m_RequestedRegion.GetSize(3)<=m_PolyDataSeries.size()) )
    return true;

  return false;
}

//##ModelId=3E70F66100BA
void mitk::Surface::SetRequestedRegion( itk::DataObject *data )
{
  mitk::Surface *surfaceData;

  surfaceData = dynamic_cast<mitk::Surface*>(data);

  if (surfaceData)
  {
    m_RequestedRegion = surfaceData->GetRequestedRegion();
  }
  else
  {
    // pointer could not be cast back down
    itkExceptionMacro( << "mitk::Surface::SetRequestedRegion(DataObject*) cannot cast " << typeid(data).name() << " to " << typeid(Surface*).name() );
  }
}

void mitk::Surface::SetRequestedRegion(Surface::RegionType *region)  //by arin
{
  if(region!=NULL)
  {
    m_RequestedRegion = *region;
  }
  else
  {
    // pointer could not be cast back down
    itkExceptionMacro( << "mitk::Surface::SetRequestedRegion(Surface::RegionType*) cannot cast " << typeid(region).name() << " to " << typeid(Surface*).name() );
  }
}

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

void mitk::Surface::Resize( unsigned int timeSteps )
{
  m_PolyDataSeries.resize( timeSteps, NULL );
  this->Modified();
  m_CalculateBoundingBox = true;
}

bool mitk::Surface::WriteXML( XMLWriter& xmlWriter )
{
	xmlWriter.BeginNode("data");
	xmlWriter.WriteProperty( "className", typeid( *this ).name() );

  std::string fileName = xmlWriter.getNewFileName();
  fileName += ".stl";
  xmlWriter.WriteProperty( "FILENAME", fileName.c_str() );

	mitk::Geometry3D* geomety = GetGeometry();

	if ( geomety )
		geomety->WriteXML( xmlWriter );

	xmlWriter.EndNode(); // data
	return true;
}


bool mitk::Surface::ReadXML( XMLReader& xmlReader )
{

  return false;
}
