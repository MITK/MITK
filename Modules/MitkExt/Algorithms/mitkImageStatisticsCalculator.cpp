/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkImageStatisticsCalculator.h"
#include "mitkImageAccessByItk.h"
#include "mitkExtractImageFilter.h"

#include <itkScalarImageToHistogramGenerator.h>

#include <itkStatisticsImageFilter.h>
#include <itkLabelStatisticsImageFilter.h>
#include <itkStatisticsImageFilter.h>

#include <itkCastImageFilter.h>
#include <itkImageFileWriter.h>
#include <itkVTKImageImport.h>
#include <itkVTKImageExport.h>


#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkImageImport.h>
#include <vtkImageExport.h>
#include <vtkImageData.h>

#include <vtkMetaImageWriter.h>

#include <exception>




namespace mitk
{

ImageStatisticsCalculator::ImageStatisticsCalculator()
: m_MaskingMode( MASKING_MODE_NONE ),
  m_TriggerImageStatisticsCalculation( false ),
  m_TriggerMaskedImageStatisticsCalculation( false ),
  m_TriggerPlanarFigureStatisticsCalculation( false )
{
  m_InputTimeSelector = ImageTimeSelector::New();
  m_EmptyHistogram = HistogramType::New();
  HistogramType::SizeType histogramSize;
  histogramSize.Fill( 256 ); 
  m_EmptyHistogram->Initialize( histogramSize );
}


ImageStatisticsCalculator::~ImageStatisticsCalculator()
{
}


void ImageStatisticsCalculator::SetImage( const mitk::Image *image )
{
  if ( m_Image != image )
  {
    m_Image = image;
    this->Modified();
    m_ImageStatisticsTimeStamp.Modified();
    m_TriggerImageStatisticsCalculation = true;
  }
}


void ImageStatisticsCalculator::SetImageMask( const mitk::Image *imageMask )
{
  if ( m_ImageMask != imageMask )
  {
    m_ImageMask = imageMask;
    this->Modified();
    m_MaskedImageStatisticsTimeStamp.Modified();
    m_TriggerMaskedImageStatisticsCalculation = true;
  }
}


void ImageStatisticsCalculator::SetPlanarFigure( const mitk::PlanarFigure *planarFigure )
{
  if ( m_PlanarFigure != planarFigure )
  {
    m_PlanarFigure = planarFigure;
    this->Modified();
    m_PlanarFigureStatisticsTimeStamp.Modified();
    m_TriggerPlanarFigureStatisticsCalculation = true;
  }
}


void ImageStatisticsCalculator::SetMaskingModeToNone()
{
  if ( m_MaskingMode != MASKING_MODE_NONE )
  {
    m_MaskingMode = MASKING_MODE_NONE;
    this->Modified();
  }
}


void ImageStatisticsCalculator::SetMaskingModeToImage()
{
  if ( m_MaskingMode != MASKING_MODE_IMAGE )
  {
    m_MaskingMode = MASKING_MODE_IMAGE;
    this->Modified();
  }
}


void ImageStatisticsCalculator::SetMaskingModeToPlanarFigure()
{
  if ( m_MaskingMode != MASKING_MODE_PLANARFIGURE )
  {
    m_MaskingMode = MASKING_MODE_PLANARFIGURE;
    this->Modified();
  }
}



void ImageStatisticsCalculator::ComputeStatistics()
{
  if ( m_Image.IsNull() )
  {
    itkExceptionMacro( << "Image not set!" );
    return;
  }

  unsigned long imageMTime = m_ImageStatisticsTimeStamp.GetMTime();
  unsigned long maskedImageMTime = m_MaskedImageStatisticsTimeStamp.GetMTime();
  unsigned long planarFigureMTime = m_PlanarFigureStatisticsTimeStamp.GetMTime();

  if ( ((m_MaskingMode != MASKING_MODE_NONE) || (imageMTime > m_Image->GetMTime() && !m_TriggerImageStatisticsCalculation))
    && ((m_MaskingMode != MASKING_MODE_IMAGE) || (maskedImageMTime > m_ImageMask->GetMTime() && !m_TriggerMaskedImageStatisticsCalculation))
    && ((m_MaskingMode != MASKING_MODE_PLANARFIGURE) || (planarFigureMTime > m_PlanarFigure->GetMTime() && !m_TriggerPlanarFigureStatisticsCalculation)) )
  {
    // Statistics is up to date!
    return;
  }

  // Depending on masking mode, extract and/or generate the required image
  // and mask data from the user input
  this->ExtractImageAndMask();


  m_InputTimeSelector->SetInput( m_InternalImage );

  //const TimeSlicedGeometry *timeSlicedGeometry = m_Image->GetTimeSlicedGeometry();
  //for( unsigned int t = 0; t < timeSlicedGeometry->GetTimeSteps(); ++t )
  //{
  m_InputTimeSelector->SetTimeNr( 0 );
  m_InputTimeSelector->UpdateLargestPossibleRegion();

  mitk::Image *timeSliceImage = m_InputTimeSelector->GetOutput();

  Statistics *statistics;
  HistogramType::ConstPointer *histogram;
  switch ( m_MaskingMode )
  {
  case MASKING_MODE_NONE:
  default:
    statistics = &m_ImageStatistics;
    histogram = &m_ImageHistogram;
    break;

  case MASKING_MODE_IMAGE:
    statistics = &m_MaskedImageStatistics;
    histogram = &m_MaskedImageHistogram;
    break;

  case MASKING_MODE_PLANARFIGURE:
    statistics = &m_PlanarFigureStatistics;
    histogram = &m_PlanarFigureHistogram;
    break;
  }

  // Calculate statistics and histogram(s)
  if ( timeSliceImage->GetDimension() == 3 )
  {
    if ( m_MaskingMode == MASKING_MODE_NONE )
    {
      AccessFixedDimensionByItk_2( 
        timeSliceImage,
        InternalCalculateStatisticsUnmasked,
        3,
        *statistics,
        histogram );
    }
    else
    {
      AccessFixedDimensionByItk_3( 
        timeSliceImage,
        InternalCalculateStatisticsMasked,
        3,
        m_InternalImageMask3D.GetPointer(),
        *statistics,
        histogram );
    }
  }
  else if ( timeSliceImage->GetDimension() == 2 )
  {
    if ( m_MaskingMode == MASKING_MODE_NONE )
    {
      AccessFixedDimensionByItk_2( 
        timeSliceImage,
        InternalCalculateStatisticsUnmasked,
        2,
        *statistics,
        histogram );
    }
    else
    {
      AccessFixedDimensionByItk_3( 
        timeSliceImage,
        InternalCalculateStatisticsMasked,
        2,
        m_InternalImageMask2D.GetPointer(),
        *statistics,
        histogram );
    }
  }
  else
  {
    LOG_ERROR << "ImageStatistics: Image dimension not supported!";
  }


  switch ( m_MaskingMode )
  {
  case MASKING_MODE_NONE:
  default:
    m_ImageStatisticsTimeStamp.Modified();
    m_TriggerImageStatisticsCalculation = false;
    break;

  case MASKING_MODE_IMAGE:
    m_MaskedImageStatisticsTimeStamp.Modified();
    m_TriggerImageStatisticsCalculation = false;
    break;

  case MASKING_MODE_PLANARFIGURE:
    m_PlanarFigureStatisticsTimeStamp.Modified();
    m_TriggerImageStatisticsCalculation = false;
    break;
  }
}


const ImageStatisticsCalculator::HistogramType *
ImageStatisticsCalculator::GetHistogram() const
{
  switch ( m_MaskingMode )
  {
  case MASKING_MODE_NONE:
  default:
    return m_ImageHistogram;

  case MASKING_MODE_IMAGE:
    return m_MaskedImageHistogram;

  case MASKING_MODE_PLANARFIGURE:
    return m_PlanarFigureHistogram;
  }
}


const ImageStatisticsCalculator::Statistics &
ImageStatisticsCalculator::GetStatistics() const
{
  switch ( m_MaskingMode )
  {
  case MASKING_MODE_NONE:
  default:
    return m_ImageStatistics;

  case MASKING_MODE_IMAGE:
    return m_MaskedImageStatistics;

  case MASKING_MODE_PLANARFIGURE:
    return m_PlanarFigureStatistics;
  }
}


void ImageStatisticsCalculator::ExtractImageAndMask()
{
  if ( m_Image.IsNull() )
  {
    throw std::runtime_error( "Error: image empty!" );
  }

  switch ( m_MaskingMode )
  {
  case MASKING_MODE_NONE:
    {
      m_InternalImage = m_Image;
      m_InternalImageMask2D = NULL;
      m_InternalImageMask3D = NULL;
      break;
    }

  case MASKING_MODE_IMAGE:
    {
      if ( m_InternalImageMask3D.IsNotNull() )
      {
        m_InternalImage = m_Image;
        CastToItkImage( m_ImageMask, m_InternalImageMask3D );
        break;
      }
      else
      {
        throw std::runtime_error( "Error: image mask empty!" );
      }
    }

  case MASKING_MODE_PLANARFIGURE:
    {
      m_InternalImageMask2D = NULL;

      if ( m_PlanarFigure.IsNull() )
      {
        throw std::runtime_error( "Error: planar figure empty!" );
      }
      if ( !m_PlanarFigure->IsClosed() )
      {
        throw std::runtime_error( "Masking not possible for non-closed figures" );
      }

      const Geometry3D *imageGeometry = m_Image->GetGeometry();
      const PlaneGeometry *planarFigureGeometry = 
        dynamic_cast< const PlaneGeometry * >( m_PlanarFigure->GetGeometry() );
      if ( planarFigureGeometry == NULL )
      {
        throw std::runtime_error( "Non-planar planar figures not supported!" );
      }

      // Find principal direction of PlanarFigure in input image
      unsigned int axis;
      if ( !this->GetPrincipalAxis( imageGeometry,
        planarFigureGeometry->GetNormal(), axis ) )
      {
        throw std::runtime_error( "Non-aligned planar figures not supported!" );
      }


      // Find slice number corresponding to PlanarFigure in input image
      MaskImage3DType::IndexType index;
      imageGeometry->WorldToIndex( planarFigureGeometry->GetOrigin(), index );

      unsigned int slice = index[axis];


      // Extract slice with given position and direction from image
      ExtractImageFilter::Pointer imageExtractor = ExtractImageFilter::New();
      imageExtractor->SetInput( m_Image );
      imageExtractor->SetSliceDimension( axis );
      imageExtractor->SetSliceIndex( slice );
      imageExtractor->Update();
      m_InternalImage = imageExtractor->GetOutput();


      // Compute mask from PlanarFigure
      AccessFixedDimensionByItk_1( 
        m_InternalImage,
        InternalCalculateMaskFromPlanarFigure,
        2, axis );
    }
  }
}


bool ImageStatisticsCalculator::GetPrincipalAxis( 
  const Geometry3D *geometry, Vector3D vector,
  unsigned int &axis )
{
  vector.Normalize();
  for ( unsigned int i = 0; i < 3; ++i )
  {
    Vector3D axisVector = geometry->GetAxisVector( i );
    axisVector.Normalize();

    if ( fabs( fabs( axisVector * vector ) - 1.0) < mitk::eps )
    {
      axis = i;
      return true;
    }
  }

  return false;
}


template < typename TPixel, unsigned int VImageDimension >
void ImageStatisticsCalculator::InternalCalculateStatisticsUnmasked(
  const itk::Image< TPixel, VImageDimension > *image,
  Statistics &statistics,
  typename HistogramType::ConstPointer *histogram )
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;
  typedef itk::Image< unsigned short, VImageDimension > MaskImageType;
  typedef typename ImageType::IndexType IndexType;

  typedef itk::Statistics::ScalarImageToHistogramGenerator< ImageType >
    HistogramGeneratorType;

  typename HistogramGeneratorType::Pointer histogramGenerator = HistogramGeneratorType::New();
  histogramGenerator->SetInput( image );
  histogramGenerator->SetMarginalScale( 100 ); // Defines y-margin width of histogram
  histogramGenerator->SetNumberOfBins( 384 );
  histogramGenerator->SetHistogramMin( -1024.0 );
  histogramGenerator->SetHistogramMax( 2048.0 );
  histogramGenerator->Compute();

  *histogram = histogramGenerator->GetOutput(); 

  typedef itk::StatisticsImageFilter< ImageType > StatisticsFilterType;
  typename StatisticsFilterType::Pointer statisticsFilter = StatisticsFilterType::New();
  statisticsFilter->SetInput( image );
  statisticsFilter->Update();

  statistics.N = image->GetBufferedRegion().GetNumberOfPixels();
  statistics.Min = statisticsFilter->GetMinimum();
  statistics.Max = statisticsFilter->GetMaximum();
  statistics.Mean = statisticsFilter->GetMean();
  statistics.Median = 0.0;
  statistics.Sigma = statisticsFilter->GetSigma();
  statistics.RMS = sqrt( statistics.Mean * statistics.Mean 
    + statistics.Sigma * statistics.Sigma );
}


template < typename TPixel, unsigned int VImageDimension >
void ImageStatisticsCalculator::InternalCalculateStatisticsMasked(
  const itk::Image< TPixel, VImageDimension > *image,
  itk::Image< unsigned short, VImageDimension > *maskImage,
  Statistics &statistics,
  typename HistogramType::ConstPointer *histogram )
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;
  typedef itk::Image< unsigned short, VImageDimension > MaskImageType;
  typedef typename ImageType::IndexType IndexType;

  typedef itk::LabelStatisticsImageFilter< ImageType, MaskImageType >
    LabelStatisticsFilterType;

  typename LabelStatisticsFilterType::Pointer labelStatisticsFilter;
  unsigned int i;

  bool maskNonEmpty = false;
  if ( maskImage != NULL )
  {
    labelStatisticsFilter = LabelStatisticsFilterType::New();

    labelStatisticsFilter->SetInput( image );
    labelStatisticsFilter->SetLabelInput( maskImage );
    labelStatisticsFilter->UseHistogramsOn();
    labelStatisticsFilter->SetHistogramParameters( 384, -1024.0, 2048.0);
    labelStatisticsFilter->Update();

    // Find label of mask (other than 0)
    for ( i = 1; i < 4096; ++i )
    {
      if ( labelStatisticsFilter->HasLabel( i ) )
      {
        maskNonEmpty = true;
        break;
      }
    }
  }

  if ( maskNonEmpty )
  {
    *histogram = labelStatisticsFilter->GetHistogram( i );
    statistics.N = labelStatisticsFilter->GetCount( i );
    statistics.Min = labelStatisticsFilter->GetMinimum( i );
    statistics.Max = labelStatisticsFilter->GetMaximum( i );
    statistics.Mean = labelStatisticsFilter->GetMean( i );
    statistics.Median = labelStatisticsFilter->GetMedian( i );
    statistics.Sigma = labelStatisticsFilter->GetSigma( i );
    statistics.RMS = sqrt( statistics.Mean * statistics.Mean 
      + statistics.Sigma * statistics.Sigma );
  }
  else
  {
    *histogram = m_EmptyHistogram;
    statistics.Reset();
  }
}


template < typename TPixel, unsigned int VImageDimension >
void ImageStatisticsCalculator::InternalCalculateMaskFromPlanarFigure(
  const itk::Image< TPixel, VImageDimension > *image, unsigned int axis )
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;

  typedef itk::CastImageFilter< ImageType, MaskImage2DType > CastFilterType;

  // Generate mask image as new image with same header as input image and
  // initialize with "1".
  typename CastFilterType::Pointer castFilter = CastFilterType::New();
  castFilter->SetInput( image );
  castFilter->Update();
  castFilter->GetOutput()->FillBuffer( 1 );

  // Generate VTK polygon from (closed) PlanarFigure polyline
  // (The polyline points are shifted by -0.5 in z-direction to make sure
  // that the extrusion filter, which afterwards elevates all points by +0.5
  // in z-direction, creates a 3D object which is cut by the the plane z=0)
  const mitk::Geometry2D *planarFigureGeometry2D = m_PlanarFigure->GetGeometry2D();
  const typename PlanarFigure::VertexContainerType *planarFigurePolyline = m_PlanarFigure->GetPolyLine( 0 );
  const mitk::Geometry3D *imageGeometry3D = m_Image->GetGeometry( 0 );

  vtkPolyData *polyline = vtkPolyData::New();
  polyline->Allocate( 1, 1 );

  // Determine x- and y-dimensions depending on principal axis
  int i0, i1;
  switch ( axis )
  {
    case 0:
      i0 = 1;
      i1 = 2;
      break;

    case 1:
      i0 = 0;
      i1 = 2;
      break;

    case 2:
      i0 = 0;
      i1 = 1;
      break;
  }

  // Create VTK polydata object of polyline contour
  bool outOfBounds = false;
  vtkPoints *points = vtkPoints::New();
  typename PlanarFigure::VertexContainerType::ConstIterator it;
  for ( it = planarFigurePolyline->Begin();
        it != planarFigurePolyline->End();
        ++it )
  {
    Point2D point2D;
    Point3D point3D;

    // Convert index point to world coordinates and back to the local index
    // coordinates of the selected image
    planarFigureGeometry2D->IndexToWorld( it->Value(), point2D );
    planarFigureGeometry2D->Map( point2D, point3D );

    // Polygons (partially) outside of the image bounds can not be processed
    // further due to a bug in vtkPolyDataToImageStencil
    if ( !imageGeometry3D->IsInside( point3D ) )
    {
      outOfBounds = true;
    }

    imageGeometry3D->WorldToIndex( point3D, point3D );

    // Add point to polyline array
    points->InsertNextPoint( point3D[i0], point3D[i1], -0.5 );
  }
  polyline->SetPoints( points );
  points->Delete();

  if ( outOfBounds )
  {
    polyline->Delete();
    throw std::runtime_error( "Figure at least partially outside of image bounds!" );
  }

  unsigned int numberOfPoints = planarFigurePolyline->Size();
  vtkIdType *ptIds = new vtkIdType[numberOfPoints];
  for ( vtkIdType i = 0; i < numberOfPoints; ++i )
  {
    ptIds[i] = i;
  }
  polyline->InsertNextCell( VTK_POLY_LINE, numberOfPoints, ptIds );


  // Extrude the generated contour polygon
  vtkLinearExtrusionFilter *extrudeFilter = vtkLinearExtrusionFilter::New();
  extrudeFilter->SetInput( polyline );
  extrudeFilter->SetScaleFactor( 1 );
  extrudeFilter->SetExtrusionTypeToNormalExtrusion();
  extrudeFilter->SetVector( 0.0, 0.0, 1.0 );

  // Make a stencil from the extruded polygon
  vtkPolyDataToImageStencil *polyDataToImageStencil = vtkPolyDataToImageStencil::New();
  polyDataToImageStencil->SetInput( extrudeFilter->GetOutput() );



  // Export from ITK to VTK (to use a VTK filter)
  typedef itk::VTKImageImport< MaskImage2DType > ImageImportType;
  typedef itk::VTKImageExport< MaskImage2DType > ImageExportType;

  typename ImageExportType::Pointer itkExporter = ImageExportType::New();
  itkExporter->SetInput( castFilter->GetOutput() );

  vtkImageImport *vtkImporter = vtkImageImport::New();
  this->ConnectPipelines( itkExporter, vtkImporter );
  vtkImporter->Update();


  // Apply the generated image stencil to the input image
  vtkImageStencil *imageStencilFilter = vtkImageStencil::New();
  imageStencilFilter->SetInput( vtkImporter->GetOutput() );
  imageStencilFilter->SetStencil( polyDataToImageStencil->GetOutput() );
  imageStencilFilter->ReverseStencilOff();
  imageStencilFilter->SetBackgroundValue( 0 );
  imageStencilFilter->Update();


  // Export from VTK back to ITK
  vtkImageExport *vtkExporter = vtkImageExport::New();
  vtkExporter->SetInput( imageStencilFilter->GetOutput() );
  vtkExporter->Update();

  typename ImageImportType::Pointer itkImporter = ImageImportType::New();
  this->ConnectPipelines( vtkExporter, itkImporter );
  itkImporter->Update();


  //typedef itk::ImageFileWriter< MaskImage2DType > FileWriterType;
  //FileWriterType::Pointer writer = FileWriterType::New();
  //writer->SetInput( itkImporter->GetOutput() );
  //writer->SetFileName( "c:/test.mhd" );
  //writer->Update();


  // Store mask
  m_InternalImageMask2D = itkImporter->GetOutput();


  // Clean up VTK objects
  polyline->Delete();
  extrudeFilter->Delete();
  polyDataToImageStencil->Delete();
  vtkImporter->Delete();
  imageStencilFilter->Delete();
  //vtkExporter->Delete(); // TODO: crashes when outcommented; memory leak??
  delete[] ptIds;
}

}
