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

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include "mitkImageWriter.h"
#include "mitkSurfaceVtkWriter.h"
#include "mitkImageToSurfaceFilter.h"

#include "mitkImageAccessByItk.h"

#include "mitkImageCast.h"


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
  std::size_t indexOfFigure = 0;
  for( std::size_t i=0; i<m_PlanarFigureList.size(); i++ )
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
    if (!m_PlanarFigureList.empty())
    {
      indexOfFigure = m_PlanarFigureList.size() -1 ;
    }
  }
  else
  {
    figureAsSurface = this->CreateSurfaceFromPlanarFigure( planarFigure );
    m_SurfaceList.at(indexOfFigure) = figureAsSurface;
  }

  if ( m_ReduceFilter.IsNull() )
  {
    InitializeFilters();
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
  std::size_t indexOfFigure = 0;
  for( std::size_t i=0; i<m_PlanarFigureList.size(); i++ )
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

  PlanarFigureListType::iterator whereIter = m_PlanarFigureList.begin();
  whereIter += indexOfFigure;
  m_PlanarFigureList.erase( whereIter );

  SurfaceListType::iterator surfaceIter = m_SurfaceList.begin();
  surfaceIter += indexOfFigure;
  m_SurfaceList.erase( surfaceIter );

  // TODO: fix this! The following code had to be removed as the method
  // RemoveInputs() has been removed in ITK 4
  // The remaining code works correctly but is slower
  if ( false && indexOfFigure == m_PlanarFigureList.size()-1 )
  {
    // Ff the removed figure was the last one in the list, we can simply
    // remove the last input from each filter.
//     m_DistanceImageCreator->RemoveInputs( m_NormalsFilter->GetOutput( indexOfFigure ) );
//     m_NormalsFilter->RemoveInput( m_ReduceFilter->GetOutput( indexOfFigure ) );
//     m_ReduceFilter->RemoveInput( const_cast<mitk::Surface*>(m_ReduceFilter->GetInput(indexOfFigure)) );
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
    int index = 0;
    for ( surfaceIter = m_SurfaceList.begin(); surfaceIter!=m_SurfaceList.end(); surfaceIter++ )
    {
      m_ReduceFilter->SetInput( index, (*surfaceIter) );
      m_NormalsFilter->SetInput( index, m_ReduceFilter->GetOutput( index ) );
      m_DistanceImageCreator->SetInput( index, m_NormalsFilter->GetOutput( index ) );

      ++index;
    }
  }
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::PlanarFigureSegmentationController::GetImageBase(itk::Image<TPixel, VImageDimension>* input, itk::ImageBase<3>::Pointer& result)
{
  result = input;
}

mitk::Image::Pointer mitk::PlanarFigureSegmentationController::GetInterpolationResult()
{
  m_SegmentationAsImage = NULL;

  if ( m_PlanarFigureList.size() == 0 )
  {
    m_SegmentationAsImage = mitk::Image::New();
    m_SegmentationAsImage->Initialize(mitk::MakeScalarPixelType<unsigned char>() , *m_ReferenceImage->GetTimeGeometry());

    return m_SegmentationAsImage;
  }

  itk::ImageBase<3>::Pointer itkImage;
  AccessFixedDimensionByItk_1( m_ReferenceImage.GetPointer(), GetImageBase, 3, itkImage );
  m_DistanceImageCreator->SetReferenceImage( itkImage.GetPointer() );

  m_ReduceFilter->Update();
  m_NormalsFilter->Update();
  m_DistanceImageCreator->Update();

  mitk::Image::Pointer distanceImage = m_DistanceImageCreator->GetOutput();

  // Cleanup the pipeline
  distanceImage->DisconnectPipeline();
  m_DistanceImageCreator = NULL;
  m_NormalsFilter = NULL;
  m_ReduceFilter = NULL;
  itkImage = NULL;

  // If this bool flag is true, the distanceImage will be written to the
  // filesystem as nrrd-image and as surface-representation.
  bool debugOutput(false);
  if ( debugOutput )
  {
    mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
    imageWriter->SetInput( distanceImage );
    imageWriter->SetExtension( ".nrrd" );
    imageWriter->SetFileName( "v:/DistanceImage" );
    imageWriter->Update();
  }

  mitk::ImageToSurfaceFilter::Pointer imageToSurfaceFilter = mitk::ImageToSurfaceFilter::New();
  imageToSurfaceFilter->SetInput( distanceImage );
  imageToSurfaceFilter->SetThreshold( 0 );
  imageToSurfaceFilter->Update();

  mitk::Surface::Pointer segmentationAsSurface = imageToSurfaceFilter->GetOutput();

  // Cleanup the pipeline
  segmentationAsSurface->DisconnectPipeline();
  imageToSurfaceFilter = NULL;

  if ( debugOutput )
  {
    mitk::SurfaceVtkWriter<vtkPolyDataWriter>::Pointer surfaceWriter = mitk::SurfaceVtkWriter<vtkPolyDataWriter>::New();
    surfaceWriter->SetInput( segmentationAsSurface );
    surfaceWriter->SetExtension( ".vtk" );
    surfaceWriter->SetFileName( "v:/DistanceImageAsSurface.vtk" );
    surfaceWriter->Update();
  }


  mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();
  surfaceToImageFilter->SetInput( segmentationAsSurface );
  surfaceToImageFilter->SetImage( m_ReferenceImage );
  surfaceToImageFilter->SetMakeOutputBinary(true);
  surfaceToImageFilter->Update();

  m_SegmentationAsImage = surfaceToImageFilter->GetOutput();

  // Cleanup the pipeline
  m_SegmentationAsImage->DisconnectPipeline();

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

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New();
  vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();

  const mitk::PlaneGeometry* figureGeometry = figure->GetPlaneGeometry();

  // Get the polyline
  mitk::PlanarFigure::PolyLineType planarPolyLine = figure->GetPolyLine(0);
  mitk::PlanarFigure::PolyLineType::iterator iter;

  // iterate over the polyline, ...
  int pointCounter = 0;
  for( iter = planarPolyLine.begin(); iter != planarPolyLine.end(); iter++ )
  {
    // ... determine the world-coordinates
    mitk::Point3D pointInWorldCoordiantes;
    figureGeometry->Map( *iter, pointInWorldCoordiantes );

    // and add them as new points to the vtkPoints
    points->InsertNextPoint( pointInWorldCoordiantes[0], pointInWorldCoordiantes[1], pointInWorldCoordiantes[2] );
    ++pointCounter;
  }

  // create a polygon with the points of the polyline
  polygon->GetPointIds()->SetNumberOfIds( pointCounter );
  for(int i = 0; i < pointCounter; i++)
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
