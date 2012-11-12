/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center, 
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without 
even the implied warranty of MERCHANTABILITY or FITNESS FOR 
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkPlanarFigureSegmentationController.h"

#include "mitkSurfaceToImageFilter.h"

#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkPolygon.h>
#include <vtkCellArray.h>

#include <vtkMarchingCubes.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

#include "itkTimeProbe.h"
#include "itkResampleImageFilter.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "mitkImageAccessByItk.h"

mitk::PlanarFigureSegmentationController::PlanarFigureSegmentationController()
: itk::Object()
, m_ReduceFilter( NULL )
, m_NormalsFilter( NULL )
, m_DistanceImageCreator( NULL )
, m_ReferenceImage( NULL )
, m_SegmentationAsImage( NULL )
{
  InitializeFilters();
}

mitk::PlanarFigureSegmentationController::~PlanarFigureSegmentationController()
{
}

void mitk::PlanarFigureSegmentationController::SetReferenceImage( mitk::Image::Pointer referenceImage )
{
  m_ReferenceImage = referenceImage;
}

void mitk::PlanarFigureSegmentationController::AddPlanarFigure( mitk::PlanarFigure::Pointer planarFigure )
{
  if ( planarFigure.IsNull() )
    return;

  bool newFigure = true;
  int indexOfFigure = -1;
  for( int i=0; i<m_PlanarFigureList.size(); i++ )
  {
    if( m_PlanarFigureList.at(i) == planarFigure )
    {
      indexOfFigure = i;
      newFigure = false;
      break;
    }
  }

  mitk::Surface::Pointer figureAsSurface = NULL;

  if ( newFigure )
  {
    m_PlanarFigureList.push_back( planarFigure );
    figureAsSurface = this->CreateSurfaceFromPlanarFigure( planarFigure );
    m_SurfaceList.push_back( figureAsSurface );
    indexOfFigure = m_PlanarFigureList.size() -1 ;
  }
  else
  {
    figureAsSurface = this->CreateSurfaceFromPlanarFigure( planarFigure );
    m_SurfaceList.at(indexOfFigure) = figureAsSurface;
  }

  m_ReduceFilter->SetInput( indexOfFigure, figureAsSurface );

  m_NormalsFilter->SetInput( indexOfFigure, m_ReduceFilter->GetOutput( indexOfFigure ) );
  m_DistanceImageCreator->SetInput( indexOfFigure, m_NormalsFilter->GetOutput( indexOfFigure ) );
}

void mitk::PlanarFigureSegmentationController::RemovePlanarFigure( mitk::PlanarFigure::Pointer planarFigure )
{
  if ( planarFigure.IsNull() )
    return;

  bool figureFound = false;
  int indexOfFigure = -1;
  for( int i=0; i<m_PlanarFigureList.size(); i++ )
  {
    if( m_PlanarFigureList.at(i) == planarFigure )
    {
      indexOfFigure = i;
      figureFound = true;
      break;
    }
  }

  if ( !figureFound )
    return;

  if ( indexOfFigure == m_PlanarFigureList.size()-1 )
  {
    // Ff the removed figure was the last one in the list, we can simply
    // remove the last input from each filter.
    m_DistanceImageCreator->RemoveInputs( m_NormalsFilter->GetOutput( indexOfFigure ) );
    m_NormalsFilter->RemoveInputs( m_ReduceFilter->GetOutput( indexOfFigure ) );
    m_ReduceFilter->RemoveInputs( const_cast<mitk::Surface*>(m_ReduceFilter->GetInput(indexOfFigure)) );

//      PlanarFigureListType::iterator whereIter = m_PlanarFigureList.begin();
//      whereIter += indexOfFigure;
//      m_PlanarFigureList.erase( whereIter );
//  
//      SurfaceListType::iterator surfaceIter = m_SurfaceList.begin();
//      surfaceIter += indexOfFigure;
//      m_SurfaceList.erase( surfaceIter );
  }
  else
  {
    // this is not very nice! If the figure that has been removed is NOT the last
    // one in the list we have to create new filters and add all remaining 
    // inputs again.
    // 
    // Has to be done as the filters do not work when removing an input 
    // other than the last one.
    
    // create new filters
    InitializeFilters();

    // and add all existing surfaces
    SurfaceListType::iterator surfaceIter = m_SurfaceList.begin();
    for ( surfaceIter = m_SurfaceList.begin(); surfaceIter!=m_SurfaceList.end(); surfaceIter++ )
    {
      m_ReduceFilter->SetInput( indexOfFigure, (*surfaceIter) );
      m_NormalsFilter->SetInput( indexOfFigure, m_ReduceFilter->GetOutput( indexOfFigure ) );
      m_DistanceImageCreator->SetInput( indexOfFigure, m_NormalsFilter->GetOutput( indexOfFigure ) );
    }
  }

  PlanarFigureListType::iterator whereIter = m_PlanarFigureList.begin();
  whereIter += indexOfFigure;
  m_PlanarFigureList.erase( whereIter );

  SurfaceListType::iterator surfaceIter = m_SurfaceList.begin();
  surfaceIter += indexOfFigure;
  m_SurfaceList.erase( surfaceIter );
}


mitk::Image::Pointer mitk::PlanarFigureSegmentationController::GetInterpolationResult()
{
  m_SegmentationAsImage = NULL;

  if ( m_PlanarFigureList.size() == 0 )
  {
    m_SegmentationAsImage = mitk::Image::New();
    //m_SegmentationAsImage->Initialize(m_ReferenceImage);
    m_SegmentationAsImage->Initialize(mitk::MakeScalarPixelType<unsigned char>() , *m_ReferenceImage->GetTimeSlicedGeometry());

    return m_SegmentationAsImage;
  }

  m_ReduceFilter->Update();
  m_NormalsFilter->Update();
  m_DistanceImageCreator->Update();

  mitk::Image::Pointer distanceImage = m_DistanceImageCreator->GetOutput();

  vtkSmartPointer<vtkMarchingCubes> marchingCubes = vtkMarchingCubes::New();
  marchingCubes->SetInput( distanceImage->GetVtkImageData() );
  marchingCubes->SetValue(0,0);
  marchingCubes->Update();

  mitk::Surface::Pointer segmentationAsSurface = mitk::Surface::New();
  segmentationAsSurface->SetVtkPolyData(marchingCubes->GetOutput());
  segmentationAsSurface->GetGeometry()->SetOrigin( distanceImage->GetGeometry()->GetOrigin() );

  mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();
  surfaceToImageFilter->SetInput( segmentationAsSurface );
  surfaceToImageFilter->SetImage( m_ReferenceImage );
  surfaceToImageFilter->SetMakeOutputBinary(true);
  surfaceToImageFilter->Update();

  m_SegmentationAsImage = surfaceToImageFilter->GetOutput();

  return m_SegmentationAsImage;
}


mitk::Surface::Pointer mitk::PlanarFigureSegmentationController::CreateSurfaceFromPlanarFigure( mitk::PlanarFigure::Pointer figure )
{
  if ( figure.IsNull() )
  {
    MITK_ERROR << "Given PlanarFigure is NULL. Please provide valid PlanarFigure.";
    return NULL;
  }

  mitk::Surface::Pointer newSurface = mitk::Surface::New();

  vtkSmartPointer<vtkPoints> points = vtkPoints::New();
  vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New();
  vtkSmartPointer<vtkCellArray> cells = vtkCellArray::New();
  vtkSmartPointer<vtkPolyData> polyData = vtkPolyData::New();

  const mitk::Geometry2D* figureGeometry = figure->GetGeometry2D();

  // Get the polyline
  mitk::PlanarFigure::PolyLineType planarPolyLine = figure->GetPolyLine(0);
  mitk::PlanarFigure::PolyLineType::iterator iter;

  // iterate over the polyline, ...
  int pointCounter = 0;
  for( iter = planarPolyLine.begin(); iter != planarPolyLine.end(); iter++ )
  {
    // ... determine the world-coordinates
    mitk::Point2D polyLinePoint = iter->Point;
    mitk::Point3D pointInWorldCoordiantes;
    figureGeometry->Map( polyLinePoint, pointInWorldCoordiantes );

    // and add them as new points to the vtkPoints
    points->InsertNextPoint( pointInWorldCoordiantes[0], pointInWorldCoordiantes[1], pointInWorldCoordiantes[2] );
    ++pointCounter;
  }

//   for( int i=0; i<figure->GetNumberOfControlPoints(); i++ )
//   {
//     // ... determine the world-coordinates
//     mitk::Point2D polyLinePoint = figure->GetControlPoint(i);
//     mitk::Point3D pointInWorldCoordiantes;
//     figureGeometry->Map( polyLinePoint, pointInWorldCoordiantes );
// 
//     // and add them as new points to the vtkPoints
//     points->InsertNextPoint( pointInWorldCoordiantes[0], pointInWorldCoordiantes[1], pointInWorldCoordiantes[2] );
//     ++pointCounter;
//   }


  // create a polygon with the points of the polyline
  polygon->GetPointIds()->SetNumberOfIds( pointCounter );
  for(unsigned int i = 0; i < pointCounter; i++)
  {
    polygon->GetPointIds()->SetId(i,i);
  }

  // initialize the vtkCellArray and vtkPolyData
  cells->InsertNextCell(polygon);
  polyData->SetPoints(points);
  polyData->SetPolys( cells );

  // set the polydata to the surface
  newSurface->SetVtkPolyData( polyData );

  return newSurface;
}

mitk::PlanarFigureSegmentationController::PlanarFigureListType mitk::PlanarFigureSegmentationController::GetAllPlanarFigures()
{
  return m_PlanarFigureList;
}

void mitk::PlanarFigureSegmentationController::InitializeFilters()
{
  m_ReduceFilter = mitk::ReduceContourSetFilter::New();
  m_ReduceFilter->SetReductionType(ReduceContourSetFilter::NTH_POINT);
  m_ReduceFilter->SetStepSize( 10 );
  m_NormalsFilter = mitk::ComputeContourSetNormalsFilter::New();
  m_DistanceImageCreator = mitk::CreateDistanceImageFromSurfaceFilter::New();
}

