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
: m_MaskingMode( MASKING_MODE_NONE )
{ 
  m_EmptyHistogram = HistogramType::New();
  HistogramType::SizeType histogramSize;
  histogramSize.Fill( 256 ); 
  m_EmptyHistogram->Initialize( histogramSize );

  m_EmptyStatistics.Reset();
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

    unsigned int numberOfTimeSteps = image->GetTimeSteps();

    // Initialize vectors to time-size of this image
    m_ImageHistogramVector.resize( numberOfTimeSteps );
    m_MaskedImageHistogramVector.resize( numberOfTimeSteps );
    m_PlanarFigureHistogramVector.resize( numberOfTimeSteps );

    m_ImageStatisticsVector.resize( numberOfTimeSteps );
    m_MaskedImageStatisticsVector.resize( numberOfTimeSteps );
    m_PlanarFigureStatisticsVector.resize( numberOfTimeSteps );

    m_ImageStatisticsTimeStampVector.resize( numberOfTimeSteps );
    m_MaskedImageStatisticsTimeStampVector.resize( numberOfTimeSteps );
    m_PlanarFigureStatisticsTimeStampVector.resize( numberOfTimeSteps );

    m_ImageStatisticsCalculationTriggerVector.resize( numberOfTimeSteps );
    m_MaskedImageStatisticsCalculationTriggerVector.resize( numberOfTimeSteps );
    m_PlanarFigureStatisticsCalculationTriggerVector.resize( numberOfTimeSteps );

    for ( unsigned int t = 0; t < image->GetTimeSteps(); ++t )
    {
      m_ImageStatisticsTimeStampVector[t].Modified();
      m_ImageStatisticsCalculationTriggerVector[t] = true;
    }
  }
}


void ImageStatisticsCalculator::SetImageMask( const mitk::Image *imageMask )
{
  if ( m_Image.IsNull() )
  {
    itkExceptionMacro( << "Image needs to be set first!" );
  }

  if ( m_Image->GetTimeSteps() != imageMask->GetTimeSteps() )
  {
    itkExceptionMacro( << "Image and image mask need to have equal number of time steps!" );
  }

  if ( m_ImageMask != imageMask )
  {
    m_ImageMask = imageMask;
    this->Modified();

    for ( unsigned int t = 0; t < m_Image->GetTimeSteps(); ++t )
    {
      m_MaskedImageStatisticsTimeStampVector[t].Modified();
      m_MaskedImageStatisticsCalculationTriggerVector[t] = true;
    }
  }
}


void ImageStatisticsCalculator::SetPlanarFigure( const mitk::PlanarFigure *planarFigure )
{
  if ( m_Image.IsNull() )
  {
    itkExceptionMacro( << "Image needs to be set first!" );
  }

  if ( m_PlanarFigure != planarFigure )
  {
    m_PlanarFigure = planarFigure;
    this->Modified();

    for ( unsigned int t = 0; t < m_Image->GetTimeSteps(); ++t )
    {
      m_PlanarFigureStatisticsTimeStampVector[t].Modified();
      m_PlanarFigureStatisticsCalculationTriggerVector[t] = true;
    }
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



bool ImageStatisticsCalculator::ComputeStatistics( unsigned int timeStep )
{
  if ( m_Image.IsNull() )
  {
    itkExceptionMacro( << "Image not set!" );
  }

  if ( timeStep >= m_Image->GetTimeSteps() )
  {
    throw std::runtime_error( "Error: invalid time step!" );
  }

  unsigned long imageMTime = m_ImageStatisticsTimeStampVector[timeStep].GetMTime();
  unsigned long maskedImageMTime = m_MaskedImageStatisticsTimeStampVector[timeStep].GetMTime();
  unsigned long planarFigureMTime = m_PlanarFigureStatisticsTimeStampVector[timeStep].GetMTime();

  bool imageStatisticsCalculationTrigger = m_ImageStatisticsCalculationTriggerVector[timeStep];
  bool maskedImageStatisticsCalculationTrigger = m_MaskedImageStatisticsCalculationTriggerVector[timeStep];
  bool planarFigureStatisticsCalculationTrigger = m_PlanarFigureStatisticsCalculationTriggerVector[timeStep];

  if ( ((m_MaskingMode != MASKING_MODE_NONE) || (imageMTime > m_Image->GetMTime() && !imageStatisticsCalculationTrigger))
    && ((m_MaskingMode != MASKING_MODE_IMAGE) || (maskedImageMTime > m_ImageMask->GetMTime() && !maskedImageStatisticsCalculationTrigger))
    && ((m_MaskingMode != MASKING_MODE_PLANARFIGURE) || (planarFigureMTime > m_PlanarFigure->GetMTime() && !planarFigureStatisticsCalculationTrigger)) )
  {
    // Statistics is up to date!
    return false;
  }

  // Depending on masking mode, extract and/or generate the required image
  // and mask data from the user input
  this->ExtractImageAndMask( timeStep );


  Statistics *statistics;
  HistogramType::ConstPointer *histogram;
  switch ( m_MaskingMode )
  {
  case MASKING_MODE_NONE:
  default:
    statistics = &m_ImageStatisticsVector[timeStep];
    histogram = &m_ImageHistogramVector[timeStep];

    m_ImageStatisticsTimeStampVector[timeStep].Modified();
    m_ImageStatisticsCalculationTriggerVector[timeStep] = false;
    break;

  case MASKING_MODE_IMAGE:
    statistics = &m_MaskedImageStatisticsVector[timeStep];
    histogram = &m_MaskedImageHistogramVector[timeStep];

    m_MaskedImageStatisticsTimeStampVector[timeStep].Modified();
    m_MaskedImageStatisticsCalculationTriggerVector[timeStep] = false;
    break;

  case MASKING_MODE_PLANARFIGURE:
    statistics = &m_PlanarFigureStatisticsVector[timeStep];
    histogram = &m_PlanarFigureHistogramVector[timeStep];

    m_PlanarFigureStatisticsTimeStampVector[timeStep].Modified();
    m_PlanarFigureStatisticsCalculationTriggerVector[timeStep] = false;
    break;
  }

  // Calculate statistics and histogram(s)
  if ( m_InternalImage->GetDimension() == 3 )
  {
    if ( m_MaskingMode == MASKING_MODE_NONE )
    {
      AccessFixedDimensionByItk_2( 
        m_InternalImage,
        InternalCalculateStatisticsUnmasked,
        3,
        *statistics,
        histogram );
    }
    else
    {
      AccessFixedDimensionByItk_3( 
        m_InternalImage,
        InternalCalculateStatisticsMasked,
        3,
        m_InternalImageMask3D.GetPointer(),
        *statistics,
        histogram );
    }
  }
  else if ( m_InternalImage->GetDimension() == 2 )
  {
    if ( m_MaskingMode == MASKING_MODE_NONE )
    {
      AccessFixedDimensionByItk_2( 
        m_InternalImage,
        InternalCalculateStatisticsUnmasked,
        2,
        *statistics,
        histogram );
    }
    else
    {
      AccessFixedDimensionByItk_3( 
        m_InternalImage,
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

  return true;
}


const ImageStatisticsCalculator::HistogramType *
ImageStatisticsCalculator::GetHistogram( unsigned int timeStep ) const
{
  if ( m_Image.IsNull() || (timeStep >= m_Image->GetTimeSteps()) )
  {
    return NULL;
  }

  switch ( m_MaskingMode )
  {
  case MASKING_MODE_NONE:
  default:
    return m_ImageHistogramVector[timeStep];

  case MASKING_MODE_IMAGE:
    return m_MaskedImageHistogramVector[timeStep];

  case MASKING_MODE_PLANARFIGURE:
    return m_PlanarFigureHistogramVector[timeStep];
  }
}


const ImageStatisticsCalculator::Statistics &
ImageStatisticsCalculator::GetStatistics( unsigned int timeStep ) const
{
  if ( m_Image.IsNull() || (timeStep >= m_Image->GetTimeSteps()) )
  {
    return m_EmptyStatistics;
  }

  switch ( m_MaskingMode )
  {
  case MASKING_MODE_NONE:
  default:
    return m_ImageStatisticsVector[timeStep];

  case MASKING_MODE_IMAGE:
    return m_MaskedImageStatisticsVector[timeStep];

  case MASKING_MODE_PLANARFIGURE:
    return m_PlanarFigureStatisticsVector[timeStep];
  }
}


void ImageStatisticsCalculator::ExtractImageAndMask( unsigned int timeStep )
{
  if ( m_Image.IsNull() )
  {
    throw std::runtime_error( "Error: image empty!" );
  }

  if ( timeStep >= m_Image->GetTimeSteps() )
  {
    throw std::runtime_error( "Error: invalid time step!" );
  }

  ImageTimeSelector::Pointer imageTimeSelector = ImageTimeSelector::New();
  imageTimeSelector->SetInput( m_Image );
  imageTimeSelector->SetTimeNr( timeStep );
  imageTimeSelector->UpdateLargestPossibleRegion();
  mitk::Image *timeSliceImage = imageTimeSelector->GetOutput();


  switch ( m_MaskingMode )
  {
  case MASKING_MODE_NONE:
    {
      m_InternalImage = timeSliceImage;
      m_InternalImageMask2D = NULL;
      m_InternalImageMask3D = NULL;
      break;
    }

  case MASKING_MODE_IMAGE:
    {
      if ( m_ImageMask.IsNotNull() )
      {
        if ( timeStep < m_ImageMask->GetTimeSteps() )
        {
          ImageTimeSelector::Pointer maskedImageTimeSelector = ImageTimeSelector::New();
          maskedImageTimeSelector->SetInput( m_ImageMask );
          maskedImageTimeSelector->SetTimeNr( timeStep );
          maskedImageTimeSelector->UpdateLargestPossibleRegion();
          mitk::Image *timeSliceMaskedImage = maskedImageTimeSelector->GetOutput();

          m_InternalImage = timeSliceImage;
          CastToItkImage( timeSliceMaskedImage, m_InternalImageMask3D );
        }
        else
        {
          throw std::runtime_error( "Error: image mask has not enough time steps!" );
        }
      }
      else
      {
        throw std::runtime_error( "Error: image mask empty!" );
      }
      break;
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

      const Geometry3D *imageGeometry = timeSliceImage->GetGeometry();
      if ( imageGeometry == NULL )
      {
        throw std::runtime_error( "Image geometry invalid!" );
      }

      const Geometry2D *planarFigureGeometry2D = m_PlanarFigure->GetGeometry2D();
      if ( planarFigureGeometry2D == NULL )
      {
        throw std::runtime_error( "Planar-Figure not yet initialized!" );
      }

      const PlaneGeometry *planarFigureGeometry = 
        dynamic_cast< const PlaneGeometry * >( planarFigureGeometry2D );
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
      imageExtractor->SetInput( timeSliceImage );
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
  histogramGenerator->SetNumberOfBins( 768 ); // CT range [-1024, +2048] --> bin size 4 values
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
    Point3D point3D;

    // Convert 2D point back to the local index coordinates of the selected
    // image
    planarFigureGeometry2D->Map( it->Value(), point3D );

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
