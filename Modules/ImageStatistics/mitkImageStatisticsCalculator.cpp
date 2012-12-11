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


#include "mitkImageStatisticsCalculator.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkExtractImageFilter.h"

#include <itkScalarImageToHistogramGenerator.h>

#include <itkChangeInformationImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkStatisticsImageFilter.h>
#include <itkLabelStatisticsImageFilter.h>
#include <itkMaskImageFilter.h>

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

#include <list>

#if ( ( VTK_MAJOR_VERSION <= 5 ) && ( VTK_MINOR_VERSION<=8)  )
  #include "mitkvtkLassoStencilSource.h"
#else
  #include "vtkLassoStencilSource.h"
#endif


#include <vtkMetaImageWriter.h>

#include <exception>

namespace mitk
{

ImageStatisticsCalculator::ImageStatisticsCalculator()
: m_MaskingMode( MASKING_MODE_NONE ),
  m_MaskingModeChanged( false ),
  m_IgnorePixelValue(0.0),
  m_DoIgnorePixelValue(false),
  m_IgnorePixelValueChanged(false)
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


void ImageStatisticsCalculator::SetPlanarFigure( mitk::PlanarFigure *planarFigure )
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


void ImageStatisticsCalculator::SetMaskingMode( unsigned int mode )
{
  if ( m_MaskingMode != mode )
  {
    m_MaskingMode = mode;
    m_MaskingModeChanged = true;
    this->Modified();
  }
}


void ImageStatisticsCalculator::SetMaskingModeToNone()
{
  if ( m_MaskingMode != MASKING_MODE_NONE )
  {
    m_MaskingMode = MASKING_MODE_NONE;
    m_MaskingModeChanged = true;
    this->Modified();
  }
}


void ImageStatisticsCalculator::SetMaskingModeToImage()
{
  if ( m_MaskingMode != MASKING_MODE_IMAGE )
  {
    m_MaskingMode = MASKING_MODE_IMAGE;
    m_MaskingModeChanged = true;
    this->Modified();
  }
}


void ImageStatisticsCalculator::SetMaskingModeToPlanarFigure()
{
  if ( m_MaskingMode != MASKING_MODE_PLANARFIGURE )
  {
    m_MaskingMode = MASKING_MODE_PLANARFIGURE;
    m_MaskingModeChanged = true;
    this->Modified();
  }
}

void ImageStatisticsCalculator::SetIgnorePixelValue(double value)
{
  if ( m_IgnorePixelValue != value )
  {
    m_IgnorePixelValue = value;
    if(m_DoIgnorePixelValue)
    {
      m_IgnorePixelValueChanged = true;
    }
    this->Modified();
  }
}

double ImageStatisticsCalculator::GetIgnorePixelValue()
{
  return m_IgnorePixelValue;
}

void ImageStatisticsCalculator::SetDoIgnorePixelValue(bool value)
{
  if ( m_DoIgnorePixelValue != value )
  {
    m_DoIgnorePixelValue = value;
    m_IgnorePixelValueChanged = true;
    this->Modified();
  }
}

bool ImageStatisticsCalculator::GetDoIgnorePixelValue()
{
  return m_DoIgnorePixelValue;
}

bool ImageStatisticsCalculator::ComputeStatistics( unsigned int timeStep )
{

  if (m_Image.IsNull() )
  {
    mitkThrow() << "Image not set!";
  }

  if (!m_Image->IsInitialized())
  {
    mitkThrow() << "Image not initialized!";
  }

  if ( m_Image->GetReferenceCount() == 1 )
  {
    // Image no longer valid; we are the only ones to still hold a reference on it
    return false;
  }

  if ( timeStep >= m_Image->GetTimeSteps() )
  {
    throw std::runtime_error( "Error: invalid time step!" );
  }

  // If a mask was set but we are the only ones to still hold a reference on
  // it, delete it.
  if ( m_ImageMask.IsNotNull() && (m_ImageMask->GetReferenceCount() == 1) )
  {
    m_ImageMask = NULL;
  }


  // Check if statistics is already up-to-date
  unsigned long imageMTime = m_ImageStatisticsTimeStampVector[timeStep].GetMTime();
  unsigned long maskedImageMTime = m_MaskedImageStatisticsTimeStampVector[timeStep].GetMTime();
  unsigned long planarFigureMTime = m_PlanarFigureStatisticsTimeStampVector[timeStep].GetMTime();

  bool imageStatisticsCalculationTrigger = m_ImageStatisticsCalculationTriggerVector[timeStep];
  bool maskedImageStatisticsCalculationTrigger = m_MaskedImageStatisticsCalculationTriggerVector[timeStep];
  bool planarFigureStatisticsCalculationTrigger = m_PlanarFigureStatisticsCalculationTriggerVector[timeStep];

  if ( !m_IgnorePixelValueChanged
    && ((m_MaskingMode != MASKING_MODE_NONE) || (imageMTime > m_Image->GetMTime() && !imageStatisticsCalculationTrigger))
    && ((m_MaskingMode != MASKING_MODE_IMAGE) || (maskedImageMTime > m_ImageMask->GetMTime() && !maskedImageStatisticsCalculationTrigger))
    && ((m_MaskingMode != MASKING_MODE_PLANARFIGURE) || (planarFigureMTime > m_PlanarFigure->GetMTime() && !planarFigureStatisticsCalculationTrigger)) )
  {
    // Statistics is up to date!
    if ( m_MaskingModeChanged )
    {
      m_MaskingModeChanged = false;
      return true;
    }
    else
    {
      return false;
    }
  }

  // Reset state changed flag
  m_MaskingModeChanged = false;
  m_IgnorePixelValueChanged = false;

  // Depending on masking mode, extract and/or generate the required image
  // and mask data from the user input
  this->ExtractImageAndMask( timeStep );


  StatisticsContainer *statisticsContainer;
  HistogramContainer *histogramContainer;
  switch ( m_MaskingMode )
  {
  case MASKING_MODE_NONE:
  default:
    if(!m_DoIgnorePixelValue)
    {
      statisticsContainer = &m_ImageStatisticsVector[timeStep];
      histogramContainer = &m_ImageHistogramVector[timeStep];

      m_ImageStatisticsTimeStampVector[timeStep].Modified();
      m_ImageStatisticsCalculationTriggerVector[timeStep] = false;
    }
    else
    {
      statisticsContainer = &m_MaskedImageStatisticsVector[timeStep];
      histogramContainer = &m_MaskedImageHistogramVector[timeStep];

      m_MaskedImageStatisticsTimeStampVector[timeStep].Modified();
      m_MaskedImageStatisticsCalculationTriggerVector[timeStep] = false;
    }
    break;

  case MASKING_MODE_IMAGE:
    statisticsContainer = &m_MaskedImageStatisticsVector[timeStep];
    histogramContainer = &m_MaskedImageHistogramVector[timeStep];

    m_MaskedImageStatisticsTimeStampVector[timeStep].Modified();
    m_MaskedImageStatisticsCalculationTriggerVector[timeStep] = false;
    break;

  case MASKING_MODE_PLANARFIGURE:
    statisticsContainer = &m_PlanarFigureStatisticsVector[timeStep];
    histogramContainer = &m_PlanarFigureHistogramVector[timeStep];

    m_PlanarFigureStatisticsTimeStampVector[timeStep].Modified();
    m_PlanarFigureStatisticsCalculationTriggerVector[timeStep] = false;
    break;
  }

  // Calculate statistics and histogram(s)
  if ( m_InternalImage->GetDimension() == 3 )
  {
    if ( m_MaskingMode == MASKING_MODE_NONE && !m_DoIgnorePixelValue )
    {
      AccessFixedDimensionByItk_2(
        m_InternalImage,
        InternalCalculateStatisticsUnmasked,
        3,
        statisticsContainer,
        histogramContainer );
    }
    else
    {
      AccessFixedDimensionByItk_3(
        m_InternalImage,
        InternalCalculateStatisticsMasked,
        3,
        m_InternalImageMask3D.GetPointer(),
        statisticsContainer,
        histogramContainer );
    }
  }
  else if ( m_InternalImage->GetDimension() == 2 )
  {
    if ( m_MaskingMode == MASKING_MODE_NONE && !m_DoIgnorePixelValue )
    {
      AccessFixedDimensionByItk_2(
        m_InternalImage,
        InternalCalculateStatisticsUnmasked,
        2,
        statisticsContainer,
        histogramContainer );
    }
    else
    {
      AccessFixedDimensionByItk_3(
        m_InternalImage,
        InternalCalculateStatisticsMasked,
        2,
        m_InternalImageMask2D.GetPointer(),
        statisticsContainer,
        histogramContainer );
    }
  }
  else
  {
    MITK_ERROR << "ImageStatistics: Image dimension not supported!";
  }


  // Release unused image smart pointers to free memory
  m_InternalImage = mitk::Image::ConstPointer();
  m_InternalImageMask3D = MaskImage3DType::Pointer();
  m_InternalImageMask2D = MaskImage2DType::Pointer();
  return true;
}


const ImageStatisticsCalculator::HistogramType *
ImageStatisticsCalculator::GetHistogram( unsigned int timeStep, unsigned int label ) const
{
  if ( m_Image.IsNull() || (timeStep >= m_Image->GetTimeSteps()) )
  {
    return NULL;
  }

  switch ( m_MaskingMode )
  {
  case MASKING_MODE_NONE:
  default:
    {
      if(m_DoIgnorePixelValue)
        return m_MaskedImageHistogramVector[timeStep][label];

      return m_ImageHistogramVector[timeStep][label];
    }

  case MASKING_MODE_IMAGE:
    return m_MaskedImageHistogramVector[timeStep][label];

  case MASKING_MODE_PLANARFIGURE:
    return m_PlanarFigureHistogramVector[timeStep][label];
  }
}

const ImageStatisticsCalculator::HistogramContainer &
ImageStatisticsCalculator::GetHistogramVector( unsigned int timeStep ) const
{
  if ( m_Image.IsNull() || (timeStep >= m_Image->GetTimeSteps()) )
  {
    return m_EmptyHistogramContainer;
  }

  switch ( m_MaskingMode )
  {
  case MASKING_MODE_NONE:
  default:
    {
      if(m_DoIgnorePixelValue)
        return m_MaskedImageHistogramVector[timeStep];

      return m_ImageHistogramVector[timeStep];
    }

  case MASKING_MODE_IMAGE:
    return m_MaskedImageHistogramVector[timeStep];

  case MASKING_MODE_PLANARFIGURE:
    return m_PlanarFigureHistogramVector[timeStep];
  }
}


const ImageStatisticsCalculator::Statistics &
ImageStatisticsCalculator::GetStatistics( unsigned int timeStep, unsigned int label ) const
{
  if ( m_Image.IsNull() || (timeStep >= m_Image->GetTimeSteps()) )
  {
    return m_EmptyStatistics;
  }

  switch ( m_MaskingMode )
  {
  case MASKING_MODE_NONE:
  default:
    {
      if(m_DoIgnorePixelValue)
        return m_MaskedImageStatisticsVector[timeStep][label];

      return m_ImageStatisticsVector[timeStep][label];
    }
  case MASKING_MODE_IMAGE:
    return m_MaskedImageStatisticsVector[timeStep][label];

  case MASKING_MODE_PLANARFIGURE:
    return m_PlanarFigureStatisticsVector[timeStep][label];
  }
}


const ImageStatisticsCalculator::StatisticsContainer &
ImageStatisticsCalculator::GetStatisticsVector( unsigned int timeStep ) const
{
  if ( m_Image.IsNull() || (timeStep >= m_Image->GetTimeSteps()) )
  {
    return m_EmptyStatisticsContainer;
  }

  switch ( m_MaskingMode )
  {
  case MASKING_MODE_NONE:
  default:
    {
      if(m_DoIgnorePixelValue)
        return m_MaskedImageStatisticsVector[timeStep];

      return m_ImageStatisticsVector[timeStep];
    }
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

      if(m_DoIgnorePixelValue)
      {
        if( m_InternalImage->GetDimension() == 3 )
        {
          CastToItkImage( timeSliceImage, m_InternalImageMask3D );
          m_InternalImageMask3D->FillBuffer(1);
        }
        if( m_InternalImage->GetDimension() == 2 )
        {
          CastToItkImage( timeSliceImage, m_InternalImageMask2D );
          m_InternalImageMask2D->FillBuffer(1);
        }
      }
      break;
    }

  case MASKING_MODE_IMAGE:
    {
      if ( m_ImageMask.IsNotNull() && (m_ImageMask->GetReferenceCount() > 1) )
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

  if(m_DoIgnorePixelValue)
  {
    if ( m_InternalImage->GetDimension() == 3 )
    {
      AccessFixedDimensionByItk_1(
          m_InternalImage,
          InternalMaskIgnoredPixels,
          3,
          m_InternalImageMask3D.GetPointer() );
    }
    else if ( m_InternalImage->GetDimension() == 2 )
    {
      AccessFixedDimensionByItk_1(
          m_InternalImage,
          InternalMaskIgnoredPixels,
          2,
          m_InternalImageMask2D.GetPointer() );
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
  StatisticsContainer *statisticsContainer,
  HistogramContainer* histogramContainer )
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;
  typedef itk::Image< unsigned short, VImageDimension > MaskImageType;
  typedef typename ImageType::IndexType IndexType;

  typedef itk::Statistics::ScalarImageToHistogramGenerator< ImageType >
    HistogramGeneratorType;

  statisticsContainer->clear();
  histogramContainer->clear();

  // Progress listening...
  typedef itk::SimpleMemberCommand< ImageStatisticsCalculator > ITKCommandType;
  ITKCommandType::Pointer progressListener;
  progressListener = ITKCommandType::New();
  progressListener->SetCallbackFunction( this,
    &ImageStatisticsCalculator::UnmaskedStatisticsProgressUpdate );


  // Issue 100 artificial progress events since ScalarIMageToHistogramGenerator
  // does not (yet?) support progress reporting
  this->InvokeEvent( itk::StartEvent() );
  for ( unsigned int i = 0; i < 100; ++i )
  {
    this->UnmaskedStatisticsProgressUpdate();
  }

  // Calculate statistics (separate filter)
  typedef itk::StatisticsImageFilter< ImageType > StatisticsFilterType;
  typename StatisticsFilterType::Pointer statisticsFilter = StatisticsFilterType::New();
  statisticsFilter->SetInput( image );
  unsigned long observerTag = statisticsFilter->AddObserver( itk::ProgressEvent(), progressListener );
  statisticsFilter->Update();
  statisticsFilter->RemoveObserver( observerTag );
  this->InvokeEvent( itk::EndEvent() );

  // Calculate minimum and maximum
  typedef itk::MinimumMaximumImageCalculator< ImageType > MinMaxFilterType;
  typename MinMaxFilterType::Pointer minMaxFilter = MinMaxFilterType::New();
  minMaxFilter->SetImage( image );
  unsigned long observerTag2 = minMaxFilter->AddObserver( itk::ProgressEvent(), progressListener );
  minMaxFilter->Compute();
  minMaxFilter->RemoveObserver( observerTag2 );
  this->InvokeEvent( itk::EndEvent() );

  Statistics statistics; statistics.Reset();
  statistics.Label = 1;
  statistics.N = image->GetBufferedRegion().GetNumberOfPixels();
  statistics.Min = statisticsFilter->GetMinimum();
  statistics.Max = statisticsFilter->GetMaximum();
  statistics.Mean = statisticsFilter->GetMean();
  statistics.Median = 0.0;
  statistics.Sigma = statisticsFilter->GetSigma();
  statistics.RMS = sqrt( statistics.Mean * statistics.Mean + statistics.Sigma * statistics.Sigma );

  statistics.MinIndex.set_size(image->GetImageDimension());
  statistics.MaxIndex.set_size(image->GetImageDimension());
  for (int i=0; i<statistics.MaxIndex.size(); i++)
  {
      statistics.MaxIndex[i] = minMaxFilter->GetIndexOfMaximum()[i];
      statistics.MinIndex[i] = minMaxFilter->GetIndexOfMinimum()[i];
  }

  statisticsContainer->push_back( statistics );

   // Calculate histogram
  typename HistogramGeneratorType::Pointer histogramGenerator = HistogramGeneratorType::New();
  histogramGenerator->SetInput( image );
  histogramGenerator->SetMarginalScale( 100 );
  histogramGenerator->SetNumberOfBins( 768 );
  histogramGenerator->SetHistogramMin( statistics.Min );
  histogramGenerator->SetHistogramMax( statistics.Max );
  histogramGenerator->Compute();

  histogramContainer->push_back( histogramGenerator->GetOutput() );
}

template < typename TPixel, unsigned int VImageDimension >
void ImageStatisticsCalculator::InternalMaskIgnoredPixels(
  const itk::Image< TPixel, VImageDimension > *image,
  itk::Image< unsigned short, VImageDimension > *maskImage )
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;
  typedef itk::Image< unsigned short, VImageDimension > MaskImageType;

  itk::ImageRegionIterator<MaskImageType>
      itmask(maskImage, maskImage->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<ImageType>
      itimage(image, image->GetLargestPossibleRegion());

  itmask = itmask.Begin();
  itimage = itimage.Begin();

  while( !itmask.IsAtEnd() )
  {
    if(m_IgnorePixelValue == itimage.Get())
    {
      itmask.Set(0);
    }

    ++itmask;
    ++itimage;
  }
}

template < typename TPixel, unsigned int VImageDimension >
void ImageStatisticsCalculator::InternalCalculateStatisticsMasked(
  const itk::Image< TPixel, VImageDimension > *image,
  itk::Image< unsigned short, VImageDimension > *maskImage,
  StatisticsContainer* statisticsContainer,
  HistogramContainer* histogramContainer )
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;
  typedef itk::Image< unsigned short, VImageDimension > MaskImageType;

  typedef typename ImageType::IndexType IndexType;
  typedef typename ImageType::PointType PointType;
  typedef typename ImageType::SpacingType SpacingType;

  typedef itk::LabelStatisticsImageFilter< ImageType, MaskImageType > LabelStatisticsFilterType;

  typedef itk::ChangeInformationImageFilter< MaskImageType > ChangeInformationFilterType;

  typedef itk::ExtractImageFilter< ImageType, ImageType > ExtractImageFilterType;

  statisticsContainer->clear();
  histogramContainer->clear();


  // Make sure that mask is set
  if ( maskImage == NULL  )
  {
    itkExceptionMacro( << "Mask image needs to be set!" );
  }


  // Make sure that spacing of mask and image are the same
  SpacingType imageSpacing = image->GetSpacing();
  SpacingType maskSpacing = maskImage->GetSpacing();
  PointType zeroPoint; zeroPoint.Fill( 0.0 );
  if ( (zeroPoint + imageSpacing).SquaredEuclideanDistanceTo( (zeroPoint + maskSpacing) ) > mitk::eps )
  {
    itkExceptionMacro( << "Mask needs to have same spacing as image! (Image spacing: " << imageSpacing << "; Mask spacing: " << maskSpacing << ")" );
  }

  // Make sure that orientation of mask and image are the same
  typedef typename ImageType::DirectionType DirectionType;
  DirectionType imageDirection = image->GetDirection();
  DirectionType maskDirection = maskImage->GetDirection();
  for( int i = 0; i < imageDirection.ColumnDimensions; ++i )
  {
    for( int j = 0; j < imageDirection.ColumnDimensions; ++j )
    {
      double differenceDirection = imageDirection[i][j] - maskDirection[i][j];
      if ( fabs( differenceDirection ) > mitk::eps )
      {
        itkExceptionMacro( << "Mask needs to have same direction as image! (Image direction: " << imageDirection << "; Mask direction: " << maskDirection << ")" );
      }
    }
  }

  // Make sure that the voxels of mask and image are correctly "aligned", i.e., voxel boundaries are the same in both images
  PointType imageOrigin = image->GetOrigin();
  PointType maskOrigin = maskImage->GetOrigin();
  long offset[ImageType::ImageDimension];

  typedef itk::ContinuousIndex<double, VImageDimension> ContinousIndexType;
  ContinousIndexType maskOriginContinousIndex, imageOriginContinousIndex;

  image->TransformPhysicalPointToContinuousIndex(maskOrigin, maskOriginContinousIndex);
  image->TransformPhysicalPointToContinuousIndex(imageOrigin, imageOriginContinousIndex);

  for ( unsigned int i = 0; i < ImageType::ImageDimension; ++i )
  {
    double misalignment = maskOriginContinousIndex[i] - floor( maskOriginContinousIndex[i] + 0.5 );
    if ( fabs( misalignment ) > mitk::eps )
    {
      itkExceptionMacro( << "Pixels/voxels of mask and image are not sufficiently aligned! (Misalignment: " << misalignment << ")" );
    }

    double indexCoordDistance = maskOriginContinousIndex[i] - imageOriginContinousIndex[i];
    offset[i] = (int) indexCoordDistance + image->GetBufferedRegion().GetIndex()[i];
  }

  // Adapt the origin and region (index/size) of the mask so that the origin of both are the same
  typename ChangeInformationFilterType::Pointer adaptMaskFilter;
  adaptMaskFilter = ChangeInformationFilterType::New();
  adaptMaskFilter->ChangeOriginOn();
  adaptMaskFilter->ChangeRegionOn();
  adaptMaskFilter->SetInput( maskImage );
  adaptMaskFilter->SetOutputOrigin( image->GetOrigin() );
  adaptMaskFilter->SetOutputOffset( offset );
  adaptMaskFilter->Update();
  typename MaskImageType::Pointer adaptedMaskImage = adaptMaskFilter->GetOutput();


  // Make sure that mask region is contained within image region
  if ( !image->GetLargestPossibleRegion().IsInside( adaptedMaskImage->GetLargestPossibleRegion() ) )
  {
    itkExceptionMacro( << "Mask region needs to be inside of image region! (Image region: "
      << image->GetLargestPossibleRegion() << "; Mask region: " << adaptedMaskImage->GetLargestPossibleRegion() << ")" );
  }


  // If mask region is smaller than image region, extract the sub-sampled region from the original image
  typename ImageType::SizeType imageSize = image->GetBufferedRegion().GetSize();
  typename ImageType::SizeType maskSize = maskImage->GetBufferedRegion().GetSize();
  bool maskSmallerImage = false;
  for ( unsigned int i = 0; i < ImageType::ImageDimension; ++i )
  {
    if ( maskSize[i] < imageSize[i] )
    {
      maskSmallerImage = true;
    }
  }

  typename ImageType::ConstPointer adaptedImage;
  if ( maskSmallerImage )
  {
    typename ExtractImageFilterType::Pointer extractImageFilter = ExtractImageFilterType::New();
    extractImageFilter->SetInput( image );
    extractImageFilter->SetExtractionRegion( adaptedMaskImage->GetBufferedRegion() );
    extractImageFilter->Update();
    adaptedImage = extractImageFilter->GetOutput();
  }
  else
  {
    adaptedImage = image;
  }

  // Initialize Filter
  typedef itk::StatisticsImageFilter< ImageType > StatisticsFilterType;
  typename StatisticsFilterType::Pointer statisticsFilter = StatisticsFilterType::New();
  statisticsFilter->SetInput( adaptedImage );

  statisticsFilter->Update();

  int numberOfBins = ( m_DoIgnorePixelValue && (m_MaskingMode == MASKING_MODE_NONE) ) ? 768 : 384;
  typename LabelStatisticsFilterType::Pointer labelStatisticsFilter;
  labelStatisticsFilter = LabelStatisticsFilterType::New();
  labelStatisticsFilter->SetInput( adaptedImage );
  labelStatisticsFilter->SetLabelInput( adaptedMaskImage );
  labelStatisticsFilter->UseHistogramsOn();
  labelStatisticsFilter->SetHistogramParameters( numberOfBins, statisticsFilter->GetMinimum(), statisticsFilter->GetMaximum() );


  // Add progress listening
  typedef itk::SimpleMemberCommand< ImageStatisticsCalculator > ITKCommandType;
  ITKCommandType::Pointer progressListener;
  progressListener = ITKCommandType::New();
  progressListener->SetCallbackFunction( this,
    &ImageStatisticsCalculator::MaskedStatisticsProgressUpdate );
  unsigned long observerTag = labelStatisticsFilter->AddObserver(
    itk::ProgressEvent(), progressListener );


  // Execute filter
  this->InvokeEvent( itk::StartEvent() );


  // Make sure that only the mask region is considered (otherwise, if the mask region is smaller
  // than the image region, the Update() would result in an exception).
  labelStatisticsFilter->GetOutput()->SetRequestedRegion( adaptedMaskImage->GetLargestPossibleRegion() );


  // Execute the filter
  labelStatisticsFilter->Update();

  this->InvokeEvent( itk::EndEvent() );

  labelStatisticsFilter->RemoveObserver( observerTag );



  // Find all relevant labels of mask (other than 0)
  std::list< int > relevantLabels;
  bool maskNonEmpty = false;
  unsigned int i;
  for ( i = 1; i < 4096; ++i )
  {
    if ( labelStatisticsFilter->HasLabel( i ) )
    {
      relevantLabels.push_back( i );
      maskNonEmpty = true;
    }
  }

  typedef itk::MaskImageFilter< ImageType, MaskImageType, ImageType > MaskImageFilterType;
  typename MaskImageFilterType::Pointer masker = MaskImageFilterType::New();
  masker->SetOutsideValue( (statisticsFilter->GetMinimum()+statisticsFilter->GetMaximum())/2 );
  masker->SetInput1(adaptedImage);
  masker->SetInput2(adaptedMaskImage);
  masker->Update();

  // Calculate minimum and maximum
  typedef itk::MinimumMaximumImageCalculator< ImageType > MinMaxFilterType;
  typename MinMaxFilterType::Pointer minMaxFilter = MinMaxFilterType::New();
  minMaxFilter->SetImage( masker->GetOutput() );
  unsigned long observerTag2 = minMaxFilter->AddObserver( itk::ProgressEvent(), progressListener );
  minMaxFilter->Compute();
  minMaxFilter->RemoveObserver( observerTag2 );
  this->InvokeEvent( itk::EndEvent() );

  if ( maskNonEmpty )
  {
    std::list< int >::iterator it;
    for ( it = relevantLabels.begin(), i = 0;
          it != relevantLabels.end();
          ++it, ++i )
    {
      histogramContainer->push_back( HistogramType::ConstPointer( labelStatisticsFilter->GetHistogram( (*it) ) ) );

      Statistics statistics;
      statistics.Label = (*it);
      statistics.N = labelStatisticsFilter->GetCount( *it );
      statistics.Min = labelStatisticsFilter->GetMinimum( *it );
      statistics.Max = labelStatisticsFilter->GetMaximum( *it );
      statistics.Mean = labelStatisticsFilter->GetMean( *it );
      statistics.Median = labelStatisticsFilter->GetMedian( *it );
      statistics.Sigma = labelStatisticsFilter->GetSigma( *it );
      statistics.RMS = sqrt( statistics.Mean * statistics.Mean
        + statistics.Sigma * statistics.Sigma );

        statistics.MinIndex.set_size(adaptedImage->GetImageDimension());
        statistics.MaxIndex.set_size(adaptedImage->GetImageDimension());
        for (int i=0; i<statistics.MaxIndex.size(); i++)
        {
            statistics.MaxIndex[i] = minMaxFilter->GetIndexOfMaximum()[i];
            statistics.MinIndex[i] = minMaxFilter->GetIndexOfMinimum()[i];
        }

      statisticsContainer->push_back( statistics );
    }
  }
  else
  {
    histogramContainer->push_back( HistogramType::ConstPointer( m_EmptyHistogram ) );
    statisticsContainer->push_back( Statistics() );;
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

  // all PolylinePoints of the PlanarFigure are stored in a vtkPoints object.
  // These points are used by the vtkLassoStencilSource to create
  // a vtkImageStencil.
  const mitk::Geometry2D *planarFigureGeometry2D = m_PlanarFigure->GetGeometry2D();
  const typename PlanarFigure::PolyLineType planarFigurePolyline = m_PlanarFigure->GetPolyLine( 0 );
  const mitk::Geometry3D *imageGeometry3D = m_Image->GetGeometry( 0 );

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
    default:
      i0 = 0;
      i1 = 1;
      break;
  }

  // store the polyline contour as vtkPoints object
  bool outOfBounds = false;
  vtkSmartPointer<vtkPoints> points = vtkPoints::New();
  typename PlanarFigure::PolyLineType::const_iterator it;
  for ( it = planarFigurePolyline.begin();
        it != planarFigurePolyline.end();
        ++it )
  {
    Point3D point3D;

    // Convert 2D point back to the local index coordinates of the selected
    // image
    planarFigureGeometry2D->Map( it->Point, point3D );

    // Polygons (partially) outside of the image bounds can not be processed
    // further due to a bug in vtkPolyDataToImageStencil
    if ( !imageGeometry3D->IsInside( point3D ) )
    {
      outOfBounds = true;
    }

    imageGeometry3D->WorldToIndex( point3D, point3D );

    points->InsertNextPoint( point3D[i0], point3D[i1], 0 );
  }

  // mark a malformed 2D planar figure ( i.e. area = 0 ) as out of bounds
  // this can happen when all control points of a rectangle lie on the same line = two of the three extents are zero
  double bounds[6] = {0, 0, 0, 0, 0, 0};
  points->GetBounds( bounds );
  bool extent_x = (fabs(bounds[0] - bounds[1])) < mitk::eps;
  bool extent_y = (fabs(bounds[2] - bounds[3])) < mitk::eps;
  bool extent_z = (fabs(bounds[4] - bounds[5])) < mitk::eps;

  // throw an exception if a closed planar figure is deformed, i.e. has only one non-zero extent
  if ( m_PlanarFigure->IsClosed() &&
       ((extent_x && extent_y) || (extent_x && extent_z)  || (extent_y && extent_z)))
  {
    mitkThrow() << "Figure has a zero area and cannot be used for masking.";
  }

  if ( outOfBounds )
  {
    throw std::runtime_error( "Figure at least partially outside of image bounds!" );
  }

  // create a vtkLassoStencilSource and set the points of the Polygon
  vtkSmartPointer<vtkLassoStencilSource> lassoStencil = vtkLassoStencilSource::New();
  lassoStencil->SetShapeToPolygon();
  lassoStencil->SetPoints( points );

  // Export from ITK to VTK (to use a VTK filter)
  typedef itk::VTKImageImport< MaskImage2DType > ImageImportType;
  typedef itk::VTKImageExport< MaskImage2DType > ImageExportType;

  typename ImageExportType::Pointer itkExporter = ImageExportType::New();
  itkExporter->SetInput( castFilter->GetOutput() );

  vtkSmartPointer<vtkImageImport> vtkImporter = vtkImageImport::New();
  this->ConnectPipelines( itkExporter, vtkImporter );

  // Apply the generated image stencil to the input image
  vtkSmartPointer<vtkImageStencil> imageStencilFilter = vtkImageStencil::New();
  imageStencilFilter->SetInputConnection( vtkImporter->GetOutputPort() );
  imageStencilFilter->SetStencil( lassoStencil->GetOutput() );
  imageStencilFilter->ReverseStencilOff();
  imageStencilFilter->SetBackgroundValue( 0 );
  imageStencilFilter->Update();

  // Export from VTK back to ITK
  vtkSmartPointer<vtkImageExport> vtkExporter = vtkImageExport::New();
  vtkExporter->SetInputConnection( imageStencilFilter->GetOutputPort() );
  vtkExporter->Update();

  typename ImageImportType::Pointer itkImporter = ImageImportType::New();
  this->ConnectPipelines( vtkExporter, itkImporter );
  itkImporter->Update();

  // Store mask
  m_InternalImageMask2D = itkImporter->GetOutput();

  // Clean up VTK objects
  vtkImporter->Delete();
  imageStencilFilter->Delete();
  //vtkExporter->Delete(); // TODO: crashes when outcommented; memory leak??
}


void ImageStatisticsCalculator::UnmaskedStatisticsProgressUpdate()
{
  // Need to throw away every second progress event to reach a final count of
  // 100 since two consecutive filters are used in this case
  static int updateCounter = 0;
  if ( updateCounter++ % 2 == 0 )
  {
    this->InvokeEvent( itk::ProgressEvent() );
  }
}


void ImageStatisticsCalculator::MaskedStatisticsProgressUpdate()
{
  this->InvokeEvent( itk::ProgressEvent() );
}


}
