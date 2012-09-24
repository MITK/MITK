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

mitk::PlanarFigureSegmentationController::PlanarFigureSegmentationController()
: itk::Object()
, m_ReduceFilter( NULL )
, m_NormalsFilter( NULL )
, m_DistanceImageCreator( NULL )
, m_ReferenceImage( NULL )
{

  m_ReduceFilter = mitk::ReduceContourSetFilter::New();
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

  bool newFigure = false;
  if ( std::count(m_PlanarFigureList.begin(), m_PlanarFigureList.end(), planarFigure ) == 0 )
  {
    newFigure = true;
  }

  if ( newFigure )
  {
    m_PlanarFigureList.push_back( planarFigure );
    mitk::Surface::Pointer figureAsSurface = this->CreateSurfaceFromPlanarFigure( planarFigure );

    int indexOfFigure = m_PlanarFigureList.size() -1 ;
    m_ReduceFilter->SetInput( indexOfFigure, figureAsSurface );
    m_NormalsFilter->SetInput( indexOfFigure, m_ReduceFilter->GetOutput( indexOfFigure ) );
    m_DistanceImageCreator->SetInput( indexOfFigure, m_NormalsFilter->GetOutput( indexOfFigure ) );
  }

}


mitk::Image::Pointer mitk::PlanarFigureSegmentationController::GetInterpolationResult()
{
  m_DistanceImageCreator->Update(); 

  mitk::Image::Pointer distanceImage = m_DistanceImageCreator->GetOutput();

  vtkSmartPointer<vtkMarchingCubes> mcFilter = vtkMarchingCubes::New();
  mcFilter->SetInput( distanceImage->GetVtkImageData() );
  mcFilter->SetValue(0,0);
  mcFilter->Update();


  mitk::Surface::Pointer result = mitk::Surface::New();
  result->SetVtkPolyData(mcFilter->GetOutput());
  result->GetGeometry()->SetOrigin( distanceImage->GetGeometry()->GetOrigin() );

  mitk::SurfaceToImageFilter::Pointer imageCreator = mitk::SurfaceToImageFilter::New();
  imageCreator->SetInput( result );
  imageCreator->SetImage( m_ReferenceImage );
  imageCreator->SetMakeOutputBinary(true);
  imageCreator->Update();

  mitk::Image::Pointer resultImage = imageCreator->GetOutput();
  return resultImage;
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

