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
#include "mitkImageTimeSelector.h"
#include "mitkITKImageImport.h"

#include <itkScalarImageToHistogramGenerator.h>

#include <itkChangeInformationImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkStatisticsImageFilter.h>
#include <itkLabelStatisticsImageFilter.h>
#include <itkMaskImageFilter.h>

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>

#include <itkCastImageFilter.h>
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

#include <itkFFTConvolutionImageFilter.h>
#include <itkConstantBoundaryCondition.h>

#include <itkContinuousIndex.h>
#include <itkNumericTraits.h>
#include <list>

#include <exception>

//#define DEBUG_HOTSPOTSEARCH

#define _USE_MATH_DEFINES
#include <math.h>

#if ( ( VTK_MAJOR_VERSION <= 5 ) && ( VTK_MINOR_VERSION<=8)  )
  #include "mitkvtkLassoStencilSource.h"
#else
  #include "vtkLassoStencilSource.h"
#endif

// TODO DM: sort includes, check if they are really needed

namespace mitk
{

ImageStatisticsCalculator::ImageStatisticsCalculator()
: m_MaskingMode( MASKING_MODE_NONE ),
  m_MaskingModeChanged( false ),
  m_IgnorePixelValue(0.0),
  m_DoIgnorePixelValue(false),
  m_IgnorePixelValueChanged(false),
  m_PlanarFigureAxis (0),
  m_PlanarFigureSlice (0),
  m_PlanarFigureCoordinate0 (0),
  m_PlanarFigureCoordinate1 (0),          // TODO DM: check order of variable initialization
  m_HotspotRadiusInMM(6.2035049089940),   // radius of a 1cm3 sphere in mm
  m_CalculateHotspot(false),
  m_HotspotRadiusInMMChanged(false)
{
  m_EmptyHistogram = HistogramType::New();
  m_EmptyHistogram->SetMeasurementVectorSize(1);
  HistogramType::SizeType histogramSize(1);
  histogramSize.Fill( 256 );
  m_EmptyHistogram->Initialize( histogramSize );

 // m_EmptyStatistics.Reset();

}

ImageStatisticsCalculator::~ImageStatisticsCalculator()
{
}


ImageStatisticsCalculator::Statistics::Statistics()
: m_Label(0),
  m_N(0),
  m_Min(0.0),
  m_Max(0.0),
  m_Median(0.0),
  m_Mean(0.0),
  m_Sigma(0.0),
  m_RMS(0.0),
  m_MaxIndex(1,-1),      //TODO: Dimension anpassen
  m_MinIndex(1,-1),      //TODO: Dimension anpassen
  m_HotspotN(0),
  m_HotspotMin(0.0),
  m_HotspotMax(0.0),
  m_HotspotMedian(0.0),
  m_HotspotMean(0.0),
  m_HotspotSigma(0.0),
  m_HotspotRMS(0.0),
  m_HotspotIndex(1,-1),    //TODO: Dimension anpassen
  m_HotspotMaxIndex(1,-1), //TODO: Dimension anpassen
  m_HotspotMinIndex(1,-1)  //TODO: Dimension anpassen
{
}

ImageStatisticsCalculator::Statistics::~Statistics()
{
}

//ImageStatisticsCalculator::Statistics::Reset()
//{
//  m_Label = 0;
//  m_N = 0;
//  m_Min = 0.0;
//  m_Max = 0.0;
//  m_Median = 0.0;
//  m_Mean = 0.0;
//  m_Sigma = 0.0;
//  m_RMS = 0.0;
//  m_MaxIndex = 0;
//  m_MinIndex = 0;
//  m_Label = 0;
//  m_HotspotN = 0;
//  m_HotspotMin = 0.0;
//  m_HotspotMax = 0.0;
//  m_HotspotMedian = 0.0;
//  m_HotspotMean = 0.0;
//  m_HotspotSigma = 0.0;
//  m_HotspotRMS = 0.0;
//  m_HotspotIndex = 0;
//  m_HotspotMaxIndex = 0;
//  m_HotspotMinIndex = 0;
//}

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

void ImageStatisticsCalculator::SetHotspotRadiusInMM(double value)
{
  if ( m_HotspotRadiusInMM != value )
  {
    m_HotspotRadiusInMM = value;
    if(m_CalculateHotspot)
    {
      m_HotspotRadiusInMMChanged = true;
      MITK_INFO <<"Hotspot radius changed, new convolution required";
    }
    this->Modified();
  }
}

double ImageStatisticsCalculator::GetHotspotRadiusInMM()
{
  return m_HotspotRadiusInMM;
}

void ImageStatisticsCalculator::SetCalculateHotspot(bool on)
{
  if ( m_CalculateHotspot != on )
  {
    m_CalculateHotspot = on;
    m_HotspotRadiusInMMChanged = true;
    MITK_INFO <<"Hotspot calculation changed, new convolution required";
    this->Modified();
  }
}

bool ImageStatisticsCalculator::IsHotspotCalculated()
{
  return m_CalculateHotspot;
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
    && !m_HotspotRadiusInMMChanged
    && ((m_MaskingMode != MASKING_MODE_NONE) || (imageMTime > m_Image->GetMTime() && !imageStatisticsCalculationTrigger))
    && ((m_MaskingMode != MASKING_MODE_IMAGE) || (maskedImageMTime > m_ImageMask->GetMTime() && !maskedImageStatisticsCalculationTrigger))
    && ((m_MaskingMode != MASKING_MODE_PLANARFIGURE) || (planarFigureMTime > m_PlanarFigure->GetMTime() && !planarFigureStatisticsCalculationTrigger)) )
  {
    // Statistics is up to date!
    if ( m_MaskingModeChanged )
    {
      m_MaskingModeChanged = false;
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
      m_PlanarFigureAxis = axis;


      // Find slice number corresponding to PlanarFigure in input image
      MaskImage3DType::IndexType index;
      imageGeometry->WorldToIndex( planarFigureGeometry->GetOrigin(), index );

      unsigned int slice = index[axis];
      m_PlanarFigureSlice = slice;


      // Extract slice with given position and direction from image
      unsigned int dimension = timeSliceImage->GetDimension();

      if (dimension != 2)
      {
        ExtractImageFilter::Pointer imageExtractor = ExtractImageFilter::New();
        imageExtractor->SetInput( timeSliceImage );
        imageExtractor->SetSliceDimension( axis );
        imageExtractor->SetSliceIndex( slice );
        imageExtractor->Update();
        m_InternalImage = imageExtractor->GetOutput();
      }
      else
      {
        m_InternalImage = timeSliceImage;
      }

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

  MITK_DEBUG << "Update of convolution image required?\n  m_CalculateHotspot: " << m_CalculateHotspot
            << "\n  m_HotspotSearchConvolutionImage: " << (void*) m_HotspotSearchConvolutionImage.GetPointer()
            << "\n  m_ImageStatisticsCalculationTriggerVector["<<timeStep<<"]: " << m_ImageStatisticsCalculationTriggerVector[timeStep]
            << "\n m_HotspotRadiusInMMChanged" << m_HotspotRadiusInMMChanged
            << "\n  m_InternalImage::MTime: " << m_InternalImage->GetMTime()
            << "\n  ImageStatistics::MTime: " << this->GetMTime()
            << "\n  m_Image->GetMTime(): " << m_Image->GetMTime();

  if( m_CalculateHotspot
      &&
      (
        m_HotspotSearchConvolutionImage.IsNull()
        ||
        m_Image->GetMTime() > this->GetMTime() // TODO check when m_InternalImage 'really' changes; depends on timeStep
        ||
        m_HotspotRadiusInMMChanged == true
      )
    )
  {
    MITK_DEBUG <<"  --> Update required.";
    if ( m_InternalImage->GetDimension() == 3 )
    {
      AccessFixedDimensionByItk(
          m_InternalImage,
          InternalUpdateConvolutionImage,
          3 );
    }
    else if ( m_InternalImage->GetDimension() == 2 )
    {
      AccessFixedDimensionByItk(
          m_InternalImage,
          InternalUpdateConvolutionImage,
          2 );
    }
  }
  else
  {
    MITK_DEBUG <<"  --> Update required.";
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

  Statistics statistics; //statistics.Reset();
  statistics.SetLabel(1);
  statistics.SetN(image->GetBufferedRegion().GetNumberOfPixels());
  statistics.SetMin(statisticsFilter->GetMinimum());
  statistics.SetMax(statisticsFilter->GetMaximum());
  statistics.SetMean(statisticsFilter->GetMean());
  statistics.SetMedian(0.0);
  statistics.SetSigma(statisticsFilter->GetSigma());
  statistics.SetRMS(sqrt( statistics.GetMean() * statistics.GetMean() + statistics.GetSigma() * statistics.GetSigma() ));

  statistics.GetMinIndex().set_size(image->GetImageDimension());
  statistics.GetMaxIndex().set_size(image->GetImageDimension());

  vnl_vector<int> tmpMaxIndex;
  vnl_vector<int> tmpMinIndex;

  for (unsigned int i=0; i<statistics.GetMaxIndex().size(); i++)
  {
      tmpMaxIndex[i] = minMaxFilter->GetIndexOfMaximum()[i];
      tmpMinIndex[i] = minMaxFilter->GetIndexOfMinimum()[i];
  }

  statistics.SetMinIndex(tmpMaxIndex);
  statistics.SetMinIndex(tmpMinIndex);

  if( IsHotspotCalculated() )
  {
    typedef itk::Image< unsigned short, VImageDimension > MaskImageType;
    typename MaskImageType::Pointer maskImageITK = MaskImageType::New();

    maskImageITK->SetRegions ( image->GetLargestPossibleRegion() );
    maskImageITK->Allocate();
    typedef itk::ImageRegionIteratorWithIndex< MaskImageType > MaskImageIteratorType;

    MaskImageIteratorType maskIt(maskImageITK, image->GetLargestPossibleRegion());

    for(maskIt.GoToBegin();!maskIt.IsAtEnd(); ++maskIt)
      maskIt.Set(1);

    Image::Pointer maskImageMITK = ImportItkImage( maskImageITK );

    ImageStatisticsCalculator::Pointer hotspotCalculator = ImageStatisticsCalculator::New();
    hotspotCalculator->SetImage( this->m_Image );
    hotspotCalculator->SetMaskingModeToImage();
    hotspotCalculator->SetImageMask( maskImageMITK );
    hotspotCalculator->SetCalculateHotspot( true );
    hotspotCalculator->SetHotspotRadiusInMM( GetHotspotRadiusInMM() );
    hotspotCalculator->ComputeStatistics(0); // TODO: timestep

    Statistics hotspotStatistics = hotspotCalculator->GetStatistics();
    ImageExtrema hotspotExtrema = CalculateExtremaWorld(image, maskImageITK.GetPointer());

    statistics.SetHotspotN(hotspotStatistics.GetHotspotN());
    statistics.SetHotspotMax (hotspotStatistics.GetHotspotMax());
    statistics.SetHotspotMin(hotspotStatistics.GetHotspotMin());
    statistics.SetHotspotMean(hotspotStatistics.GetHotspotMean());
    statistics.SetHotspotMedian(0.0);
    statistics.SetHotspotSigma (hotspotStatistics.GetHotspotSigma());
    statistics.SetHotspotRMS (sqrt (statistics.GetHotspotMean() * statistics.GetHotspotMean()
      + statistics.GetHotspotSigma() * statistics.GetHotspotSigma() ));
    statistics.SetHotspotMaxIndex(hotspotStatistics.GetHotspotMaxIndex());
    statistics.SetHotspotMinIndex(hotspotStatistics.GetHotspotMinIndex());
    statistics.SetHotspotIndex(hotspotExtrema.MaxIndex);
  }

  statisticsContainer->push_back( statistics );

   // Calculate histogram
  typename HistogramGeneratorType::Pointer histogramGenerator = HistogramGeneratorType::New();
  histogramGenerator->SetInput( image );
  histogramGenerator->SetMarginalScale( 100 );
  histogramGenerator->SetNumberOfBins( 768 );
  histogramGenerator->SetHistogramMin( statistics.GetMin() );
  histogramGenerator->SetHistogramMax( statistics.GetMax() );
  histogramGenerator->Compute();

  // TODO DM: add hotspot search here!

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

  itmask.GoToBegin();
  itimage.GoToBegin();

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

  MITK_INFO <<"Mask has pixels? " << maskNonEmpty;
  if ( maskNonEmpty )
  {
    Statistics statistics; // restore previous code
    std::list< int >::iterator it;
    for ( it = relevantLabels.begin(), i = 0;
          it != relevantLabels.end();
          ++it, ++i )
    {
      histogramContainer->push_back( HistogramType::ConstPointer( labelStatisticsFilter->GetHistogram( (*it) ) ) );

      statistics.SetLabel (*it);
      statistics.SetN(labelStatisticsFilter->GetCount( *it ));
      statistics.SetMin(labelStatisticsFilter->GetMinimum( *it ));
      statistics.SetMax(labelStatisticsFilter->GetMaximum( *it ));
      statistics.SetMean(labelStatisticsFilter->GetMean( *it ));
      statistics.SetMedian(labelStatisticsFilter->GetMedian( *it ));
      statistics.SetSigma(labelStatisticsFilter->GetSigma( *it ));
      statistics.SetRMS(sqrt( statistics.GetMean() * statistics.GetMean()
        + statistics.GetSigma() * statistics.GetSigma() ));

      // restrict image to mask area for min/max index calculation
      typedef itk::MaskImageFilter< ImageType, MaskImageType, ImageType > MaskImageFilterType;
      typename MaskImageFilterType::Pointer masker = MaskImageFilterType::New();
      masker->SetOutsideValue( (statistics.GetMin()+statistics.GetMax())/2 );
      masker->SetInput1(adaptedImage);
      masker->SetInput2(adaptedMaskImage);
      masker->Update();
      // get index of minimum and maximum
      typedef itk::MinimumMaximumImageCalculator< ImageType > MinMaxFilterType;
      typename MinMaxFilterType::Pointer minMaxFilter = MinMaxFilterType::New();
      minMaxFilter->SetImage( masker->GetOutput() );
      unsigned long observerTag2 = minMaxFilter->AddObserver( itk::ProgressEvent(), progressListener );
      minMaxFilter->Compute();
      minMaxFilter->RemoveObserver( observerTag2 );
      this->InvokeEvent( itk::EndEvent() );


        typename MinMaxFilterType::IndexType tempMaxIndex = minMaxFilter->GetIndexOfMaximum();
        typename MinMaxFilterType::IndexType tempMinIndex = minMaxFilter->GetIndexOfMinimum();

// FIX BUG 14644
        //If a PlanarFigure is used for segmentation the
        //adaptedImage is a single slice (2D). Adding the
        // 3. dimension.

        vnl_vector<int> maxIndex;
        vnl_vector<int> minIndex;
        maxIndex.set_size(m_Image->GetDimension());
        minIndex.set_size(m_Image->GetDimension());

        if (m_MaskingMode == MASKING_MODE_PLANARFIGURE && m_Image->GetDimension()==3)
        {
            maxIndex[m_PlanarFigureCoordinate0] = tempMaxIndex[0];
            maxIndex[m_PlanarFigureCoordinate1] = tempMaxIndex[1];
            maxIndex[m_PlanarFigureAxis] = m_PlanarFigureSlice;

            minIndex[m_PlanarFigureCoordinate0] = tempMinIndex[0] ;
            minIndex[m_PlanarFigureCoordinate1] = tempMinIndex[1];
            minIndex[m_PlanarFigureAxis] = m_PlanarFigureSlice;
        } else
        {
          for (unsigned int i = 0; i<maxIndex.size(); i++)
          {
            maxIndex[i] = tempMaxIndex[i];
            minIndex[i] = tempMinIndex[i];
          }
        }

        statistics.SetMaxIndex(maxIndex);
        statistics.SetMinIndex(minIndex);
     }


// FIX END

    // TODO DM: what about different label values? ImageStatisticsCalculator usually calculates statistics sets for EACH label in the given mask
    // TODO DM: it would be more consistent if we calculate hotspot statistics for EACH label, not only for the "unequal 0" label (after all other TODOs)
    /*****************************************************Calculate Hotspot Statistics**********************************************/
    if(IsHotspotCalculated())
    {
      // TODO DM: CalculateHotspotStatistics should
      //  1. regard mask
      //  2. calculate a hotspot (and its statistics) per mask label/value
      //  3. use LabelStatisticsImageFilter where possible
      Statistics hotspotStatistics = CalculateHotspotStatistics (adaptedImage.GetPointer(), adaptedMaskImage.GetPointer(), GetHotspotRadiusInMM()); // TODO: input label = *it
      statistics.SetHotspotMax( hotspotStatistics.GetHotspotMax() );
      statistics.SetHotspotMin( hotspotStatistics.GetHotspotMin() );
      statistics.SetHotspotMean( hotspotStatistics.GetHotspotMean() );
      statistics.SetHotspotMaxIndex( hotspotStatistics.GetHotspotMaxIndex() );
      statistics.SetHotspotMinIndex( hotspotStatistics.GetHotspotMinIndex() );
      statistics.SetHotspotIndex( hotspotStatistics.GetHotspotIndex() );
      statistics.SetHotspotMedian( hotspotStatistics.GetHotspotMedian() );
      statistics.SetHotspotN( hotspotStatistics.GetHotspotN() );
      statistics.SetHotspotRMS( hotspotStatistics.GetHotspotRMS() );
    }
    statisticsContainer->push_back( statistics );
  }
  else
  {
    histogramContainer->push_back( HistogramType::ConstPointer( m_EmptyHistogram ) );
    statisticsContainer->push_back( Statistics() ); // TODO DM: this is uninitialized! (refactor into real class!)
  }
}

// TODO DM: needs to be modified to calculate a specific or multiple(!) labels
template <typename TPixel, unsigned int VImageDimension  >
ImageStatisticsCalculator::ImageExtrema ImageStatisticsCalculator::CalculateExtremaWorld(
  const itk::Image<TPixel, VImageDimension> *inputImage,
  itk::Image<unsigned short, VImageDimension> *maskImage)
{
  // TODO iterate onlye the region of the mask (in the inputImage), which is usually smaller (not sure if this is possible)
  typedef itk::Image< TPixel, VImageDimension > ImageType;
  typedef itk::Image< unsigned short, VImageDimension > MaskImageType;

  typedef itk::ImageRegionConstIteratorWithIndex<MaskImageType> MaskImageIteratorType;
  typedef itk::ImageRegionConstIteratorWithIndex<ImageType> InputImageIndexIteratorType;

  MaskImageIteratorType maskIt(maskImage, inputImage->GetLargestPossibleRegion());
  InputImageIndexIteratorType imageIndexIt(inputImage, inputImage->GetLargestPossibleRegion());

  float maxValue = itk::NumericTraits<float>::min();
  float minValue = itk::NumericTraits<float>::max();

  typename ImageType::IndexType maxIndex;
  typename ImageType::IndexType minIndex;

  typename ImageType::IndexType imageIndex;
  typename ImageType::PointType worldPosition;
  typename ImageType::IndexType maskIndex;

  for(imageIndexIt.GoToBegin(); !imageIndexIt.IsAtEnd(); ++imageIndexIt)
  {
    imageIndex = imageIndexIt.GetIndex();
    inputImage->TransformIndexToPhysicalPoint(imageIndex, worldPosition);
    maskImage->TransformPhysicalPointToIndex(worldPosition, maskIndex);

    maskIt.SetIndex( maskIndex );
    if(maskIt.Get() > 0)
    {
      double value = imageIndexIt.Get();

      //Calculate minimum, maximum and corresponding index-values
      if( value > maxValue )
      {
        maxIndex = imageIndexIt.GetIndex();
        maxValue = value;
      }

      if(value < minValue )
      {
        minIndex = imageIndexIt.GetIndex();
        minValue = value;
      }
    }
  }

  ImageExtrema minMax;

  minMax.MinIndex.set_size(inputImage->GetImageDimension());
  minMax.MaxIndex.set_size(inputImage->GetImageDimension());

  for(unsigned int i = 0; i < minMax.MaxIndex.size(); ++i)
    minMax.MaxIndex[i] = maxIndex[i];

  for(unsigned int i = 0; i < minMax.MinIndex.size(); ++i)
    minMax.MinIndex[i] = minIndex[i];


  minMax.Max = maxValue;
  minMax.Min = minValue;

  return minMax;
}

template <unsigned int VImageDimension>
itk::Size<VImageDimension>
ImageStatisticsCalculator
::CalculateConvolutionKernelSize(double spacing[VImageDimension], double radiusInMM)
{
  typedef itk::Image< float, VImageDimension > KernelImageType;
  typedef typename KernelImageType::SizeType SizeType;
  SizeType maskSize;

  for(unsigned int i = 0; i < VImageDimension; ++i)
  {
    maskSize[i] = ::ceil( 2.0 * radiusInMM / spacing[i] );

    // We always want an uneven size to have a clear center point in the convolution mask
    if(maskSize[i] % 2 == 0 )
    {
      ++maskSize[i];
    }
  }

  return maskSize;
}

template <unsigned int VImageDimension>
itk::SmartPointer< itk::Image<float, VImageDimension> >
ImageStatisticsCalculator
::GenerateHotspotSearchConvolutionKernel(double spacing[VImageDimension], double radiusInMM)
{
  std::stringstream ss;
  for (unsigned int i = 0; i < VImageDimension; ++i)
  {
    ss << spacing[i];
    if (i < VImageDimension -1)
      ss << ",";
  }
  MITK_DEBUG << "Update convolution kernel for spacing (" << ss.str() << ") and radius " << radiusInMM << "mm";


  double radiusInMMSquared = radiusInMM * radiusInMM;
  typedef itk::Image< float, VImageDimension > KernelImageType;
  typename KernelImageType::Pointer convolutionKernel = KernelImageType::New();

  // Calculate size and allocate mask image
  typedef typename KernelImageType::IndexType IndexType;
  IndexType maskIndex;
  maskIndex.Fill(0);

  typedef typename KernelImageType::SizeType SizeType;
  SizeType maskSize = this->CalculateConvolutionKernelSize<VImageDimension>(spacing, radiusInMM);

  Point3D convolutionMaskCenter; convolutionMaskCenter.Fill(0.0);
  for(unsigned int i = 0; i < VImageDimension; ++i)
  {
    convolutionMaskCenter[i] = 0.5 * (double)(maskSize[i]-1);
  }

  typedef typename KernelImageType::RegionType RegionType;
  RegionType maskRegion;
  maskRegion.SetSize(maskSize);
  maskRegion.SetIndex(maskIndex);

  convolutionKernel->SetRegions(maskRegion);
  convolutionKernel->SetSpacing(spacing);
  convolutionKernel->Allocate();

  // Fill mask image values by subsampling the image grid
  typedef itk::ImageRegionIteratorWithIndex<KernelImageType> MaskIteratorType;
  MaskIteratorType maskIt(convolutionKernel,maskRegion);

  int numberOfSubVoxelsPerDimension = 2; // per dimension!
  int numberOfSubVoxels = ::pow( static_cast<float>(numberOfSubVoxelsPerDimension), static_cast<float>(VImageDimension) );
  double subVoxelSize = 1.0 / (double)numberOfSubVoxelsPerDimension;
  double valueOfOneSubVoxel = 1.0 / (double)numberOfSubVoxels;
  double maskValue = 0.0;
  Point3D subVoxelPosition;
  double distanceSquared = 0.0;

  typedef itk::ContinuousIndex<double, VImageDimension> ContinuousIndexType;
  for(maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt)
  {
    ContinuousIndexType indexPoint(maskIt.GetIndex());
    Point3D voxelPosition;
    for (unsigned int dimension = 0; dimension < VImageDimension; ++dimension)
    {
      voxelPosition[dimension] = indexPoint[dimension];
    }

    // TODO think about 2D case
    // TODO this could be done by calling a recursive method, handing over the "remaining number of dimensions to iterate"
    maskValue = 0.0;
    Vector3D subVoxelOffset; subVoxelOffset.Fill(0.0);
    // iterate sub-voxels by iterating all possible offsets
    for (subVoxelOffset[0] = -0.5 + subVoxelSize / 2.0;
        subVoxelOffset[0] < +0.5;
        subVoxelOffset[0] += subVoxelSize)
    {
      for (subVoxelOffset[1] = -0.5 + subVoxelSize / 2.0;
          subVoxelOffset[1] < +0.5;
          subVoxelOffset[1] += subVoxelSize)
      {
        for (subVoxelOffset[2] = -0.5 + subVoxelSize / 2.0;
            subVoxelOffset[2] < +0.5;
            subVoxelOffset[2] += subVoxelSize)
        {
          subVoxelPosition = voxelPosition + subVoxelOffset; // this COULD be integrated into the for-loops if neccessary (add voxelPosition to initializer and end condition)
         distanceSquared =
              (subVoxelPosition[0]-convolutionMaskCenter[0]) / spacing[0] * (subVoxelPosition[0]-convolutionMaskCenter[0]) / spacing[0]
            + (subVoxelPosition[1]-convolutionMaskCenter[1]) / spacing[1] * (subVoxelPosition[1]-convolutionMaskCenter[1]) / spacing[1]
            + (subVoxelPosition[2]-convolutionMaskCenter[2]) / spacing[2] * (subVoxelPosition[2]-convolutionMaskCenter[2]) / spacing[2];

          if (distanceSquared <= radiusInMMSquared)
          {
            maskValue += valueOfOneSubVoxel;
          }
        }
      }
    }
    maskIt.Set( maskValue );
  }
  return convolutionKernel;
}

template <typename TPixel, unsigned int VImageDimension>
void
ImageStatisticsCalculator::InternalUpdateConvolutionImage( itk::Image<TPixel, VImageDimension>* inputImage )
{
  double spacing[VImageDimension];
  for (unsigned int dimension = 0; dimension < VImageDimension; ++dimension)
  {
    spacing[dimension] = inputImage->GetSpacing()[dimension];
  }

  // update convolution kernel
  typedef itk::Image< float, VImageDimension > KernelImageType;
  typename KernelImageType::Pointer convolutionKernel = this->GenerateHotspotSearchConvolutionKernel<VImageDimension>(spacing, m_HotspotRadiusInMM);
  // TODO: if GenerateHotspotSearchConvolutionKernel() consumes relevant time, we need an additional condition

  // update convolution image
  typedef itk::Image< TPixel, VImageDimension > InputImageType;
  typedef itk::Image< TPixel, VImageDimension > ConvolutionImageType;
  typedef itk::FFTConvolutionImageFilter<InputImageType,
                                         KernelImageType,
                                         ConvolutionImageType> ConvolutionFilterType;

  typedef itk::ConstantBoundaryCondition<InputImageType, InputImageType> BoundaryConditionType;
  BoundaryConditionType boundaryCondition;
  boundaryCondition.SetConstant(0.0);

  typename ConvolutionFilterType::Pointer convolutionFilter = ConvolutionFilterType::New();
  convolutionFilter->SetBoundaryCondition(&boundaryCondition);
  convolutionFilter->SetInput(inputImage);
  convolutionFilter->SetKernelImage(convolutionKernel);
  convolutionFilter->SetNormalize(true);
  //MITK_INFO << "Update Convolution image for hotspot search";
  convolutionFilter->UpdateLargestPossibleRegion(); // TODO check if we could benefit from restricting this for a region

  typename ConvolutionImageType::Pointer convolutionImage = convolutionFilter->GetOutput();
  convolutionImage->SetSpacing( inputImage->GetSpacing() ); // TODO: only workaround because convolution filter seems to ignore spacing of input image

  m_HotspotSearchConvolutionImage = convolutionImage.GetPointer();

  m_HotspotRadiusInMMChanged = false;
}

template < typename TPixel, unsigned int VImageDimension>
void
ImageStatisticsCalculator
::FillHotspotMaskPixels( itk::Image<TPixel, VImageDimension>* maskImage,
                         itk::Point<double, VImageDimension> sphereCenter,
                         double sphereRadiusInMM)
{
  typedef itk::Image< TPixel, VImageDimension > MaskImageType;
  typedef itk::ImageRegionIteratorWithIndex<MaskImageType> MaskImageIteratorType;

  MaskImageIteratorType maskIt(maskImage, maskImage->GetLargestPossibleRegion()); // TODO DM: we should use the same regions here

  typename MaskImageType::IndexType maskIndex;
  typename MaskImageType::PointType worldPosition;

  for(maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt)
  {
    maskIndex = maskIt.GetIndex();
    maskImage->TransformIndexToPhysicalPoint(maskIndex, worldPosition);

    maskIt.Set( worldPosition.EuclideanDistanceTo(sphereCenter) <= sphereRadiusInMM ? 1 : 0 );
  }
}

template < typename TPixel, unsigned int VImageDimension>
ImageStatisticsCalculator::Statistics ImageStatisticsCalculator::CalculateHotspotStatistics(
    const itk::Image<TPixel, VImageDimension>* inputImage,
    itk::Image<unsigned short, VImageDimension>* maskImage,
    double radiusInMM)
{
  //MITK_INFO << "CalculateHotspotStatistics()";

  // get convolution image (updated in InternalUpdateConvolutionImage())
  typedef itk::Image< TPixel, VImageDimension > ConvolutionImageType;
  typedef itk::Image< float, VImageDimension > KernelImageType;
  typedef itk::Image< unsigned short, VImageDimension > MaskImageType;
  typename ConvolutionImageType::Pointer convolutionImage = dynamic_cast<ConvolutionImageType*>(m_HotspotSearchConvolutionImage.GetPointer());
  if (convolutionImage.IsNull())
  {
    MITK_ERROR << "Empty convolution image in CalculateHotspotStatistics(). We should never reach this state (logic error).";
    throw std::logic_error("Empty convolution image in CalculateHotspotStatistics()");
  }

  // find maximum in convolution image, given the current mask (this might change over time, while we assume the input fixed (TODO wrong assumption))
  ImageExtrema pi = CalculateExtremaWorld(convolutionImage.GetPointer(), maskImage);
 /* MITK_INFO <<"Initial search for hotspot: "
  "\n  Index: " << pi.MaxIndex[0] << "," << pi.MaxIndex[1] << "," << pi.MaxIndex[2] <<
    "\n  Value(hotspot): " << pi.Max<<
  "\n  Index(min): " << pi.MinIndex[0] << "," << pi.MinIndex[1] << "," << pi.MinIndex[2] <<
    "\n  Value(min): " << pi.Min;*/

  // create mask corresponding to hotspot region
  // mask is defined on the inputImage grid and is
  // dimensioned as the hotspot convolution kernel (the sphere)
  double spacing[VImageDimension];
  for (unsigned int dimension = 0; dimension < VImageDimension; ++dimension)
  {
    spacing[dimension] = inputImage->GetSpacing()[dimension];
  }

  typedef typename ConvolutionImageType::SizeType SizeType;
  SizeType maskSize = this->CalculateConvolutionKernelSize<VImageDimension>(spacing, m_HotspotRadiusInMM);

  typedef typename ConvolutionImageType::IndexType IndexType;
  IndexType maskIndex; maskIndex.Fill(0);
  MITK_DEBUG << "Hotspot statistics mask started with size ["<<maskSize[0]<<"x"<<maskSize[1]<<"x"<<maskSize[2]<<"] at ["<<maskIndex[0]<<","<<maskIndex[1]<<","<<maskIndex[2]<<"]";
  for (unsigned int dimension = 0; dimension < VImageDimension; ++dimension)
  {
    maskIndex[dimension] = pi.MaxIndex[dimension] - (maskSize[dimension]-1)/2; // maskSize is always odd (size of 5 --> shift -2 required
    if (maskIndex[dimension] < 0)
    {
      maskIndex[dimension] = 0;
    }

    if (maskIndex[dimension] + maskSize[dimension] > inputImage->GetRequestedRegion().GetSize()[dimension] )
    {
      maskSize[dimension] = inputImage->GetRequestedRegion().GetSize()[dimension] - maskIndex[dimension];
    }
  }

  MITK_DEBUG << "Hotspot statistics mask corrected as region of size ["<<maskSize[0]<<"x"<<maskSize[1]<<"x"<<maskSize[2]<<"] at ["<<maskIndex[0]<<","<<maskIndex[1]<<","<<maskIndex[2]<<"]";

  typename ConvolutionImageType::Pointer hotspotMaskITK = ConvolutionImageType::New();
  // copy origin and spacing of maskImage
  hotspotMaskITK->CopyInformation( inputImage ); // type not optimal, but image grid is good

  typedef typename ConvolutionImageType::RegionType RegionType;
  RegionType hotspotMaskRegion;
  IndexType mi; mi.Fill(0);
  hotspotMaskRegion.SetIndex( mi );
  hotspotMaskRegion.SetSize( maskSize );

  hotspotMaskITK->SetRegions( hotspotMaskRegion );
  hotspotMaskITK->Allocate();

  typename ConvolutionImageType::PointType maskOrigin;
  inputImage->TransformIndexToPhysicalPoint(maskIndex,maskOrigin);
  //MITK_INFO << "Mask origin at: " << maskOrigin;
  hotspotMaskITK->SetOrigin(maskOrigin);

  IndexType maskCenterIndex;
  for (unsigned int d =0; d< VImageDimension;++d) maskCenterIndex[d]=pi.MaxIndex[d];
  typename ConvolutionImageType::PointType maskCenter;
  inputImage->TransformIndexToPhysicalPoint(maskCenterIndex,maskCenter);
  //MITK_INFO << "Mask center in input image: " << maskCenter;

  this->FillHotspotMaskPixels(hotspotMaskITK.GetPointer(), maskCenter, m_HotspotRadiusInMM);

  Image::Pointer hotspotMaskMITK = ImportItkImage( hotspotMaskITK );

  Point3D maskCenterPosition = hotspotMaskMITK->GetGeometry()->GetCenter();
  //MITK_INFO << "Mask center: " << maskCenterPosition;

  // use second instance of ImageStatisticsCalculator to calculate hotspot statistics

  Image::Pointer hotspotInputMITK = ImportItkImage( inputImage );

  ImageStatisticsCalculator::Pointer calculator = ImageStatisticsCalculator::New();
  calculator->SetImage( hotspotInputMITK );
  calculator->SetMaskingModeToImage();
  calculator->SetImageMask( hotspotMaskMITK );
  calculator->SetCalculateHotspot( false );
  calculator->ComputeStatistics(0); // timestep 0, because inputImage already IS the image of timestep N (from perspective of ImageStatisticsCalculator caller)

  Statistics hotspotMaskStatistics = calculator->GetStatistics(0);

  Statistics hotspotStatistics;
  hotspotStatistics.SetHotspotMin(hotspotMaskStatistics.GetMin());
  hotspotStatistics.SetHotspotMinIndex(hotspotMaskStatistics.GetMinIndex());
  hotspotStatistics.SetHotspotMax(hotspotMaskStatistics.GetMax());
  hotspotStatistics.SetHotspotMaxIndex(hotspotMaskStatistics.GetMaxIndex());
  hotspotStatistics.SetHotspotMean(hotspotMaskStatistics.GetMean());
  hotspotStatistics.SetHotspotMedian(hotspotMaskStatistics.GetMedian());
  hotspotStatistics.SetHotspotIndex(pi.MaxIndex);
  hotspotStatistics.SetHotspotSigma(hotspotMaskStatistics.GetSigma());
  hotspotStatistics.SetHotspotRMS(sqrt( hotspotMaskStatistics.GetMean() * hotspotMaskStatistics.GetMean()
    + hotspotMaskStatistics.GetSigma() * hotspotMaskStatistics.GetSigma() ));
  hotspotStatistics.SetHotspotN(hotspotMaskStatistics.GetN());

  /*MITK_INFO << "----- Hotspot search results:"
               "\n  Index: " << hotspotStatistics.HotspotIndex[0] << "," << hotspotStatistics.HotspotIndex[1] << "," << hotspotStatistics.HotspotIndex[2] <<
               "\n  Value: " << hotspotStatistics.HotspotMean <<
               "\n  Max Index: " << hotspotStatistics.HotspotMaxIndex[0] << "," << hotspotStatistics.HotspotMaxIndex[1] << "," << hotspotStatistics.HotspotMaxIndex[2] <<
               "\n  Max Value: " << hotspotStatistics.HotspotMax <<
               "\n  Min Index: " << hotspotStatistics.HotspotMinIndex[0] << "," << hotspotStatistics.HotspotMinIndex[1] << "," << hotspotStatistics.HotspotMinIndex[2] <<
               "\n  Min Value: " << hotspotStatistics.HotspotMin;*/

  return hotspotStatistics;
}

template < typename TPixel, unsigned int VImageDimension >
void ImageStatisticsCalculator::InternalCalculateMaskFromPlanarFigure(
  const itk::Image< TPixel, VImageDimension > *image, unsigned int axis )
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;

  typedef itk::CastImageFilter< ImageType, MaskImage2DType > CastFilterType;

  // Generate mask image as new image with same header as input image and
  // initialize with 1.
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
  m_PlanarFigureCoordinate0= i0;
  m_PlanarFigureCoordinate1= i1;

  // store the polyline contour as vtkPoints object
  bool outOfBounds = false;
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
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
  vtkSmartPointer<vtkLassoStencilSource> lassoStencil = vtkSmartPointer<vtkLassoStencilSource>::New();
  lassoStencil->SetShapeToPolygon();
  lassoStencil->SetPoints( points );

  // Export from ITK to VTK (to use a VTK filter)
  typedef itk::VTKImageImport< MaskImage2DType > ImageImportType;
  typedef itk::VTKImageExport< MaskImage2DType > ImageExportType;

  typename ImageExportType::Pointer itkExporter = ImageExportType::New();
  itkExporter->SetInput( castFilter->GetOutput() );

  vtkSmartPointer<vtkImageImport> vtkImporter = vtkSmartPointer<vtkImageImport>::New();
  this->ConnectPipelines( itkExporter, vtkImporter );

  // Apply the generated image stencil to the input image
  vtkSmartPointer<vtkImageStencil> imageStencilFilter = vtkSmartPointer<vtkImageStencil>::New();
  imageStencilFilter->SetInputConnection( vtkImporter->GetOutputPort() );
  imageStencilFilter->SetStencil( lassoStencil->GetOutput() );
  imageStencilFilter->ReverseStencilOff();
  imageStencilFilter->SetBackgroundValue( 0 );
  imageStencilFilter->Update();

  // Export from VTK back to ITK
  vtkSmartPointer<vtkImageExport> vtkExporter = vtkImageExport::New(); // TODO: this is WRONG, should be vtkSmartPointer<vtkImageExport>::New(), but bug # 14455
  vtkExporter->SetInputConnection( imageStencilFilter->GetOutputPort() );
  vtkExporter->Update();

  typename ImageImportType::Pointer itkImporter = ImageImportType::New();
  this->ConnectPipelines( vtkExporter, itkImporter );
  itkImporter->Update();

  // Store mask
  m_InternalImageMask2D = itkImporter->GetOutput();
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

// Get-functions statistics
unsigned int ImageStatisticsCalculator::Statistics::GetN() const { return m_N; }
double ImageStatisticsCalculator::Statistics::GetMean() const { return m_Mean; }
double ImageStatisticsCalculator::Statistics::GetMin() const { return m_Min; }
double ImageStatisticsCalculator::Statistics::GetMax() const { return m_Max; }
double ImageStatisticsCalculator::Statistics::GetMedian() const { return m_Median; }
double ImageStatisticsCalculator::Statistics::GetVariance() const { return m_Variance; }
double ImageStatisticsCalculator::Statistics::GetSigma() const { return m_Sigma; }
double ImageStatisticsCalculator::Statistics::GetRMS() const { return m_RMS; }
vnl_vector<int> ImageStatisticsCalculator::Statistics::GetMaxIndex() const { return m_MaxIndex; }
vnl_vector<int> ImageStatisticsCalculator::Statistics::GetMinIndex() const { return m_MinIndex; }

// Set-fucntions statistics
void ImageStatisticsCalculator::Statistics::SetLabel(unsigned int label) { m_Label = label; }
void ImageStatisticsCalculator::Statistics::SetN(unsigned int n) {  m_N = n; }
void ImageStatisticsCalculator::Statistics::SetMean(double mean) {  m_Mean = mean; }
void ImageStatisticsCalculator::Statistics::SetMin(double min) {  m_Min = min; }
void ImageStatisticsCalculator::Statistics::SetMax(double max) {  m_Max = max; }
void ImageStatisticsCalculator::Statistics::SetMedian(double median) {  m_Median = median; }
void ImageStatisticsCalculator::Statistics::SetVariance(double variance) {  m_Variance = variance; }
void ImageStatisticsCalculator::Statistics::SetSigma(double sigma) {  m_Sigma = sigma; }
void ImageStatisticsCalculator::Statistics::SetRMS(double rms) {  m_RMS = rms; }
void ImageStatisticsCalculator::Statistics::SetMaxIndex(vnl_vector<int> maxIndex) { m_MaxIndex = maxIndex; }
void ImageStatisticsCalculator::Statistics::SetMinIndex(vnl_vector<int> minIndex) { m_MinIndex = minIndex; }

// Get-fucntions hotspot-statistics
unsigned int ImageStatisticsCalculator::Statistics::GetHotspotN() const { return m_HotspotN; }
double ImageStatisticsCalculator::Statistics::GetHotspotMean() const { return m_HotspotMean; }
double ImageStatisticsCalculator::Statistics::GetHotspotMin() const { return m_HotspotMin; }
double ImageStatisticsCalculator::Statistics::GetHotspotMax() const { return m_HotspotMax; }
double ImageStatisticsCalculator::Statistics::GetHotspotMedian() const { return m_HotspotMedian; }
double ImageStatisticsCalculator::Statistics::GetHotspotVariance() const { return m_HotspotVariance; }
double ImageStatisticsCalculator::Statistics::GetHotspotSigma() const { return m_HotspotSigma; }
double ImageStatisticsCalculator::Statistics::GetHotspotRMS() const { return m_HotspotRMS; }
vnl_vector<int> ImageStatisticsCalculator::Statistics::GetHotspotMaxIndex() const { return m_HotspotMaxIndex; }
vnl_vector<int> ImageStatisticsCalculator::Statistics::GetHotspotMinIndex() const { return m_HotspotMinIndex; }
vnl_vector<int> ImageStatisticsCalculator::Statistics::GetHotspotIndex() const {return m_HotspotIndex;}

// Set-functions hotspot-statistics
void ImageStatisticsCalculator::Statistics::SetHotspotN(unsigned int n) {  m_HotspotN = n; }
void ImageStatisticsCalculator::Statistics::SetHotspotMean(double mean) {  m_HotspotMean = mean; }
void ImageStatisticsCalculator::Statistics::SetHotspotMin(double min) {  m_HotspotMin = min; }
void ImageStatisticsCalculator::Statistics::SetHotspotMax(double max) {  m_HotspotMax = max; }
void ImageStatisticsCalculator::Statistics::SetHotspotMedian(double median) {  m_HotspotMedian = median; }
void ImageStatisticsCalculator::Statistics::SetHotspotVariance(double variance) {  m_HotspotVariance = variance; }
void ImageStatisticsCalculator::Statistics::SetHotspotSigma(double sigma) {  m_HotspotSigma = sigma; }
void ImageStatisticsCalculator::Statistics::SetHotspotRMS(double rms) {  m_HotspotRMS = rms; }
void ImageStatisticsCalculator::Statistics::SetHotspotMaxIndex(vnl_vector<int> maxIndex) { m_HotspotMaxIndex = maxIndex; }
void ImageStatisticsCalculator::Statistics::SetHotspotMinIndex(vnl_vector<int> minIndex) { m_HotspotMinIndex = minIndex; }
void ImageStatisticsCalculator::Statistics::SetHotspotIndex(vnl_vector<int> index) { m_HotspotIndex = index; }

}
