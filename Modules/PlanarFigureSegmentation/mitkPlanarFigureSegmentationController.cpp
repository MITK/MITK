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

  m_ReduceFilter = mitk::ReduceContourSetFilter::New();
  m_ReduceFilter->SetReductionType(ReduceContourSetFilter::NTH_POINT);
  m_ReduceFilter->SetStepSize( 10 );
  m_NormalsFilter = mitk::ComputeContourSetNormalsFilter::New();
  m_DistanceImageCreator = mitk::CreateDistanceImageFromSurfaceFilter::New();

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

  if ( newFigure )
  {
    m_PlanarFigureList.push_back( planarFigure );
    indexOfFigure = m_PlanarFigureList.size() -1 ;
  }

  mitk::Surface::Pointer figureAsSurface = this->CreateSurfaceFromPlanarFigure( planarFigure );
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
    m_DistanceImageCreator->RemoveInputs( m_NormalsFilter->GetOutput( indexOfFigure ) );
    m_NormalsFilter->RemoveInputs( m_ReduceFilter->GetOutput( indexOfFigure ) );
    m_ReduceFilter->RemoveInputs( const_cast<mitk::Surface*>(m_ReduceFilter->GetInput(indexOfFigure)) );
  }

}


mitk::Image::Pointer mitk::PlanarFigureSegmentationController::GetInterpolationResult()
{
  m_SegmentationAsImage = NULL;

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

