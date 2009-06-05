/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
#include "mitkXMLReader.h"
#include <vtkPolyDataWriter.h>
//#include <vtkPolyDataReader.h>
#include <vtkSTLWriter.h>
#include "mitkSurfaceVtkWriter.h"
#include "mitkDataTreeNodeFactory.h"
#include <itkSmartPointerForwardReference.txx>
#include "mitkInteractionConst.h"
#include "mitkSurfaceOperation.h"


mitk::Surface::Surface() : m_CalculateBoundingBox( false )
{
  m_Initialized = false;
  vtkPolyData* pdnull = NULL;
  m_PolyDataSeries.resize( 1, pdnull );
  Superclass::InitializeTimeSlicedGeometry(1);

  m_Initialized = true;
}

mitk::Surface::~Surface()
{
  for ( VTKPolyDataSeries::iterator it = m_PolyDataSeries.begin(); it != m_PolyDataSeries.end(); ++it )
  {
    if ( ( *it ) != NULL )
      ( *it )->Delete();
  }
  m_PolyDataSeries.clear();
}

void mitk::Surface::SetVtkPolyData( vtkPolyData* polydata, unsigned int t )
{
  // Adapt the size of the data vector if necessary
  this->Expand( t+1 );

  if(m_PolyDataSeries[ t ] != NULL)
  {
    // we do not need the reference on the object any longer
    m_PolyDataSeries[ t ]->Delete();
  }
  m_PolyDataSeries[ t ] = polydata;
  // call m_VtkPolyData->Register(NULL) to tell 
  // the reference counting that we want to keep a 
  // reference on the object
  if(m_PolyDataSeries[ t ] != NULL)
  {
    m_PolyDataSeries[ t ]->Register( NULL );
  }
  this->Modified();
  m_CalculateBoundingBox = true;
}

bool mitk::Surface::IsEmpty(unsigned int t) const
{
  if(!IsInitialized())
    return false;
  vtkPolyData* polydata = const_cast<Surface*>(this)->GetVtkPolyData(t);
  return 
    (polydata == NULL) || 
    (
    (polydata->GetNumberOfVerts()  <= 0) &&
    (polydata->GetNumberOfPolys()  <= 0) &&
    (polydata->GetNumberOfStrips() <= 0) &&
    (polydata->GetNumberOfLines()  <= 0)
    );
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

void mitk::Surface::UpdateOutputInformation()
{
  if ( this->GetSource() )
  {
    this->GetSource()->UpdateOutputInformation();
  }
  if ( ( m_CalculateBoundingBox ) && ( m_PolyDataSeries.size() > 0 ) )
    CalculateBoundingBox();
  else
    GetTimeSlicedGeometry()->UpdateInformation();
}

void mitk::Surface::CalculateBoundingBox()
{
  //
  // first make sure, that the associated time sliced geometry has
  // the same number of geometry 3d's as vtkPolyDatas are present
  //
  mitk::TimeSlicedGeometry* timeGeometry = GetTimeSlicedGeometry();
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
    vtkFloatingPointType bounds[ ] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
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
  timeGeometry->UpdateInformation();

  mitk::BoundingBox::Pointer bb = const_cast<mitk::BoundingBox*>( timeGeometry->GetBoundingBox() );
  itkDebugMacro( << "boundingbox min: "<< bb->GetMinimum());
  itkDebugMacro( << "boundingbox max: "<< bb->GetMaximum());
  m_CalculateBoundingBox = false;
}

void mitk::Surface::SetRequestedRegionToLargestPossibleRegion()
{
  m_RequestedRegion = GetLargestPossibleRegion();
}

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

bool mitk::Surface::VerifyRequestedRegion()
{
  if( (m_RequestedRegion.GetIndex(3)>=0) && 
    (m_RequestedRegion.GetIndex(3)+m_RequestedRegion.GetSize(3)<=m_PolyDataSeries.size()) )
    return true;

  return false;
}

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

void mitk::Surface::CopyInformation( const itk::DataObject * data)
{
  Superclass::CopyInformation( data );

  const mitk::Surface* surfaceData;

  surfaceData = dynamic_cast<const mitk::Surface*>( data );

  if ( surfaceData )
  {
    m_LargestPossibleRegion = surfaceData->GetLargestPossibleRegion();
  }
  else
  {
    // pointer could not be cast back down
    itkExceptionMacro( << "mitk::Surface::CopyInformation(const DataObject *data) cannot cast " << typeid(data).name() << " to " << typeid(surfaceData).name() );
  }
  

}

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

void mitk::Surface::Expand( unsigned int timeSteps )
{  
  // check if the vector is long enouth to contain the new element
  // at the given position. If not, expand it with sufficient zero-filled elements.
  if ( timeSteps > m_PolyDataSeries.size() )
  {
    Superclass::Expand( timeSteps );
    vtkPolyData* pdnull = NULL;
    m_PolyDataSeries.resize( timeSteps, pdnull );
    m_CalculateBoundingBox = true;
  }
}

bool mitk::Surface::WriteXMLData( XMLWriter& xmlWriter )
{
  BaseData::WriteXMLData( xmlWriter );
  std::string fileName = xmlWriter.GetRelativePath();

  if(xmlWriter.IsFileExtension(".vtk", fileName))
  {
    xmlWriter.WriteProperty( XMLReader::FILENAME, fileName.c_str() );

    if(xmlWriter.SaveSourceFiles()){
      mitk::SurfaceVtkWriter<vtkPolyDataWriter>::Pointer writer = mitk::SurfaceVtkWriter<vtkPolyDataWriter>::New();
      writer->SetInput( this );
      fileName = xmlWriter.GetAbsolutePath();
      if(!xmlWriter.IsFileExtension(".vtk", fileName))
        fileName += ".vtk";
      writer->SetFileName( fileName.c_str() );
      writer->Write();
    }
  }
  else
  {
    if(!xmlWriter.IsFileExtension(".stl", fileName))
      fileName += ".stl";
    xmlWriter.WriteProperty( XMLReader::FILENAME, fileName.c_str() );

    if(xmlWriter.SaveSourceFiles()){
      mitk::SurfaceVtkWriter<vtkSTLWriter>::Pointer writer = mitk::SurfaceVtkWriter<vtkSTLWriter>::New();
      writer->SetInput( this );
      fileName = xmlWriter.GetAbsolutePath();
      if(!xmlWriter.IsFileExtension(".stl", fileName))
        fileName += ".stl";
      writer->SetFileName( fileName.c_str() );
      writer->Write();
    }
  }
  return true;
}

bool mitk::Surface::ReadXMLData( XMLReader& xmlReader )
{
  BaseData::ReadXMLData( xmlReader );
  std::string fileName;
  xmlReader.GetAttribute( XMLReader::FILENAME, fileName );

  std::cout << fileName << std::endl;

  mitk::DataTreeNodeFactory::Pointer factory = mitk::DataTreeNodeFactory::New();

  try
  {
    factory->SetFileName( fileName.c_str() );
    factory->Update();
    mitk::DataTreeNode::Pointer tmpNode = factory->GetOutput( 0 );

    if ( tmpNode.IsNull() )
      return false;

    mitk::Surface::Pointer tmpSurface = dynamic_cast<mitk::Surface*>(tmpNode->GetData());

    if ( tmpSurface )
      SetVtkPolyData( tmpSurface->GetVtkPolyData() );

  }
  catch ( itk::ExceptionObject & ex )
  {
    itkGenericOutputMacro( << "Exception during file open: " << ex );
    return false;
  }

  return true;
}

void mitk::Surface::ExecuteOperation(Operation *operation)
{
  switch ( operation->GetOperationType() )
  {
  case OpSURFACECHANGED:
  
    mitk::SurfaceOperation* surfOp = dynamic_cast<mitk::SurfaceOperation*>(operation);
    if( ! surfOp ) break;

    unsigned int time = surfOp->GetTimeStep();

    if(m_PolyDataSeries[ time ] != NULL)
    {
      vtkPolyData* updatePoly = surfOp->GetVtkPolyData();
      if( updatePoly ){
        this->SetVtkPolyData( updatePoly, time );
        this->CalculateBoundingBox();
      }
    }
    break;
  }
  this->Modified();
}

unsigned int mitk::Surface::GetSizeOfPolyDataSeries() const
{
  return m_PolyDataSeries.size();
}

void mitk::Surface::Graft( const DataObject* data )
{
  const Self* surface;
  try 
  {
    surface = dynamic_cast<const Self*>( data );
  }
  catch(...)
  {
    itkExceptionMacro( << "mitk::Surface::Graft cannot cast "
      << typeid(data).name() << " to "
      << typeid(const Self *).name() );
    return;    
  }

  if(!surface)
  {
    // pointer could not be cast back down
    itkExceptionMacro( << "mitk::Surface::Graft cannot cast "
      << typeid(data).name() << " to "
      << typeid(const Self *).name() );
    return;
  }

  this->CopyInformation( data );
  //clear list of PolyData's
  m_PolyDataSeries.clear();
  // do copy
  for (int i=0; i<surface->GetSizeOfPolyDataSeries(); i++)
    {
      m_PolyDataSeries.push_back(vtkPolyData::New());
      m_PolyDataSeries.back()->DeepCopy( const_cast<mitk::Surface*>(surface)->GetVtkPolyData( i ) );
      //CopyStructure( const_cast<mitk::Surface*>(surface)->GetVtkPolyData( i ) );
    }
}
