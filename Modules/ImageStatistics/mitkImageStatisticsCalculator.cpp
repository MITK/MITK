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

#include <mitkExtendedStatisticsImageFilter.h>
#include <mitkExtendedLabelStatisticsImageFilter.h>

#include <itkScalarImageToHistogramGenerator.h>

#include <itkChangeInformationImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkStatisticsImageFilter.h>
#include <itkLabelStatisticsImageFilter.h>
#include <itkMaskImageFilter.h>
#include <itkImageFileWriter.h>
#include <itkRescaleIntensityImageFilter.h>

#include <itkMaskImageFilter.h>

#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>

#include <itkCastImageFilter.h>
#include <itkVTKImageImport.h>
#include <itkVTKImageExport.h>
#include <itkImageDuplicator.h>

#include <vtkPoints.h>
#include <vtkImageStencil.h>
#include <vtkImageImport.h>
#include <vtkImageExport.h>
#include <vtkLassoStencilSource.h>

#include <itkFFTConvolutionImageFilter.h>
#include <itkConstantBoundaryCondition.h>
#include <itkImageDuplicator.h>

#include <itkContinuousIndex.h>
#include <itkNumericTraits.h>
#include <list>

#include <exception>

#include "itkImage.h"

//#define DEBUG_HOTSPOTSEARCH

#define _USE_MATH_DEFINES
#include <math.h>

#include "vtkLassoStencilSource.h"


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
    m_PlanarFigureCoordinate1 (0),
    m_HistogramBinSize(1.0),
    m_UseDefaultBinSize(true),
    m_UseBinSizeBasedOnVOIRegion(false),
    m_HotspotRadiusInMM(6.2035049089940),   // radius of a 1cm3 sphere in mm
    m_CalculateHotspot(false),
    m_HotspotRadiusInMMChanged(false),
    m_HotspotMustBeCompletelyInsideImage(true)
  {
    m_EmptyHistogram = HistogramType::New();
    m_EmptyHistogram->SetMeasurementVectorSize(1);
    HistogramType::SizeType histogramSize(1);
    histogramSize.Fill( 256 );
    m_EmptyHistogram->Initialize( histogramSize );

    m_EmptyStatistics.Reset();
  }

  ImageStatisticsCalculator::~ImageStatisticsCalculator()
  {
  }


  void ImageStatisticsCalculator::SetUseDefaultBinSize(bool useDefault)
  {
    m_UseDefaultBinSize = useDefault;
  }




  ImageStatisticsCalculator::Statistics::Statistics(bool withHotspotStatistics)
    :m_HotspotStatistics(withHotspotStatistics ? new Statistics(false) : nullptr)
  {
    Reset();
  }

  ImageStatisticsCalculator::Statistics::Statistics(const Statistics& other)
    :m_HotspotStatistics( nullptr)
  {
    this->SetLabel( other.GetLabel() );
    this->SetN( other.GetN() );
    this->SetMin( other.GetMin() );
    this->SetMax( other.GetMax() );
    this->SetMedian( other.GetMedian() );
    this->SetMean( other.GetMean() );
    this->SetVariance( other.GetVariance() );
    this->SetKurtosis( other.GetKurtosis() );
    this->SetSkewness( other.GetSkewness() );
    this->SetUniformity( other.GetUniformity() );
    this->SetEntropy( other.GetEntropy() );
    this->SetUPP( other.GetUPP() );
    this->SetMPP( other.GetMPP() );
    this->SetSigma( other.GetSigma() );
    this->SetRMS( other.GetRMS() );
    this->SetMaxIndex( other.GetMaxIndex() );
    this->SetMinIndex( other.GetMinIndex() );
    this->SetHotspotIndex( other.GetHotspotIndex() );

    if (other.m_HotspotStatistics)
    {
      this->m_HotspotStatistics = new Statistics(false);
      *this->m_HotspotStatistics = *other.m_HotspotStatistics;
    }
  }

  bool ImageStatisticsCalculator::Statistics::HasHotspotStatistics() const
  {
    return m_HotspotStatistics != nullptr;
  }

  void ImageStatisticsCalculator::Statistics::SetHasHotspotStatistics(bool hasHotspotStatistics)
  {
    m_HasHotspotStatistics = hasHotspotStatistics;
  }


  ImageStatisticsCalculator::Statistics::~Statistics()
  {
    delete m_HotspotStatistics;
  }

  double ImageStatisticsCalculator::Statistics::GetVariance() const
  {
    return this->Variance;
  }

  void ImageStatisticsCalculator::Statistics::SetVariance( const double value )
  {
    if( this->Variance != value )
    {
      if( value < 0.0 )
      {
        this->Variance = 0.0; // if given value is negative set variance to 0.0
      }
      else
      {
        this->Variance = value;
      }
    }
  }

  double ImageStatisticsCalculator::Statistics::GetSigma() const
  {
    return this->Sigma;
  }

  void ImageStatisticsCalculator::Statistics::SetSigma( const double value )
  {
    if( this->Sigma != value )
    {
      // for some compiler the value != value works to check for NaN but not for all
      // but we can always be sure that the standard deviation is a positive value
      if( value != value || value < 0.0 )
      {
        // if standard deviation is NaN we just assume 0.0
        this->Sigma = 0.0;
      }
      else
      {
        this->Sigma = value;
      }
    }
  }

  void ImageStatisticsCalculator::Statistics::Reset(unsigned int dimension)
  {
    SetLabel(0);
    SetN( 0 );
    SetMin( 0.0 );
    SetMax( 0.0 );
    SetMedian( 0.0 );
    SetVariance( 0.0 );
    SetMean( 0.0 );
    SetSigma( 0.0 );
    SetRMS( 0.0 );

    SetSkewness( 0.0 );
    SetKurtosis( 0.0 );
    SetUniformity( 0.0 );
    SetEntropy( 0.0 );
    SetMPP( 0.0 );
    SetUPP( 0.0 );

    vnl_vector<int> zero;
    zero.set_size(dimension);
    for(unsigned int i = 0; i < dimension; ++i)
    {
      zero[i] = 0;
    }

    SetMaxIndex(zero);
    SetMinIndex(zero);
    SetHotspotIndex(zero);

    if (m_HotspotStatistics != nullptr)
    {
      m_HotspotStatistics->Reset(dimension);
    }
  }

  const ImageStatisticsCalculator::Statistics&
    ImageStatisticsCalculator::Statistics::GetHotspotStatistics() const
  {
    if (m_HotspotStatistics)
    {
      return *m_HotspotStatistics;
    }
    else
    {
      throw std::logic_error("Object has no hostspot statistics, see HasHotspotStatistics()");
    }
  }

  ImageStatisticsCalculator::Statistics&
    ImageStatisticsCalculator::Statistics::GetHotspotStatistics()
  {
    if (m_HotspotStatistics)
    {
      return *m_HotspotStatistics;
    }
    else
    {
      throw std::logic_error("Object has no hostspot statistics, see HasHotspotStatistics()");
    }
  }

  ImageStatisticsCalculator::Statistics&
    ImageStatisticsCalculator::Statistics::operator=(ImageStatisticsCalculator::Statistics const& other)
  {
    if (this == &other)
      return *this;

    this->SetLabel( other.GetLabel() );
    this->SetN( other.GetN() );
    this->SetMin( other.GetMin() );
    this->SetMax( other.GetMax() );
    this->SetMean( other.GetMean() );
    this->SetMedian( other.GetMedian() );
    this->SetVariance( other.GetVariance() );
    this->SetSigma( other.GetSigma() );
    this->SetRMS( other.GetRMS() );
    this->SetMinIndex( other.GetMinIndex() );
    this->SetMaxIndex( other.GetMaxIndex() );
    this->SetHotspotIndex( other.GetHotspotIndex() );
    this->SetSkewness( other.GetSkewness() );
    this->SetKurtosis( other.GetKurtosis() );
    this->SetUniformity( other.GetUniformity() );
    this->SetEntropy( other.GetEntropy() );
    this->SetUPP( other.GetUPP() );
    this->SetMPP( other.GetMPP() );


    delete this->m_HotspotStatistics;
    this->m_HotspotStatistics = nullptr;

    if (other.m_HotspotStatistics)
    {
      this->m_HotspotStatistics = new Statistics(false);
      *this->m_HotspotStatistics = *other.m_HotspotStatistics;
    }

    return *this;

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

  void ImageStatisticsCalculator::SetHistogramBinSize(double size)
  {
    this->m_HistogramBinSize = size;
  }
  double ImageStatisticsCalculator::GetHistogramBinSize()
  {
    return this->m_HistogramBinSize;
  }

  void ImageStatisticsCalculator::SetHotspotRadiusInMM(double value)
  {
    if ( m_HotspotRadiusInMM != value )
    {
      m_HotspotRadiusInMM = value;
      if(m_CalculateHotspot)
      {
        m_HotspotRadiusInMMChanged = true;
        //MITK_INFO <<"Hotspot radius changed, new convolution required";
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
      //MITK_INFO <<"Hotspot calculation changed, new convolution required";
      this->Modified();
    }
  }

  bool ImageStatisticsCalculator::IsHotspotCalculated()
  {
    return m_CalculateHotspot;
  }

  void ImageStatisticsCalculator::SetHotspotMustBeCompletlyInsideImage(bool hotspotMustBeCompletelyInsideImage, bool warn)
  {
    m_HotspotMustBeCompletelyInsideImage = hotspotMustBeCompletelyInsideImage;
    if (!m_HotspotMustBeCompletelyInsideImage && warn)
    {
      MITK_WARN << "Hotspot calculation will extrapolate pixels at image borders. Be aware of the consequences for the hotspot location.";
    }
  }

  bool ImageStatisticsCalculator::GetHotspotMustBeCompletlyInsideImage() const
  {
    return m_HotspotMustBeCompletelyInsideImage;
  }


  /* Implementation of the min max values for setting the range of the histogram */
  template < typename TPixel, unsigned int VImageDimension >
  void  ImageStatisticsCalculator::GetMinAndMaxValue( double &min,
    double &max, int &counter, double &sigma,
    const itk::Image< TPixel, VImageDimension > *InputImage,
    itk::Image< unsigned short, VImageDimension > *MaskedImage )
  {
    typedef itk::Image< unsigned short, VImageDimension > MaskImageType;
    typedef itk::Image< TPixel, VImageDimension > ImageType;

    typedef itk::ImageRegionConstIteratorWithIndex<ImageType> Imageie;
    typedef itk::ImageRegionConstIteratorWithIndex<MaskImageType> Imageie2;

    Imageie2 labelIterator2( MaskedImage, MaskedImage->GetRequestedRegion() );
    Imageie labelIterator3( InputImage, InputImage->GetRequestedRegion() );

    max = 0;
    min = 0;
    counter = 0;
    sigma = 0;
    double SumOfSquares = 0;
    double sumSquared = 0;

    double actualPielValue = 0;
    int counterOfPixelsInROI = 0;


    for( labelIterator2.GoToBegin(); !labelIterator2.IsAtEnd(); ++labelIterator2, ++labelIterator3)
    {
      if( labelIterator2.Value()== 1.0)
      {
        counter++;

        counterOfPixelsInROI++;
        actualPielValue = labelIterator3.Value();

        sumSquared = sumSquared + actualPielValue;
        SumOfSquares = SumOfSquares + std::pow(actualPielValue,2);

        if(counterOfPixelsInROI == 1)
        {
          max = actualPielValue;
          min = actualPielValue;
        }

        if(actualPielValue >= max)
        {
          max = actualPielValue;
        }
        else if(actualPielValue <= min)
        {
          min = actualPielValue;
        }

      }
    }

    if (counter > 1)
    {
      sigma = ( SumOfSquares - std::pow( sumSquared, 2) / counter ) / ( counter-1 );
    }
    else
    {
      sigma = 0;
    }

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
      m_ImageMask = nullptr;
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


  ImageStatisticsCalculator::BinFrequencyType
    ImageStatisticsCalculator::GetBinsAndFreuqencyForHistograms( unsigned int timeStep , unsigned int label ) const
  {
    const HistogramType *binsAndFrequencyToCalculate = this->GetHistogram(0);

    // ToDo: map should be created on stack not on heap
    std::map<int, double> returnedHistogramMap;

    unsigned int size = binsAndFrequencyToCalculate->Size();
    for( unsigned int bin=0; bin < size; ++bin )
    {
      double frequency = binsAndFrequencyToCalculate->GetFrequency( bin, 0 );
      //if( frequency > mitk::eps )
      {
        returnedHistogramMap.insert( std::pair<int, double>(binsAndFrequencyToCalculate->GetMeasurement( bin, 0 ), binsAndFrequencyToCalculate->GetFrequency( bin, 0 ) ) );
      }
    }

    return returnedHistogramMap;
  }

  const ImageStatisticsCalculator::HistogramType *
    ImageStatisticsCalculator::GetHistogram( unsigned int timeStep, unsigned int label ) const
  {
    if ( m_Image.IsNull() || (timeStep >= m_Image->GetTimeSteps()) )
    {
      return nullptr;
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
        m_InternalImageMask2D = nullptr;
        m_InternalImageMask3D = nullptr;
        if(m_DoIgnorePixelValue)
        {
          if( m_InternalImage->GetDimension() == 3 )
          {
            if(itk::ImageIOBase::USHORT != timeSliceImage->GetPixelType().GetComponentType())
              CastToItkImage( timeSliceImage, m_InternalImageMask3D );
            else
              CastToItkImage( timeSliceImage->Clone(), m_InternalImageMask3D  );
            m_InternalImageMask3D->FillBuffer(1);
          }
          if( m_InternalImage->GetDimension() == 2 )
          {
            if(itk::ImageIOBase::USHORT != timeSliceImage->GetPixelType().GetComponentType())
              CastToItkImage( timeSliceImage, m_InternalImageMask2D );
            else
              CastToItkImage( timeSliceImage->Clone(), m_InternalImageMask2D );
            m_InternalImageMask2D->FillBuffer(1);
          }
        }
        break;
      }

    case MASKING_MODE_IMAGE:
      {
        if ( m_ImageMask.IsNotNull() && (m_ImageMask->GetReferenceCount() > 1) )
        {
          if ( timeStep >= m_ImageMask->GetTimeSteps() )
          {
            // Use the last mask time step in case the current time step is bigger than the total
            // number of mask time steps.
            // It makes more sense setting this to the last mask time step than to 0.
            // For instance if you have a mask with 2 time steps and an image with 5:
            // If time step 0 is selected, the mask will use time step 0.
            // If time step 1 is selected, the mask will use time step 1.
            // If time step 2+ is selected, the mask will use time step 1.
            // If you have a mask with only one time step instead, this will always default to 0.
            timeStep = m_ImageMask->GetTimeSteps() - 1;
          }

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
          throw std::runtime_error( "Error: image mask empty!" );
        }
        break;
      }
    case MASKING_MODE_PLANARFIGURE:
      {
        m_InternalImageMask2D = nullptr;

        if ( m_PlanarFigure.IsNull() )
        {
          throw std::runtime_error( "Error: planar figure empty!" );
        }
        if ( !m_PlanarFigure->IsClosed() )
        {
          throw std::runtime_error( "Masking not possible for non-closed figures" );
        }

        const BaseGeometry *imageGeometry = timeSliceImage->GetGeometry();
        if ( imageGeometry == nullptr )
        {
          throw std::runtime_error( "Image geometry invalid!" );
        }

        const PlaneGeometry *planarFigurePlaneGeometry = m_PlanarFigure->GetPlaneGeometry();
        if ( planarFigurePlaneGeometry == nullptr )
        {
          throw std::runtime_error( "Planar-Figure not yet initialized!" );
        }

        const PlaneGeometry *planarFigureGeometry =
          dynamic_cast< const PlaneGeometry * >( planarFigurePlaneGeometry );
        if ( planarFigureGeometry == nullptr )
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
  }


  bool ImageStatisticsCalculator::GetPrincipalAxis(
    const BaseGeometry *geometry, Vector3D vector,
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


  unsigned int ImageStatisticsCalculator::calcNumberOfBins(mitk::ScalarType min, mitk::ScalarType max)
  {
    return std::ceil( ( (max - min ) / m_HistogramBinSize) );
  }


  template < typename TPixel, unsigned int VImageDimension >
  void ImageStatisticsCalculator::InternalCalculateStatisticsUnmasked(
    const itk::Image< TPixel, VImageDimension > *image,
    StatisticsContainer *statisticsContainer,
    HistogramContainer* histogramContainer )
  {
    typedef itk::Image< TPixel, VImageDimension > ImageType;
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
    typedef itk::ExtendedStatisticsImageFilter< ImageType > StatisticsFilterType;
    typename StatisticsFilterType::Pointer statisticsFilter = StatisticsFilterType::New();
    statisticsFilter->SetInput( image );
    statisticsFilter->SetBinSize( 100 );
    statisticsFilter->SetCoordinateTolerance( 0.001 );
    statisticsFilter->SetDirectionTolerance( 0.001 );

    unsigned long observerTag = statisticsFilter->AddObserver( itk::ProgressEvent(), progressListener );
    try
    {
      statisticsFilter->Update();
    }
    catch (const itk::ExceptionObject& e)
    {
      mitkThrow() << "Image statistics calculation failed due to following ITK Exception: \n " << e.what();
    }
    catch( const std::exception& e )
    {
      //mitkThrow() << "Image statistics calculation failed due to following ITK Exception: \n " << e.what();
    }

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

    Statistics statistics;
    statistics.Reset();
    statistics.SetLabel(1);
    statistics.SetN(image->GetBufferedRegion().GetNumberOfPixels());
    statistics.SetMin(statisticsFilter->GetMinimum());
    statistics.SetMax(statisticsFilter->GetMaximum());
    statistics.SetMean(statisticsFilter->GetMean());
    statistics.SetMedian(statisticsFilter->GetMedian());
    statistics.SetVariance(statisticsFilter->GetVariance());
    statistics.SetSkewness(statisticsFilter->GetSkewness());
    statistics.SetKurtosis(statisticsFilter->GetKurtosis());
    statistics.SetUniformity( statisticsFilter->GetUniformity());
    statistics.SetEntropy( statisticsFilter->GetEntropy());
    statistics.SetUPP( statisticsFilter->GetUPP());
    statistics.SetMPP( statisticsFilter->GetMPP());
    statistics.SetSigma(statisticsFilter->GetSigma());
    statistics.SetRMS(sqrt( statistics.GetMean() * statistics.GetMean() + statistics.GetSigma() * statistics.GetSigma() ));

    statistics.GetMinIndex().set_size(image->GetImageDimension());
    statistics.GetMaxIndex().set_size(image->GetImageDimension());

    vnl_vector<int> maxIndex;
    vnl_vector<int> minIndex;

    maxIndex.set_size( VImageDimension );
    minIndex.set_size( VImageDimension );

    typename MinMaxFilterType::IndexType tempMaxIndex = minMaxFilter->GetIndexOfMaximum();
    typename MinMaxFilterType::IndexType tempMinIndex = minMaxFilter->GetIndexOfMinimum();

    for (unsigned int i=0; i <VImageDimension; i++)
    {
      maxIndex[i] = tempMaxIndex[i];
      minIndex[i] = tempMinIndex[i];
    }

    statistics.SetMaxIndex(maxIndex);
    statistics.SetMinIndex(minIndex);

    if( IsHotspotCalculated() && VImageDimension == 3 )
    {
      typedef itk::Image< unsigned short, VImageDimension > MaskImageType;
      typename MaskImageType::Pointer nullMask;
      bool isHotspotDefined(false);
      Statistics hotspotStatistics = this->CalculateHotspotStatistics(image, nullMask.GetPointer(), m_HotspotRadiusInMM, isHotspotDefined, 0 );
      if (isHotspotDefined)
      {
        statistics.SetHasHotspotStatistics(true);
        statistics.GetHotspotStatistics() = hotspotStatistics;
      }
      else
      {
        statistics.SetHasHotspotStatistics(false);
      }

      if(statistics.GetHotspotStatistics().HasHotspotStatistics() )
      {
        MITK_DEBUG << "Hotspot statistics available";
        statistics.SetHotspotIndex(hotspotStatistics.GetHotspotIndex());
      }
      else
      {
        MITK_ERROR << "No hotspot statistics available!";
      }
    }

    statisticsContainer->push_back( statistics );

    // Calculate histogram
    // calculate bin size or number of bins
    unsigned int numberOfBins = 200; // default number of bins
    if (m_UseDefaultBinSize)
    {
      m_HistogramBinSize = std::ceil( (statistics.GetMax() - statistics.GetMin() + 1)/numberOfBins );
    }
    else
    {
      numberOfBins = calcNumberOfBins(statistics.GetMin(), statistics.GetMax());
    }
    typename HistogramGeneratorType::Pointer histogramGenerator = HistogramGeneratorType::New();
    histogramGenerator->SetInput( image );
    histogramGenerator->SetMarginalScale( 100 );
    histogramGenerator->SetNumberOfBins( numberOfBins );
    histogramGenerator->SetHistogramMin( statistics.GetMin() );
    histogramGenerator->SetHistogramMax( statistics.GetMax() );
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
    typedef typename ImageType::Pointer ImagePointer;
    typedef itk::ExtendedLabelStatisticsImageFilter< ImageType, MaskImageType > LabelStatisticsFilterType;
    typedef itk::ChangeInformationImageFilter< MaskImageType > ChangeInformationFilterType;
    typedef itk::ExtractImageFilter< ImageType, ImageType > ExtractImageFilterType;

    statisticsContainer->clear();
    histogramContainer->clear();

    // Make sure that mask is set
    if ( maskImage == nullptr  )
    {
      itkExceptionMacro( << "Mask image needs to be set!" );
    }

    // Make sure that spacing of mask and image are the same
    //SpacingType imageSpacing = image->GetSpacing();
    //SpacingType maskSpacing = maskImage->GetSpacing();
    //PointType zeroPoint; zeroPoint.Fill( 0.0 );
    //if ( (zeroPoint + imageSpacing).SquaredEuclideanDistanceTo( (zeroPoint + maskSpacing) ) > mitk::eps )
    //{
    //  itkExceptionMacro( << "Mask needs to have same spacing as image! (Image spacing: " << imageSpacing << "; Mask spacing: " << maskSpacing << ")" );
    //}
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
          double differenceDirection = imageDirection[i][j] - maskDirection[i][j];
          if ( fabs( differenceDirection ) > 0.001 /*mitk::eps*/ ) // TODO: temp fix (bug 17121)
          {
            itkExceptionMacro( << "Mask needs to have same direction as image! (Image direction: " << imageDirection << "; Mask direction: " << maskDirection << ")" );
          }
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
        itkWarningMacro( << "Pixels/voxels of mask and image are not sufficiently aligned! (Misalignment: " << misalignment << ")" );
      }

      double indexCoordDistance = maskOriginContinousIndex[i] - imageOriginContinousIndex[i];
      offset[i] = int( indexCoordDistance + image->GetBufferedRegion().GetIndex()[i] + 0.5 );
    }

    // Adapt the origin and region (index/size) of the mask so that the origin of both are the same
    typename ChangeInformationFilterType::Pointer adaptMaskFilter;
    adaptMaskFilter = ChangeInformationFilterType::New();
    adaptMaskFilter->ChangeOriginOn();
    adaptMaskFilter->ChangeRegionOn();
    adaptMaskFilter->SetInput( maskImage );
    adaptMaskFilter->SetOutputOrigin( image->GetOrigin() );
    adaptMaskFilter->SetOutputOffset( offset );
    adaptMaskFilter->SetCoordinateTolerance( 0.001 );
    adaptMaskFilter->SetDirectionTolerance( 0.001 );


    typename MaskImageType::Pointer adaptedMaskImage;
    try
    {
      adaptMaskFilter->Update();
      adaptedMaskImage = adaptMaskFilter->GetOutput();
    }
    catch( const itk::ExceptionObject &e)
    {
      mitkThrow() << "Attempt to adapt shifted origin of the mask image failed due to ITK Exception: \n" << e.what();
    }
    catch( const std::exception& e )
    {
      //mitkThrow() << "Image statistics calculation failed due to following ITK Exception: \n " << e.what();
    }


    // Make sure that mask region is contained within image region
    if ( adaptedMaskImage.IsNotNull() &&
      !image->GetLargestPossibleRegion().IsInside( adaptedMaskImage->GetLargestPossibleRegion() ) )
    {
      itkWarningMacro( << "Mask region needs to be inside of image region! (Image region: "
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
      extractImageFilter->SetCoordinateTolerance( 0.001 );
      extractImageFilter->SetDirectionTolerance( 0.001 );
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

    try
    {
      statisticsFilter->Update();
    }
    catch( const itk::ExceptionObject& e)
    {
      mitkThrow() << "Image statistics initialization computation failed with ITK Exception: \n " << e.what();
    }
    catch( const std::exception& e )
    {
      //mitkThrow() << "Image statistics calculation failed due to following ITK Exception: \n " << e.what();
    }

    // Calculate bin size or number of bins
    unsigned int numberOfBins = 200; // default number of bins
    double maximum = 0.0;
    double minimum = 0.0;

    if (m_UseBinSizeBasedOnVOIRegion)
    {
      maximum = statisticsFilter->GetMaximum();
      minimum = statisticsFilter->GetMinimum();

      if (m_UseDefaultBinSize)
      {
        m_HistogramBinSize = std::ceil( static_cast<double>((statisticsFilter->GetMaximum() - statisticsFilter->GetMinimum() + 1)/numberOfBins) );
      }
      else
      {
        numberOfBins = calcNumberOfBins(statisticsFilter->GetMinimum(), statisticsFilter->GetMaximum());
      }
    }
    else
    {
      double sig = 0.0;
      int counter = 0;

      //Find the min and max values for the Roi to set the range for the histogram
      GetMinAndMaxValue( minimum, maximum, counter, sig, image, maskImage);
      numberOfBins = maximum - minimum;
      if(maximum - minimum <= 10)
      {
        numberOfBins = 100;
      }
    }


    typename LabelStatisticsFilterType::Pointer labelStatisticsFilter = LabelStatisticsFilterType::New();
    labelStatisticsFilter->SetInput( adaptedImage );
    labelStatisticsFilter->SetLabelInput( adaptedMaskImage );
    labelStatisticsFilter->SetCoordinateTolerance( 0.001 );
    labelStatisticsFilter->SetDirectionTolerance( 0.001 );
    labelStatisticsFilter->UseHistogramsOn();
    labelStatisticsFilter->SetHistogramParameters( numberOfBins, floor(minimum), ceil(maximum) );   //statisticsFilter->GetMinimum() statisticsFilter->GetMaximum()

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
    try
    {
      labelStatisticsFilter->Update();
    }
    catch( const itk::ExceptionObject& e)
    {
      mitkThrow() << "Image statistics calculation failed due to following ITK Exception: \n " << e.what();
    }
    catch( const std::exception& e )
    {
      //mitkThrow() << "Image statistics calculation failed due to following ITK Exception: \n " << e.what();
    }

    this->InvokeEvent( itk::EndEvent() );

    if( observerTag )
      labelStatisticsFilter->RemoveObserver( observerTag );

    // Find all relevant labels of mask (other than 0)
    std::list< int > relevantLabels = labelStatisticsFilter->GetRelevantLabels();
    unsigned int i;

    if ( labelStatisticsFilter->GetMaskingNonEmpty() )
    {
      std::list< int >::iterator it;
      for ( it = relevantLabels.begin(), i = 0;
        it != relevantLabels.end();
        ++it, ++i )
      {
        Statistics statistics; // restore previous code
        labelStatisticsFilter->GetHistogram(*it) ;
        histogramContainer->push_back( HistogramType::ConstPointer( labelStatisticsFilter->GetHistogram( (*it) ) ) );

        statistics.SetLabel (*it);
        statistics.SetN(labelStatisticsFilter->GetCount( *it ));
        statistics.SetMin(labelStatisticsFilter->GetMinimum( *it ));
        statistics.SetMax(labelStatisticsFilter->GetMaximum( *it ));
        statistics.SetMean(labelStatisticsFilter->GetMean( *it ));
        statistics.SetMedian(labelStatisticsFilter->GetMedian( *it));
        statistics.SetMedian(labelStatisticsFilter->GetMedian( *it ));
        statistics.SetVariance(labelStatisticsFilter->GetVariance( *it ));
        statistics.SetSigma(labelStatisticsFilter->GetSigma( *it ));
        statistics.SetSkewness(labelStatisticsFilter->GetSkewness( *it ));
        statistics.SetKurtosis(labelStatisticsFilter->GetKurtosis( *it ));
        statistics.SetUniformity( labelStatisticsFilter->GetUniformity( *it ));
        statistics.SetEntropy( labelStatisticsFilter->GetEntropy( *it ));
        statistics.SetUPP( labelStatisticsFilter->GetUPP( *it));
        statistics.SetMPP( labelStatisticsFilter->GetMPP( *it));
        statistics.SetRMS(sqrt( statistics.GetMean() * statistics.GetMean()
          + statistics.GetSigma() * statistics.GetSigma() ));

        // restrict image to mask area for min/max index calculation
        typedef itk::MaskImageFilter< ImageType, MaskImageType, ImageType > MaskImageFilterType;
        typename MaskImageFilterType::Pointer masker = MaskImageFilterType::New();
        bool isMinAndMaxSameValue = (statistics.GetMin() == statistics.GetMax());
        // bug 17962: following is a workaround for the case when min and max are the same, we can probably find a nicer way here
        double outsideValue = (isMinAndMaxSameValue ? (statistics.GetMax()/2) : (statistics.GetMin()+statistics.GetMax())/2);
        masker->SetOutsideValue( outsideValue );
        masker->SetInput1(adaptedImage);
        masker->SetInput2(adaptedMaskImage);
        masker->SetCoordinateTolerance( 0.001 );
        masker->SetDirectionTolerance( 0.001 );
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
        // bug 17962: following is a workaround for the case when min and max are the same, we can probably find a nicer way here
        typename MinMaxFilterType::IndexType tempMinIndex =
          (isMinAndMaxSameValue ? minMaxFilter->GetIndexOfMaximum() : minMaxFilter->GetIndexOfMinimum());

        // FIX BUG 14644
        //If a PlanarFigure is used for segmentation the
        //adaptedImage is a single slice (2D). Adding the
        // 3. dimension.

        // FIX Bug 19625 pt. 1
        // m_Image will yield 4 coordinates if it has 4 Dimensions, the min max value is however only searched for in one timeSliceImage (3D)
        // the 3D min/max coordinates are then set and the 4th coordinate is blank, causing random numbers to appear in the GUI
        // (Fix = replace m_Image with VImageDimension (Dimension of image))

        vnl_vector<int> maxIndex;
        vnl_vector<int> minIndex;
        unsigned int imageDimension = m_Image->GetDimension();
        unsigned int maxDimensionsToDisplay = 3; // we do not want to display the time step as part of the coordinates
        maxIndex.set_size(std::min(imageDimension, maxDimensionsToDisplay));
        minIndex.set_size(std::min(imageDimension, maxDimensionsToDisplay));

        if (m_MaskingMode == MASKING_MODE_PLANARFIGURE && imageDimension == 3)
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
        // FIX END
        statistics.SetMaxIndex(maxIndex);
        statistics.SetMinIndex(minIndex);
        /*****************************************************Calculate Hotspot Statistics**********************************************/

        if(IsHotspotCalculated() && VImageDimension == 3)
        {
          bool isDefined(false);
          Statistics hotspotStatistics = CalculateHotspotStatistics(adaptedImage.GetPointer(), adaptedMaskImage.GetPointer(),GetHotspotRadiusInMM(), isDefined, *it);
          statistics.GetHotspotStatistics() = hotspotStatistics;
          if(statistics.GetHotspotStatistics().HasHotspotStatistics())
          {
            MITK_DEBUG << "Hotspot statistics available";
            statistics.SetHotspotIndex( hotspotStatistics.GetHotspotIndex() );
          }
          else
          {
            MITK_ERROR << "No hotspot statistics available!";
          }
        }
        statisticsContainer->push_back( statistics );
      }
    }
    else
    {
      histogramContainer->push_back( HistogramType::ConstPointer( m_EmptyHistogram ) );
      statisticsContainer->push_back( Statistics() );
    }
  }


  template <typename TPixel, unsigned int VImageDimension  >
  ImageStatisticsCalculator::ImageExtrema
    ImageStatisticsCalculator::CalculateExtremaWorld(
    const itk::Image<TPixel, VImageDimension> *inputImage,
    itk::Image<unsigned short, VImageDimension> *maskImage,
    double neccessaryDistanceToImageBorderInMM,
    unsigned int label)
  {
    typedef itk::Image< TPixel, VImageDimension > ImageType;
    typedef itk::Image< unsigned short, VImageDimension > MaskImageType;

    typedef itk::ImageRegionConstIteratorWithIndex<MaskImageType> MaskImageIteratorType;
    typedef itk::ImageRegionConstIteratorWithIndex<ImageType> InputImageIndexIteratorType;

    typename ImageType::SpacingType spacing = inputImage->GetSpacing();

    ImageExtrema minMax;
    minMax.Defined = false;
    minMax.MaxIndex.set_size(VImageDimension);
    minMax.MaxIndex.set_size(VImageDimension);

    typename ImageType::RegionType allowedExtremaRegion = inputImage->GetLargestPossibleRegion();

    bool keepDistanceToImageBorders( neccessaryDistanceToImageBorderInMM > 0 );
    if (keepDistanceToImageBorders)
    {
      long distanceInPixels[VImageDimension];
      for(unsigned short dimension = 0; dimension < VImageDimension; ++dimension)
      {
        // To confirm that the whole hotspot is inside the image we have to keep a specific distance to the image-borders, which is as long as
        // the radius. To get the amount of indices we divide the radius by spacing and add 0.5 because voxels are center based:
        // For example with a radius of 2.2 and a spacing of 1 two indices are enough because 2.2 / 1 + 0.5 = 2.7 => 2.
        // But with a radius of 2.7 we need 3 indices because 2.7 / 1 + 0.5 = 3.2 => 3
        distanceInPixels[dimension] = int( neccessaryDistanceToImageBorderInMM / spacing[dimension] + 0.5);
      }

      allowedExtremaRegion.ShrinkByRadius(distanceInPixels);
    }

    InputImageIndexIteratorType imageIndexIt(inputImage, allowedExtremaRegion);

    float maxValue = itk::NumericTraits<float>::min();
    float minValue = itk::NumericTraits<float>::max();

    typename ImageType::IndexType maxIndex;
    typename ImageType::IndexType minIndex;

    for(unsigned short i = 0; i < VImageDimension; ++i)
    {
      maxIndex[i] = 0;
      minIndex[i] = 0;
    }

    if (maskImage != nullptr)
    {
      MaskImageIteratorType maskIt(maskImage, maskImage->GetLargestPossibleRegion());
      typename ImageType::IndexType imageIndex;
      typename ImageType::PointType worldPosition;
      typename ImageType::IndexType maskIndex;

      for(maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt)
      {
        imageIndex = maskIndex = maskIt.GetIndex();

        if(maskIt.Get() == label)
        {
          if( allowedExtremaRegion.IsInside(imageIndex) )
          {
            imageIndexIt.SetIndex( imageIndex );
            double value = imageIndexIt.Get();
            minMax.Defined = true;

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
      }
    }
    else
    {
      for(imageIndexIt.GoToBegin(); !imageIndexIt.IsAtEnd(); ++imageIndexIt)
      {
        double value = imageIndexIt.Get();
        minMax.Defined = true;

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

    minMax.MaxIndex.set_size(VImageDimension);
    minMax.MinIndex.set_size(VImageDimension);

    for(unsigned int i = 0; i < minMax.MaxIndex.size(); ++i)
    {
      minMax.MaxIndex[i] = maxIndex[i];
    }

    for(unsigned int i = 0; i < minMax.MinIndex.size(); ++i)
    {
      minMax.MinIndex[i] = minIndex[i];
    }

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
      maskSize[i] = static_cast<int>( 2 * radiusInMM / spacing[i]);

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
    ::GenerateHotspotSearchConvolutionKernel(double mmPerPixel[VImageDimension], double radiusInMM)
  {
    std::stringstream ss;
    for (unsigned int i = 0; i < VImageDimension; ++i)
    {
      ss << mmPerPixel[i];
      if (i < VImageDimension -1)
        ss << ",";
    }
    MITK_DEBUG << "Update convolution kernel for spacing (" << ss.str() << ") and radius " << radiusInMM << "mm";


    double radiusInMMSquared = radiusInMM * radiusInMM;
    typedef itk::Image< float, VImageDimension > KernelImageType;
    typename KernelImageType::Pointer convolutionKernel = KernelImageType::New();

    // Calculate size and allocate mask image
    typedef typename KernelImageType::SizeType SizeType;
    SizeType maskSize = this->CalculateConvolutionKernelSize<VImageDimension>(mmPerPixel, radiusInMM);

    Point3D convolutionMaskCenterIndex; convolutionMaskCenterIndex.Fill(0.0);
    for(unsigned int i = 0; i < VImageDimension; ++i)
    {
      convolutionMaskCenterIndex[i] = 0.5 * (double)(maskSize[i]-1);
    }

    typedef typename KernelImageType::IndexType IndexType;
    IndexType maskIndex;
    maskIndex.Fill(0);

    typedef typename KernelImageType::RegionType RegionType;
    RegionType maskRegion;
    maskRegion.SetSize(maskSize);
    maskRegion.SetIndex(maskIndex);

    convolutionKernel->SetRegions(maskRegion);
    convolutionKernel->SetSpacing(mmPerPixel);
    convolutionKernel->Allocate();

    // Fill mask image values by subsampling the image grid
    typedef itk::ImageRegionIteratorWithIndex<KernelImageType> MaskIteratorType;
    MaskIteratorType maskIt(convolutionKernel,maskRegion);

    int numberOfSubVoxelsPerDimension = 2; // per dimension!
    int numberOfSubVoxels = ::pow( static_cast<float>(numberOfSubVoxelsPerDimension), static_cast<float>(VImageDimension) );
    double subVoxelSizeInPixels = 1.0 / (double)numberOfSubVoxelsPerDimension;
    double valueOfOneSubVoxel = 1.0 / (double)numberOfSubVoxels;
    double maskValue = 0.0;
    Point3D subVoxelIndexPosition;
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

      maskValue = 0.0;
      Vector3D subVoxelOffset; subVoxelOffset.Fill(0.0);
      // iterate sub-voxels by iterating all possible offsets
      for (subVoxelOffset[0] = -0.5 + subVoxelSizeInPixels / 2.0;
        subVoxelOffset[0] < +0.5;
        subVoxelOffset[0] += subVoxelSizeInPixels)
      {
        for (subVoxelOffset[1] = -0.5 + subVoxelSizeInPixels / 2.0;
          subVoxelOffset[1] < +0.5;
          subVoxelOffset[1] += subVoxelSizeInPixels)
        {
          for (subVoxelOffset[2] = -0.5 + subVoxelSizeInPixels / 2.0;
            subVoxelOffset[2] < +0.5;
            subVoxelOffset[2] += subVoxelSizeInPixels)
          {
            subVoxelIndexPosition = voxelPosition + subVoxelOffset; // this COULD be integrated into the for-loops if neccessary (add voxelPosition to initializer and end condition)
            distanceSquared =
              (subVoxelIndexPosition[0]-convolutionMaskCenterIndex[0]) * mmPerPixel[0] * (subVoxelIndexPosition[0]-convolutionMaskCenterIndex[0]) * mmPerPixel[0]
            + (subVoxelIndexPosition[1]-convolutionMaskCenterIndex[1]) * mmPerPixel[1] * (subVoxelIndexPosition[1]-convolutionMaskCenterIndex[1]) * mmPerPixel[1]
            + (subVoxelIndexPosition[2]-convolutionMaskCenterIndex[2]) * mmPerPixel[2] * (subVoxelIndexPosition[2]-convolutionMaskCenterIndex[2]) * mmPerPixel[2];

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
  itk::SmartPointer<itk::Image<TPixel, VImageDimension> >
    ImageStatisticsCalculator::GenerateConvolutionImage( const itk::Image<TPixel, VImageDimension>* inputImage )
  {
    double mmPerPixel[VImageDimension];
    for (unsigned int dimension = 0; dimension < VImageDimension; ++dimension)
    {
      mmPerPixel[dimension] = inputImage->GetSpacing()[dimension];
    }

    // update convolution kernel
    typedef itk::Image< float, VImageDimension > KernelImageType;
    typename KernelImageType::Pointer convolutionKernel = this->GenerateHotspotSearchConvolutionKernel<VImageDimension>(mmPerPixel, m_HotspotRadiusInMM);

    // update convolution image
    typedef itk::Image< TPixel, VImageDimension > InputImageType;
    typedef itk::Image< TPixel, VImageDimension > ConvolutionImageType;
    typedef itk::FFTConvolutionImageFilter<InputImageType,
      KernelImageType,
      ConvolutionImageType> ConvolutionFilterType;

    typename ConvolutionFilterType::Pointer convolutionFilter = ConvolutionFilterType::New();
    typedef itk::ConstantBoundaryCondition<InputImageType, InputImageType> BoundaryConditionType;
    BoundaryConditionType boundaryCondition;
    boundaryCondition.SetConstant(0.0);

    if (GetHotspotMustBeCompletlyInsideImage())
    {
      // overwrite default boundary condition
      convolutionFilter->SetBoundaryCondition(&boundaryCondition);
    }

    convolutionFilter->SetInput(inputImage);
    convolutionFilter->SetKernelImage(convolutionKernel);
    convolutionFilter->SetNormalize(true);
    MITK_DEBUG << "Update Convolution image for hotspot search";
    convolutionFilter->UpdateLargestPossibleRegion();

    typename ConvolutionImageType::Pointer convolutionImage = convolutionFilter->GetOutput();
    convolutionImage->SetSpacing( inputImage->GetSpacing() ); // only workaround because convolution filter seems to ignore spacing of input image

    m_HotspotRadiusInMMChanged = false;
    return convolutionImage;
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

    MaskImageIteratorType maskIt(maskImage, maskImage->GetLargestPossibleRegion());

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
  ImageStatisticsCalculator::Statistics
    ImageStatisticsCalculator::CalculateHotspotStatistics(
    const itk::Image<TPixel, VImageDimension>* inputImage,
    itk::Image<unsigned short, VImageDimension>* maskImage,
    double radiusInMM,
    bool& isHotspotDefined,
    unsigned int label)
  {
    // get convolution image (updated in GenerateConvolutionImage())
    typedef itk::Image< TPixel, VImageDimension > InputImageType;
    typedef itk::Image< TPixel, VImageDimension > ConvolutionImageType;
    typedef itk::Image< float, VImageDimension > KernelImageType;
    typedef itk::Image< unsigned short, VImageDimension > MaskImageType;

    //typename ConvolutionImageType::Pointer convolutionImage = dynamic_cast<ConvolutionImageType*>(this->GenerateConvolutionImage(inputImage));
    typename ConvolutionImageType::Pointer convolutionImage = this->GenerateConvolutionImage(inputImage);

    if (convolutionImage.IsNull())
    {
      MITK_ERROR << "Empty convolution image in CalculateHotspotStatistics(). We should never reach this state (logic error).";
      throw std::logic_error("Empty convolution image in CalculateHotspotStatistics()");
    }

    // find maximum in convolution image, given the current mask
    double requiredDistanceToBorder = m_HotspotMustBeCompletelyInsideImage ? m_HotspotRadiusInMM : -1.0;
    ImageExtrema convolutionImageInformation = CalculateExtremaWorld(convolutionImage.GetPointer(), maskImage, requiredDistanceToBorder, label);

    isHotspotDefined = convolutionImageInformation.Defined;

    if (!isHotspotDefined)
    {
      m_EmptyStatistics.Reset(VImageDimension);
      MITK_ERROR << "No origin of hotspot-sphere was calculated! Returning empty statistics";
      return m_EmptyStatistics;
    }
    else
    {
      // create a binary mask around the "hotspot" region, fill the shape of a sphere around our hotspot center
      typedef itk::ImageDuplicator< InputImageType > DuplicatorType;
      typename DuplicatorType::Pointer copyMachine = DuplicatorType::New();
      copyMachine->SetInputImage(inputImage);
      copyMachine->Update();

      typedef itk::CastImageFilter< InputImageType, MaskImageType > CastFilterType;
      typename CastFilterType::Pointer caster = CastFilterType::New();
      caster->SetInput( copyMachine->GetOutput() );
      caster->Update();
      typename MaskImageType::Pointer hotspotMaskITK = caster->GetOutput();

      typedef typename InputImageType::IndexType IndexType;
      IndexType maskCenterIndex;
      for (unsigned int d =0; d< VImageDimension;++d) maskCenterIndex[d]=convolutionImageInformation.MaxIndex[d];
      typename ConvolutionImageType::PointType maskCenter;
      inputImage->TransformIndexToPhysicalPoint(maskCenterIndex,maskCenter);

      this->FillHotspotMaskPixels(hotspotMaskITK.GetPointer(), maskCenter, radiusInMM);

      // calculate statistics within the binary mask
      typedef itk::ExtendedLabelStatisticsImageFilter< InputImageType, MaskImageType> LabelStatisticsFilterType;
      typename LabelStatisticsFilterType::Pointer labelStatisticsFilter;
      labelStatisticsFilter = LabelStatisticsFilterType::New();
      labelStatisticsFilter->SetInput( inputImage );
      labelStatisticsFilter->SetLabelInput( hotspotMaskITK );
      labelStatisticsFilter->SetCoordinateTolerance( 0.001 );
      labelStatisticsFilter->SetDirectionTolerance( 0.001 );

      labelStatisticsFilter->Update();

      Statistics hotspotStatistics;
      hotspotStatistics.SetHotspotIndex(convolutionImageInformation.MaxIndex);
      hotspotStatistics.SetMean(convolutionImageInformation.Max);

      if ( labelStatisticsFilter->HasLabel( 1 ) )
      {
        hotspotStatistics.SetLabel (1);
        hotspotStatistics.SetN(labelStatisticsFilter->GetCount(1));
        hotspotStatistics.SetMin(labelStatisticsFilter->GetMinimum(1));
        hotspotStatistics.SetMax(labelStatisticsFilter->GetMaximum(1));
        hotspotStatistics.SetMedian(labelStatisticsFilter->GetMedian(1));
        hotspotStatistics.SetVariance(labelStatisticsFilter->GetVariance(1));
        hotspotStatistics.SetSigma(labelStatisticsFilter->GetSigma(1));
        hotspotStatistics.SetRMS(sqrt( hotspotStatistics.GetMean() * hotspotStatistics.GetMean()
          + hotspotStatistics.GetSigma() * hotspotStatistics.GetSigma() ));

        MITK_DEBUG << "Statistics for inside hotspot: Mean " << hotspotStatistics.GetMean()
          << ", SD " << hotspotStatistics.GetSigma()
          << ", Max " << hotspotStatistics.GetMax()
          << ", Min " << hotspotStatistics.GetMin();
      }
      else
      {
        MITK_ERROR << "Uh oh! Unable to calculate statistics for hotspot region...";
        return m_EmptyStatistics;
      }

      return hotspotStatistics;
    }
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
    const mitk::PlaneGeometry *planarFigurePlaneGeometry = m_PlanarFigure->GetPlaneGeometry();
    const typename PlanarFigure::PolyLineType planarFigurePolyline = m_PlanarFigure->GetPolyLine( 0 );
    const mitk::BaseGeometry *imageGeometry3D = m_Image->GetGeometry( 0 );
    // If there is a second poly line in a closed planar figure, treat it as a hole.
    PlanarFigure::PolyLineType planarFigureHolePolyline;

    if (m_PlanarFigure->GetPolyLinesSize() == 2)
      planarFigureHolePolyline = m_PlanarFigure->GetPolyLine(1);


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
      planarFigurePlaneGeometry->Map( *it, point3D );

      // Polygons (partially) outside of the image bounds can not be processed
      // further due to a bug in vtkPolyDataToImageStencil
      if ( !imageGeometry3D->IsInside( point3D ) )
      {
        outOfBounds = true;
      }

      imageGeometry3D->WorldToIndex( point3D, point3D );

      points->InsertNextPoint( point3D[i0], point3D[i1], 0 );
    }

    vtkSmartPointer<vtkPoints> holePoints = nullptr;

    if (!planarFigureHolePolyline.empty())
    {
      holePoints = vtkSmartPointer<vtkPoints>::New();

      Point3D point3D;
      PlanarFigure::PolyLineType::const_iterator end = planarFigureHolePolyline.end();

      for (it = planarFigureHolePolyline.begin(); it != end; ++it)
      {
        planarFigurePlaneGeometry->Map(*it, point3D);
        imageGeometry3D->WorldToIndex(point3D, point3D);
        holePoints->InsertNextPoint(point3D[i0], point3D[i1], 0);
      }
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

    vtkSmartPointer<vtkLassoStencilSource> holeLassoStencil = nullptr;

    if (holePoints.GetPointer() != nullptr)
    {
      holeLassoStencil = vtkSmartPointer<vtkLassoStencilSource>::New();
      holeLassoStencil->SetShapeToPolygon();
      holeLassoStencil->SetPoints(holePoints);
    }

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
    imageStencilFilter->SetStencilConnection(lassoStencil->GetOutputPort());
    imageStencilFilter->ReverseStencilOff();
    imageStencilFilter->SetBackgroundValue( 0 );
    imageStencilFilter->Update();

    vtkSmartPointer<vtkImageStencil> holeStencilFilter = nullptr;

    if (holeLassoStencil.GetPointer() != nullptr)
    {
      holeStencilFilter = vtkSmartPointer<vtkImageStencil>::New();
      holeStencilFilter->SetInputConnection(imageStencilFilter->GetOutputPort());
      holeStencilFilter->SetStencilConnection(holeLassoStencil->GetOutputPort());
      holeStencilFilter->ReverseStencilOn();
      holeStencilFilter->SetBackgroundValue(0);
      holeStencilFilter->Update();
    }

    // Export from VTK back to ITK
    vtkSmartPointer<vtkImageExport> vtkExporter = vtkSmartPointer<vtkImageExport>::New();
    vtkExporter->SetInputConnection( holeStencilFilter.GetPointer() == nullptr
      ? imageStencilFilter->GetOutputPort()
      : holeStencilFilter->GetOutputPort());
    vtkExporter->Update();

    typename ImageImportType::Pointer itkImporter = ImageImportType::New();
    this->ConnectPipelines( vtkExporter, itkImporter );
    itkImporter->Update();

    typedef itk::ImageDuplicator< ImageImportType::OutputImageType > DuplicatorType;
    DuplicatorType::Pointer duplicator = DuplicatorType::New();
    duplicator->SetInputImage( itkImporter->GetOutput() );
    duplicator->Update();

    // Store mask
    m_InternalImageMask2D = duplicator->GetOutput();
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
