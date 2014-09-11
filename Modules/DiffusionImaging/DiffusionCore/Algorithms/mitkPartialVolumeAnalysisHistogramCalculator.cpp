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


#include "mitkPartialVolumeAnalysisHistogramCalculator.h"
#include "mitkImageAccessByItk.h"
#include "mitkExtractImageFilter.h"

#include <itkScalarImageToHistogramGenerator.h>

#include <itkStatisticsImageFilter.h>
#include <itkLabelStatisticsImageFilter.h>
#include <itkBSplineUpsampleImageFilter.h>
#include <itkBSplineResampleImageFilterBase.h>
#include "itkResampleImageFilter.h"
#include "itkGaussianInterpolateImageFunction.h"

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

#include "itkGaussianInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

#include "itkImageMaskSpatialObject.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkListSample.h"

#include "mitkAbstractTransformGeometry.h"

#include <iostream>
#include <sstream>

namespace mitk
{

  PartialVolumeAnalysisHistogramCalculator::PartialVolumeAnalysisHistogramCalculator()
    : m_MaskingMode( MASKING_MODE_NONE ),
    m_MaskingModeChanged( false ),
    m_NumberOfBins(256),
    m_UpsamplingFactor(1),
    m_GaussianSigma(0),
    m_ForceUpdate(false),
    m_PlanarFigureThickness(0)
  {
    m_EmptyHistogram = HistogramType::New();
    m_EmptyHistogram->SetMeasurementVectorSize(1);
    HistogramType::SizeType histogramSize(1);
    histogramSize.Fill( 256 );
    m_EmptyHistogram->Initialize( histogramSize );

    m_EmptyStatistics.Reset();
  }


  PartialVolumeAnalysisHistogramCalculator::~PartialVolumeAnalysisHistogramCalculator()
  {
  }


  void PartialVolumeAnalysisHistogramCalculator::SetImage( const mitk::Image *image )
  {
    if ( m_Image != image )
    {
      m_Image = image;
      this->Modified();

      m_ImageStatisticsTimeStamp.Modified();
      m_ImageStatisticsCalculationTriggerBool = true;
    }
  }

  void PartialVolumeAnalysisHistogramCalculator::AddAdditionalResamplingImage( const mitk::Image *image )
  {
    m_AdditionalResamplingImages.push_back(image);
    this->Modified();
    m_ImageStatisticsTimeStamp.Modified();
    m_ImageStatisticsCalculationTriggerBool = true;
  }

  void PartialVolumeAnalysisHistogramCalculator::SetModified( )
  {
    this->Modified();
    m_ImageStatisticsTimeStamp.Modified();
    m_ImageStatisticsCalculationTriggerBool = true;
    m_MaskedImageStatisticsTimeStamp.Modified();
    m_MaskedImageStatisticsCalculationTriggerBool = true;
    m_PlanarFigureStatisticsTimeStamp.Modified();
    m_PlanarFigureStatisticsCalculationTriggerBool = true;
  }

  void PartialVolumeAnalysisHistogramCalculator::SetImageMask( const mitk::Image *imageMask )
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

      m_MaskedImageStatisticsTimeStamp.Modified();
      m_MaskedImageStatisticsCalculationTriggerBool = true;
    }
  }


  void PartialVolumeAnalysisHistogramCalculator::SetPlanarFigure( const mitk::PlanarFigure *planarFigure )
  {
    if ( m_Image.IsNull() )
    {
      itkExceptionMacro( << "Image needs to be set first!" );
    }

    if ( m_PlanarFigure != planarFigure )
    {
      m_PlanarFigure = planarFigure;
      this->Modified();

      m_PlanarFigureStatisticsTimeStamp.Modified();
      m_PlanarFigureStatisticsCalculationTriggerBool = true;
    }
  }


  void PartialVolumeAnalysisHistogramCalculator::SetMaskingMode( unsigned int mode )
  {
    if ( m_MaskingMode != mode )
    {
      m_MaskingMode = mode;
      m_MaskingModeChanged = true;
      this->Modified();
    }
  }


  void PartialVolumeAnalysisHistogramCalculator::SetMaskingModeToNone()
  {
    if ( m_MaskingMode != MASKING_MODE_NONE )
    {
      m_MaskingMode = MASKING_MODE_NONE;
      m_MaskingModeChanged = true;
      this->Modified();
    }
  }


  void PartialVolumeAnalysisHistogramCalculator::SetMaskingModeToImage()
  {
    if ( m_MaskingMode != MASKING_MODE_IMAGE )
    {
      m_MaskingMode = MASKING_MODE_IMAGE;
      m_MaskingModeChanged = true;
      this->Modified();
    }
  }


  void PartialVolumeAnalysisHistogramCalculator::SetMaskingModeToPlanarFigure()
  {
    if ( m_MaskingMode != MASKING_MODE_PLANARFIGURE )
    {
      m_MaskingMode = MASKING_MODE_PLANARFIGURE;
      m_MaskingModeChanged = true;
      this->Modified();
    }
  }



  bool PartialVolumeAnalysisHistogramCalculator::ComputeStatistics()
  {

    MITK_DEBUG << "ComputeStatistics() start";

    if ( m_Image.IsNull() )
    {
      itkExceptionMacro( << "Image not set!" );
    }

    if ( m_Image->GetReferenceCount() == 1 )
    {
      MITK_DEBUG << "No Stats calculated; no one else holds a reference on it";
      return false;
    }

    // If a mask was set but we are the only ones to still hold a reference on
    // it, delete it.
    if ( m_ImageMask.IsNotNull() && (m_ImageMask->GetReferenceCount() == 1) )
    {
      m_ImageMask = NULL;
    }


    // Check if statistics is already up-to-date
    unsigned long imageMTime = m_ImageStatisticsTimeStamp.GetMTime();
    unsigned long maskedImageMTime = m_MaskedImageStatisticsTimeStamp.GetMTime();
    unsigned long planarFigureMTime = m_PlanarFigureStatisticsTimeStamp.GetMTime();

    bool imageStatisticsCalculationTrigger = m_ImageStatisticsCalculationTriggerBool;
    bool maskedImageStatisticsCalculationTrigger = m_MaskedImageStatisticsCalculationTriggerBool;
    bool planarFigureStatisticsCalculationTrigger = m_PlanarFigureStatisticsCalculationTriggerBool;

    if ( /*prevent calculation without mask*/!m_ForceUpdate &&( m_MaskingMode == MASKING_MODE_NONE || (
        ((m_MaskingMode != MASKING_MODE_NONE) || (imageMTime > m_Image->GetMTime() && !imageStatisticsCalculationTrigger))
        && ((m_MaskingMode != MASKING_MODE_IMAGE) || (m_ImageMask.IsNotNull() && maskedImageMTime > m_ImageMask->GetMTime() && !maskedImageStatisticsCalculationTrigger))
        && ((m_MaskingMode != MASKING_MODE_PLANARFIGURE) || (m_PlanarFigure.IsNotNull() && planarFigureMTime > m_PlanarFigure->GetMTime() && !planarFigureStatisticsCalculationTrigger)) ) ) )
    {
      MITK_DEBUG << "Returning, statistics already up to date!";
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


    // Depending on masking mode, extract and/or generate the required image
    // and mask data from the user input
    this->ExtractImageAndMask( );


    Statistics *statistics;
    HistogramType::ConstPointer *histogram;
    switch ( m_MaskingMode )
    {
    case MASKING_MODE_NONE:
    default:
      statistics = &m_ImageStatistics;
      histogram = &m_ImageHistogram;

      m_ImageStatisticsTimeStamp.Modified();
      m_ImageStatisticsCalculationTriggerBool = false;
      break;

    case MASKING_MODE_IMAGE:
      statistics = &m_MaskedImageStatistics;
      histogram = &m_MaskedImageHistogram;

      m_MaskedImageStatisticsTimeStamp.Modified();
      m_MaskedImageStatisticsCalculationTriggerBool = false;
      break;

    case MASKING_MODE_PLANARFIGURE:
      statistics = &m_PlanarFigureStatistics;
      histogram = &m_PlanarFigureHistogram;

      m_PlanarFigureStatisticsTimeStamp.Modified();
      m_PlanarFigureStatisticsCalculationTriggerBool = false;
      break;
    }

    // Calculate statistics and histogram(s)
    if ( m_InternalImage->GetDimension() == 3 )
    {
      if ( m_MaskingMode == MASKING_MODE_NONE )
      {
        // Reset state changed flag
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
      MITK_ERROR << "ImageStatistics: Image dimension not supported!";
    }

    // Release unused image smart pointers to free memory
    // m_InternalImage = mitk::Image::Pointer();
    m_InternalImageMask3D = MaskImage3DType::Pointer();
    m_InternalImageMask2D = MaskImage2DType::Pointer();
    return true;
  }


  const PartialVolumeAnalysisHistogramCalculator::HistogramType *
      PartialVolumeAnalysisHistogramCalculator::GetHistogram( ) const
  {
    if ( m_Image.IsNull() )
    {
      return NULL;
    }

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


  const PartialVolumeAnalysisHistogramCalculator::Statistics &
      PartialVolumeAnalysisHistogramCalculator::GetStatistics( ) const
  {
    if ( m_Image.IsNull() )
    {
      return m_EmptyStatistics;
    }

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


  void PartialVolumeAnalysisHistogramCalculator::ExtractImageAndMask( )
  {

    MITK_DEBUG << "ExtractImageAndMask( ) start";

    if ( m_Image.IsNull() )
    {
      throw std::runtime_error( "Error: image empty!" );
    }

    mitk::Image *timeSliceImage = const_cast<mitk::Image*>(m_Image.GetPointer());//imageTimeSelector->GetOutput();

    switch ( m_MaskingMode )
    {
    case MASKING_MODE_NONE:
      {
        m_InternalImage = timeSliceImage;
        int s = m_AdditionalResamplingImages.size();
        m_InternalAdditionalResamplingImages.resize(s);
        for(int i=0; i<s; i++)
        {
          m_InternalAdditionalResamplingImages[i] = const_cast<mitk::Image*>(m_AdditionalResamplingImages[i].GetPointer());
        }
        m_InternalImageMask2D = NULL;
        m_InternalImageMask3D = NULL;
        break;
      }

    case MASKING_MODE_IMAGE:
      {
        if ( m_ImageMask.IsNotNull() && (m_ImageMask->GetReferenceCount() > 1) )
        {
          ImageTimeSelector::Pointer maskedImageTimeSelector = ImageTimeSelector::New();
          maskedImageTimeSelector->SetInput( m_ImageMask );
          maskedImageTimeSelector->SetTimeNr( 0 );
          maskedImageTimeSelector->UpdateLargestPossibleRegion();
          mitk::Image *timeSliceMaskedImage = maskedImageTimeSelector->GetOutput();

          InternalMaskImage(timeSliceMaskedImage);

          if(m_UpsamplingFactor != 1)
          {
            InternalResampleImage(m_InternalImageMask3D);
          }

          AccessFixedDimensionByItk_1(
              timeSliceImage,
              InternalResampleImageFromMask, 3, -1 );

          int s = m_AdditionalResamplingImages.size();
          m_InternalAdditionalResamplingImages.resize(s);
          for(int i=0; i<s; i++)
          {
            AccessFixedDimensionByItk_1(
                m_AdditionalResamplingImages[i],
                InternalResampleImageFromMask, 3, i );
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

        const BaseGeometry *imageGeometry = timeSliceImage->GetUpdatedGeometry();
        if ( imageGeometry == NULL )
        {
          throw std::runtime_error( "Image geometry invalid!" );
        }

        const PlaneGeometry *planarFigureGeometry2D = m_PlanarFigure->GetPlaneGeometry();
        if ( planarFigureGeometry2D == NULL )
        {
          throw std::runtime_error( "Planar-Figure not yet initialized!" );
        }

        const PlaneGeometry *planarFigureGeometry =
            dynamic_cast< const PlaneGeometry * >( planarFigureGeometry2D );
        const AbstractTransformGeometry *abstrTransfGeometry =
            dynamic_cast< const AbstractTransformGeometry * >( planarFigureGeometry2D );

        if ( !planarFigureGeometry || abstrTransfGeometry )
        {
          throw std::runtime_error( "Only PlaneGeometry supported." );
        }


//        unsigned int axis = 2;
//        unsigned int slice = 0;

        AccessFixedDimensionByItk_3(
            timeSliceImage,
            InternalReorientImagePlane, 3,
            timeSliceImage->GetGeometry(),
            m_PlanarFigure->GetGeometry(), -1 );

        AccessFixedDimensionByItk_1(
            m_InternalImage,
            InternalCalculateMaskFromPlanarFigure,
            3, 2 );

        int s = m_AdditionalResamplingImages.size();
        for(int i=0; i<s; i++)
        {
            AccessFixedDimensionByItk_3(
                m_AdditionalResamplingImages[i],
                InternalReorientImagePlane, 3,
                timeSliceImage->GetGeometry(),
                m_PlanarFigure->GetGeometry(), i );

            AccessFixedDimensionByItk_1(
                m_InternalAdditionalResamplingImages[i],
                InternalCropAdditionalImage, 3, i );
        }
      }
    }

  }


  bool PartialVolumeAnalysisHistogramCalculator::GetPrincipalAxis(
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

  void PartialVolumeAnalysisHistogramCalculator::InternalMaskImage(
      mitk::Image *image )
  {

    typedef itk::ImageMaskSpatialObject<3> ImageMaskSpatialObject;
    typedef itk::Image< unsigned char, 3 > ImageType;
    typedef itk::ImageRegion<3>            RegionType;

    typedef mitk::ImageToItk<ImageType> CastType;
    CastType::Pointer caster = CastType::New();
    caster->SetInput(image);
    caster->Update();

    ImageMaskSpatialObject::Pointer maskSO = ImageMaskSpatialObject::New();
    maskSO->SetImage ( caster->GetOutput() );
    m_InternalMask3D  =
        maskSO->GetAxisAlignedBoundingBoxRegion();

    // check if bounding box is empty, if so set it to 1,1,1
    // to prevent empty mask image
    if (m_InternalMask3D.GetSize()[0] == 0 )
    {
      m_InternalMask3D.SetSize(0,1);
      m_InternalMask3D.SetSize(1,1);
      m_InternalMask3D.SetSize(2,1);
    }
    MITK_DEBUG << "Bounding Box Region: " << m_InternalMask3D;

    typedef itk::RegionOfInterestImageFilter< ImageType, MaskImage3DType > ROIFilterType;
    ROIFilterType::Pointer roi = ROIFilterType::New();
    roi->SetRegionOfInterest(m_InternalMask3D);
    roi->SetInput(caster->GetOutput());
    roi->Update();

    m_InternalImageMask3D = roi->GetOutput();

    MITK_DEBUG << "Created m_InternalImageMask3D";

  }


  template < typename TPixel, unsigned int VImageDimension >
      void PartialVolumeAnalysisHistogramCalculator::InternalReorientImagePlane(
          const itk::Image< TPixel, VImageDimension > *image,
          mitk::BaseGeometry* , mitk::BaseGeometry* planegeo3D, int additionalIndex )
  {

    MITK_DEBUG << "InternalReorientImagePlane() start";

    typedef itk::Image< TPixel, VImageDimension > ImageType;
    typedef itk::Image< float, VImageDimension > FloatImageType;

    typedef itk::ResampleImageFilter<ImageType, FloatImageType, double> ResamplerType;
    typename ResamplerType::Pointer resampler = ResamplerType::New();

    mitk::PlaneGeometry* planegeo = dynamic_cast<mitk::PlaneGeometry*>(planegeo3D);
    if ( !planegeo )
    {
      throw std::runtime_error( "Unexpected null pointer returned for pointer to PlaneGeometry." );
    }
    else
    {
      mitk::AbstractTransformGeometry* abstrGeo = dynamic_cast<mitk::AbstractTransformGeometry*>(planegeo3D);
      if ( abstrGeo )
      {
        throw std::runtime_error( "Unexpected pointer to AbstractTransformGeometry returned." );
      }
    }

    float upsamp = m_UpsamplingFactor;
    float gausssigma = m_GaussianSigma;

    // Spacing
    typename ResamplerType::SpacingType spacing = planegeo->GetSpacing();
    spacing[0] = image->GetSpacing()[0] / upsamp;
    spacing[1] = image->GetSpacing()[1] / upsamp;
    spacing[2] = image->GetSpacing()[2];
    if(m_PlanarFigureThickness)
    {
      spacing[2] = image->GetSpacing()[2] / upsamp;
    }
    resampler->SetOutputSpacing( spacing );

    // Size
    typename ResamplerType::SizeType size;
    size[0] = planegeo->GetExtentInMM(0) / spacing[0];
    size[1] = planegeo->GetExtentInMM(1) / spacing[1];
    size[2] = 1+2*m_PlanarFigureThickness; // klaus add +2*m_PlanarFigureThickness
    MITK_DEBUG << "setting size2:="<<size[2] << " (before " << 1 << ")";
    resampler->SetSize( size );

    // Origin
    typename mitk::Point3D orig = planegeo->GetOrigin();
    typename mitk::Point3D corrorig;
    planegeo3D->WorldToIndex(orig,corrorig);
    corrorig[0] += 0.5/upsamp;
    corrorig[1] += 0.5/upsamp;
    if(m_PlanarFigureThickness)
    {
      float thickyyy = m_PlanarFigureThickness;
      thickyyy/=upsamp;
      corrorig[2] -= thickyyy; // klaus add -= (float)m_PlanarFigureThickness/upsamp statt += 0
    }
    planegeo3D->IndexToWorld(corrorig,corrorig);
    resampler->SetOutputOrigin(corrorig );

    // Direction
    typename ResamplerType::DirectionType direction;
    typename mitk::AffineTransform3D::MatrixType matrix = planegeo->GetIndexToWorldTransform()->GetMatrix();
    for(int c=0; c<matrix.ColumnDimensions; c++)
    {
      double sum = 0;
      for(int r=0; r<matrix.RowDimensions; r++)
      {
        sum += matrix(r,c)*matrix(r,c);
      }
      for(int r=0; r<matrix.RowDimensions; r++)
      {
        direction(r,c) = matrix(r,c)/sqrt(sum);
      }
    }
    resampler->SetOutputDirection( direction );

    // Gaussian interpolation
    if(gausssigma != 0)
    {
      double sigma[3];
      for( unsigned int d = 0; d < 3; d++ )
      {
        sigma[d] = gausssigma * image->GetSpacing()[d];
      }
      double alpha = 2.0;

      typedef itk::GaussianInterpolateImageFunction<ImageType, double>
          GaussianInterpolatorType;

      typename GaussianInterpolatorType::Pointer interpolator
          = GaussianInterpolatorType::New();

      interpolator->SetInputImage( image );
      interpolator->SetParameters( sigma, alpha );

      resampler->SetInterpolator( interpolator );
    }
    else
    {
      //      typedef typename itk::BSplineInterpolateImageFunction<ImageType, double>
      //          InterpolatorType;
      typedef typename itk::LinearInterpolateImageFunction<ImageType, double> InterpolatorType;

      typename InterpolatorType::Pointer interpolator
          = InterpolatorType::New();

      interpolator->SetInputImage( image );

      resampler->SetInterpolator( interpolator );

    }

    // Other resampling options
    resampler->SetInput( image );
    resampler->SetDefaultPixelValue(0);

    MITK_DEBUG << "Resampling requested image plane ... ";
    resampler->Update();
    MITK_DEBUG << " ... done";

    if(additionalIndex < 0)
    {
      this->m_InternalImage = mitk::Image::New();
      this->m_InternalImage->InitializeByItk( resampler->GetOutput() );
      this->m_InternalImage->SetVolume( resampler->GetOutput()->GetBufferPointer() );
    }
    else
    {
      unsigned int myIndex = additionalIndex;
      this->m_InternalAdditionalResamplingImages.push_back(mitk::Image::New());
      this->m_InternalAdditionalResamplingImages[myIndex]->InitializeByItk( resampler->GetOutput() );
      this->m_InternalAdditionalResamplingImages[myIndex]->SetVolume( resampler->GetOutput()->GetBufferPointer() );
    }

  }

  template < typename TPixel, unsigned int VImageDimension >
      void PartialVolumeAnalysisHistogramCalculator::InternalResampleImageFromMask(
          const itk::Image< TPixel, VImageDimension > *image, int additionalIndex )
  {
    typedef itk::Image< TPixel, VImageDimension > ImageType;

    typename ImageType::Pointer outImage = ImageType::New();
    outImage->SetSpacing( m_InternalImageMask3D->GetSpacing() );   // Set the image spacing
    outImage->SetOrigin( m_InternalImageMask3D->GetOrigin() );     // Set the image origin
    outImage->SetDirection( m_InternalImageMask3D->GetDirection() );  // Set the image direction
    outImage->SetRegions( m_InternalImageMask3D->GetLargestPossibleRegion() );
    outImage->Allocate();
    outImage->FillBuffer(0);

    typedef itk::InterpolateImageFunction<ImageType, double>
        BaseInterpType;
    typedef itk::GaussianInterpolateImageFunction<ImageType, double>
        GaussianInterpolatorType;
    typedef itk::LinearInterpolateImageFunction<ImageType, double>
        LinearInterpolatorType;

    typename BaseInterpType::Pointer interpolator;

    if(m_GaussianSigma != 0)
    {
      double sigma[3];
      for( unsigned int d = 0; d < 3; d++ )
      {
        sigma[d] = m_GaussianSigma * image->GetSpacing()[d];
      }
      double alpha = 2.0;

      interpolator = GaussianInterpolatorType::New();
      dynamic_cast<GaussianInterpolatorType*>(interpolator.GetPointer())->SetParameters( sigma, alpha );

    }
    else
    {
      interpolator = LinearInterpolatorType::New();
    }

    interpolator->SetInputImage( image );

    itk::ImageRegionConstIterator<MaskImage3DType>
        itmask(m_InternalImageMask3D, m_InternalImageMask3D->GetLargestPossibleRegion());
    itk::ImageRegionIterator<ImageType>
        itimage(outImage, outImage->GetLargestPossibleRegion());

    itmask.GoToBegin();
    itimage.GoToBegin();

    itk::Point< double, 3 > point;
    itk::ContinuousIndex< double, 3 > index;
    while( !itmask.IsAtEnd() )
    {
      if(itmask.Get() != 0)
      {
        outImage->TransformIndexToPhysicalPoint (itimage.GetIndex(), point);
        image->TransformPhysicalPointToContinuousIndex(point, index);
        itimage.Set(interpolator->EvaluateAtContinuousIndex(index));
      }

      ++itmask;
      ++itimage;
    }

    if(additionalIndex < 0)
    {
      this->m_InternalImage = mitk::Image::New();
      this->m_InternalImage->InitializeByItk( outImage.GetPointer() );
      this->m_InternalImage->SetVolume( outImage->GetBufferPointer() );
    }
    else
    {
      this->m_InternalAdditionalResamplingImages[additionalIndex] = mitk::Image::New();
      this->m_InternalAdditionalResamplingImages[additionalIndex]->InitializeByItk( outImage.GetPointer() );
      this->m_InternalAdditionalResamplingImages[additionalIndex]->SetVolume( outImage->GetBufferPointer() );
    }

  }

  void PartialVolumeAnalysisHistogramCalculator::InternalResampleImage(
      const MaskImage3DType *image )
  {
    typedef itk::ResampleImageFilter<MaskImage3DType, MaskImage3DType, double> ResamplerType;
    ResamplerType::Pointer resampler = ResamplerType::New();

    // Size
    ResamplerType::SizeType size;
    size[0] = m_UpsamplingFactor * image->GetLargestPossibleRegion().GetSize()[0];
    size[1] = m_UpsamplingFactor * image->GetLargestPossibleRegion().GetSize()[1];
    size[2] = m_UpsamplingFactor * image->GetLargestPossibleRegion().GetSize()[2];;
    resampler->SetSize( size );

    // Origin
    mitk::Point3D orig = image->GetOrigin();
    resampler->SetOutputOrigin(orig );

    // Spacing
    ResamplerType::SpacingType spacing;
    spacing[0] = image->GetSpacing()[0] / m_UpsamplingFactor;
    spacing[1] = image->GetSpacing()[1] / m_UpsamplingFactor;
    spacing[2] = image->GetSpacing()[2] / m_UpsamplingFactor;
    resampler->SetOutputSpacing( spacing );

    resampler->SetOutputDirection( image->GetDirection() );

    typedef itk::NearestNeighborInterpolateImageFunction<MaskImage3DType, double>
        InterpolatorType;

    InterpolatorType::Pointer interpolator
        = InterpolatorType::New();

    interpolator->SetInputImage( image );

    resampler->SetInterpolator( interpolator );

    // Other resampling options
    resampler->SetInput( image );
    resampler->SetDefaultPixelValue(0);
    resampler->Update();

    m_InternalImageMask3D = resampler->GetOutput();

  }



  template < typename TPixel, unsigned int VImageDimension >
      void PartialVolumeAnalysisHistogramCalculator::InternalCalculateStatisticsUnmasked(
          const itk::Image< TPixel, VImageDimension > *image,
          Statistics &statistics,
          typename HistogramType::ConstPointer *histogram )
  {
    MITK_DEBUG << "InternalCalculateStatisticsUnmasked()";

    typedef itk::Image< TPixel, VImageDimension > ImageType;
    typedef itk::Image< unsigned char, VImageDimension > MaskImageType;
    typedef typename ImageType::IndexType IndexType;

    // Progress listening...
    typedef itk::SimpleMemberCommand< PartialVolumeAnalysisHistogramCalculator > ITKCommandType;
    ITKCommandType::Pointer progressListener;
    progressListener = ITKCommandType::New();
    progressListener->SetCallbackFunction( this,
                                           &PartialVolumeAnalysisHistogramCalculator::UnmaskedStatisticsProgressUpdate );

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
    unsigned long observerTag = statisticsFilter->AddObserver(
        itk::ProgressEvent(), progressListener );

    statisticsFilter->Update();

    statisticsFilter->RemoveObserver( observerTag );


    this->InvokeEvent( itk::EndEvent() );

    statistics.N = image->GetBufferedRegion().GetNumberOfPixels();
    statistics.Min = statisticsFilter->GetMinimum();
    statistics.Max = statisticsFilter->GetMaximum();
    statistics.Mean = statisticsFilter->GetMean();
    statistics.Median = 0.0;
    statistics.Sigma = statisticsFilter->GetSigma();
    statistics.RMS = sqrt( statistics.Mean * statistics.Mean
                           + statistics.Sigma * statistics.Sigma );

    typename ImageType::Pointer inImage = const_cast<ImageType*>(image);

    // Calculate histogram
    typedef itk::Statistics::ScalarImageToHistogramGenerator< ImageType >
        HistogramGeneratorType;
    typename HistogramGeneratorType::Pointer histogramGenerator = HistogramGeneratorType::New();
    histogramGenerator->SetInput( inImage );
    histogramGenerator->SetMarginalScale( 10 ); // Defines y-margin width of histogram
    histogramGenerator->SetNumberOfBins( m_NumberOfBins ); // CT range [-1024, +2048] --> bin size 4 values
    histogramGenerator->SetHistogramMin(  statistics.Min );
    histogramGenerator->SetHistogramMax(  statistics.Max );
    histogramGenerator->Compute();
    *histogram = histogramGenerator->GetOutput();
  }


  template < typename TPixel, unsigned int VImageDimension >
      void PartialVolumeAnalysisHistogramCalculator::InternalCalculateStatisticsMasked(
          const itk::Image< TPixel, VImageDimension > *image,
          itk::Image< unsigned char, VImageDimension > *maskImage,
          Statistics &,
          typename HistogramType::ConstPointer *histogram )
  {
    MITK_DEBUG << "InternalCalculateStatisticsMasked() start";

    typedef itk::Image< TPixel, VImageDimension > ImageType;
    typedef itk::Image< unsigned char, VImageDimension > MaskImageType;
    typedef typename ImageType::IndexType IndexType;

    // generate a list sample of angles at positions
    // where the fiber-prob is higher than .2*maxprob
    typedef TPixel MeasurementType;
    const unsigned int MeasurementVectorLength = 1;
    typedef itk::Vector< MeasurementType , MeasurementVectorLength >
                                                                 MeasurementVectorType;
    typedef itk::Statistics::ListSample< MeasurementVectorType > ListSampleType;
    typename ListSampleType::Pointer listSample = ListSampleType::New();
    listSample->SetMeasurementVectorSize( MeasurementVectorLength );

    itk::ImageRegionConstIterator<MaskImageType>
        itmask(maskImage, maskImage->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<ImageType>
        itimage(image, image->GetLargestPossibleRegion());

    itmask.GoToBegin();
    itimage.GoToBegin();

    while( !itmask.IsAtEnd() )
    {
      if(itmask.Get() != 0)
      {
        // apend to list
        MeasurementVectorType mv;
        mv[0] = ( MeasurementType ) itimage.Get();
        listSample->PushBack(mv);
      }
      ++itmask;
      ++itimage;
    }

    // generate a histogram from the list sample
    typedef double HistogramMeasurementType;
    typedef itk::Statistics::Histogram< HistogramMeasurementType, itk::Statistics::DenseFrequencyContainer2 > HistogramType;
    typedef itk::Statistics::SampleToHistogramFilter< ListSampleType, HistogramType > GeneratorType;
    typename GeneratorType::Pointer generator = GeneratorType::New();
    typename GeneratorType::HistogramType::SizeType size(MeasurementVectorLength);
    size.Fill(m_NumberOfBins);
    generator->SetHistogramSize( size );
    generator->SetInput( listSample );
    generator->SetMarginalScale( 10.0 );
    generator->Update();
    *histogram = generator->GetOutput();

  }


  template < typename TPixel, unsigned int VImageDimension >
      void PartialVolumeAnalysisHistogramCalculator::InternalCropAdditionalImage(
          itk::Image< TPixel, VImageDimension > *image, int additionalIndex )
  {
    typedef itk::Image< TPixel, VImageDimension > ImageType;
    typedef itk::RegionOfInterestImageFilter< ImageType, ImageType > ROIFilterType;
    typename ROIFilterType::Pointer roi = ROIFilterType::New();
    roi->SetRegionOfInterest(m_CropRegion);
    roi->SetInput(image);
    roi->Update();

    m_InternalAdditionalResamplingImages[additionalIndex] = mitk::Image::New();
    m_InternalAdditionalResamplingImages[additionalIndex]->InitializeByItk(roi->GetOutput());
    m_InternalAdditionalResamplingImages[additionalIndex]->SetVolume(roi->GetOutput()->GetBufferPointer());
  }

  template < typename TPixel, unsigned int VImageDimension >
      void PartialVolumeAnalysisHistogramCalculator::InternalCalculateMaskFromPlanarFigure(
          itk::Image< TPixel, VImageDimension > *image, unsigned int axis )
  {

    MITK_DEBUG << "InternalCalculateMaskFromPlanarFigure() start";

    typedef itk::Image< TPixel, VImageDimension > ImageType;
    typedef itk::CastImageFilter< ImageType, MaskImage3DType > CastFilterType;

    // Generate mask image as new image with same header as input image and
    // initialize with "1".
    MaskImage3DType::Pointer newMaskImage = MaskImage3DType::New();
    newMaskImage->SetSpacing( image->GetSpacing() );   // Set the image spacing
    newMaskImage->SetOrigin( image->GetOrigin() );     // Set the image origin
    newMaskImage->SetDirection( image->GetDirection() );  // Set the image direction
    newMaskImage->SetRegions( image->GetLargestPossibleRegion() );
    newMaskImage->Allocate();
    newMaskImage->FillBuffer( 1 );

    // Generate VTK polygon from (closed) PlanarFigure polyline
    // (The polyline points are shifted by -0.5 in z-direction to make sure
    // that the extrusion filter, which afterwards elevates all points by +0.5
    // in z-direction, creates a 3D object which is cut by the the plane z=0)
    const mitk::PlaneGeometry *planarFigureGeometry2D = m_PlanarFigure->GetPlaneGeometry();
    const typename PlanarFigure::PolyLineType planarFigurePolyline = m_PlanarFigure->GetPolyLine( 0 );
    const mitk::BaseGeometry *imageGeometry3D = m_InternalImage->GetGeometry( 0 );

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
    default:
      i0 = 0;
      i1 = 1;
      break;
    }

    // Create VTK polydata object of polyline contour
    bool outOfBounds = false;
    vtkPoints *points = vtkPoints::New();
    typename PlanarFigure::PolyLineType::const_iterator it;
    for ( it = planarFigurePolyline.begin();
    it != planarFigurePolyline.end();
    ++it )
    {
      Point3D point3D;

      // Convert 2D point back to the local index coordinates of the selected
      // image
      mitk::Point2D point2D = *it;
      planarFigureGeometry2D->WorldToIndex(point2D, point2D);
      point2D[0] -= 0.5/m_UpsamplingFactor;
      point2D[1] -= 0.5/m_UpsamplingFactor;
      planarFigureGeometry2D->IndexToWorld(point2D, point2D);
      planarFigureGeometry2D->Map( point2D, point3D );

      // Polygons (partially) outside of the image bounds can not be processed
      // further due to a bug in vtkPolyDataToImageStencil
      if ( !imageGeometry3D->IsInside( point3D ) )
      {
        outOfBounds = true;
      }

      imageGeometry3D->WorldToIndex( point3D, point3D );
      point3D[i0] += 0.5;
      point3D[i1] += 0.5;

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

    std::size_t numberOfPoints = planarFigurePolyline.size();
    vtkIdType *ptIds = new vtkIdType[numberOfPoints];
    for ( std::size_t i = 0; i < numberOfPoints; ++i )
    {
      ptIds[i] = i;
    }
    polyline->InsertNextCell( VTK_POLY_LINE, numberOfPoints, ptIds );


    // Extrude the generated contour polygon
    vtkLinearExtrusionFilter *extrudeFilter = vtkLinearExtrusionFilter::New();
    extrudeFilter->SetInputData( polyline );
    extrudeFilter->SetScaleFactor( 1 );
    extrudeFilter->SetExtrusionTypeToNormalExtrusion();
    extrudeFilter->SetVector( 0.0, 0.0, 1.0 );

    // Make a stencil from the extruded polygon
    vtkPolyDataToImageStencil *polyDataToImageStencil = vtkPolyDataToImageStencil::New();
    polyDataToImageStencil->SetInputConnection( extrudeFilter->GetOutputPort() );



    // Export from ITK to VTK (to use a VTK filter)
    typedef itk::VTKImageImport< MaskImage3DType > ImageImportType;
    typedef itk::VTKImageExport< MaskImage3DType > ImageExportType;

    typename ImageExportType::Pointer itkExporter = ImageExportType::New();
    itkExporter->SetInput( newMaskImage );

    vtkImageImport *vtkImporter = vtkImageImport::New();
    this->ConnectPipelines( itkExporter, vtkImporter );
    vtkImporter->Update();


    // Apply the generated image stencil to the input image
    vtkImageStencil *imageStencilFilter = vtkImageStencil::New();
    imageStencilFilter->SetInputData( vtkImporter->GetOutput() );
    imageStencilFilter->SetStencilConnection(polyDataToImageStencil->GetOutputPort() );
    imageStencilFilter->ReverseStencilOff();
    imageStencilFilter->SetBackgroundValue( 0 );
    imageStencilFilter->Update();


    // Export from VTK back to ITK
    vtkImageExport *vtkExporter = vtkImageExport::New();
    vtkExporter->SetInputData( imageStencilFilter->GetOutput() );
    vtkExporter->Update();

    typename ImageImportType::Pointer itkImporter = ImageImportType::New();
    this->ConnectPipelines( vtkExporter, itkImporter );
    itkImporter->Update();

    // calculate cropping bounding box
    m_InternalImageMask3D = itkImporter->GetOutput();
    m_InternalImageMask3D->SetDirection(image->GetDirection());

    itk::ImageRegionIterator<MaskImage3DType>
        itmask(m_InternalImageMask3D, m_InternalImageMask3D->GetLargestPossibleRegion());
    itmask.GoToBegin();
    while( !itmask.IsAtEnd() )
    {
      if(itmask.Get() != 0)
      {
        typename ImageType::IndexType index = itmask.GetIndex();
        for(unsigned int thick=0; thick<2*m_PlanarFigureThickness+1; thick++)
        {
          index[axis] = thick;
          m_InternalImageMask3D->SetPixel(index, itmask.Get());
        }
      }
      ++itmask;
    }

    itmask.GoToBegin();
    itk::ImageRegionIterator<ImageType>
        itimage(image, image->GetLargestPossibleRegion());
    itimage.GoToBegin();

    typename ImageType::SizeType lowersize;
    lowersize.Fill(std::numeric_limits<typename ImageType::SizeValueType>::max());
    typename ImageType::SizeType uppersize;
    uppersize.Fill(std::numeric_limits<typename ImageType::SizeValueType>::min());
    while( !itmask.IsAtEnd() )
    {
      if(itmask.Get() == 0)
      {
        itimage.Set(0);
      }
      else
      {
        typename ImageType::IndexType index = itimage.GetIndex();
        typename ImageType::SizeType signedindex;
        signedindex[0] = index[0];
        signedindex[1] = index[1];
        signedindex[2] = index[2];

        lowersize[0] = signedindex[0] < lowersize[0] ? signedindex[0] : lowersize[0];
        lowersize[1] = signedindex[1] < lowersize[1] ? signedindex[1] : lowersize[1];
        lowersize[2] = signedindex[2] < lowersize[2] ? signedindex[2] : lowersize[2];

        uppersize[0] = signedindex[0] > uppersize[0] ? signedindex[0] : uppersize[0];
        uppersize[1] = signedindex[1] > uppersize[1] ? signedindex[1] : uppersize[1];
        uppersize[2] = signedindex[2] > uppersize[2] ? signedindex[2] : uppersize[2];
      }

      ++itmask;
      ++itimage;
    }

    typename ImageType::IndexType index;
    index[0] = lowersize[0];
    index[1] = lowersize[1];
    index[2] = lowersize[2];

    typename ImageType::SizeType size;
    size[0] = uppersize[0] - lowersize[0] + 1;
    size[1] = uppersize[1] - lowersize[1] + 1;
    size[2] = uppersize[2] - lowersize[2] + 1;

    m_CropRegion = itk::ImageRegion<3>(index, size);

    // crop internal image
    typedef itk::RegionOfInterestImageFilter< ImageType, ImageType > ROIFilterType;
    typename ROIFilterType::Pointer roi = ROIFilterType::New();
    roi->SetRegionOfInterest(m_CropRegion);
    roi->SetInput(image);
    roi->Update();

    m_InternalImage = mitk::Image::New();
    m_InternalImage->InitializeByItk(roi->GetOutput());
    m_InternalImage->SetVolume(roi->GetOutput()->GetBufferPointer());

    // crop internal mask
    typedef itk::RegionOfInterestImageFilter< MaskImage3DType, MaskImage3DType > ROIMaskFilterType;
    typename ROIMaskFilterType::Pointer roi2 = ROIMaskFilterType::New();
    roi2->SetRegionOfInterest(m_CropRegion);
    roi2->SetInput(m_InternalImageMask3D);
    roi2->Update();
    m_InternalImageMask3D = roi2->GetOutput();

    // Clean up VTK objects
    polyline->Delete();
    extrudeFilter->Delete();
    polyDataToImageStencil->Delete();
    vtkImporter->Delete();
    imageStencilFilter->Delete();
    //vtkExporter->Delete(); // TODO: crashes when outcommented; memory leak??
    delete[] ptIds;

  }


  void PartialVolumeAnalysisHistogramCalculator::UnmaskedStatisticsProgressUpdate()
  {
    // Need to throw away every second progress event to reach a final count of
    // 100 since two consecutive filters are used in this case
    static int updateCounter = 0;
    if ( updateCounter++ % 2 == 0 )
    {
      this->InvokeEvent( itk::ProgressEvent() );
    }
  }


  void PartialVolumeAnalysisHistogramCalculator::MaskedStatisticsProgressUpdate()
  {
    this->InvokeEvent( itk::ProgressEvent() );
  }


}
