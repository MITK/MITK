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


#ifndef _MITK_PartialVolumeAnalysisHistogramCalculator_H
#define _MITK_PartialVolumeAnalysisHistogramCalculator_H

#include "MitkDiffusionCoreExports.h"

#include <itkObject.h>
#include <itkImage.h>
#include <itkTimeStamp.h>


#include "mitkImage.h"
#include "mitkImageTimeSelector.h"
#include "mitkPlanarFigure.h"



namespace mitk
{

  /**
 * \brief Class for calculating statistics and histogram for an (optionally
 * masked) image.
 *
 * Images can be masked by either a (binary) image (of the same dimensions as
 * the original image) or by a closed mitk::PlanarFigure, e.g. a circle or
 * polygon. When masking with a planar figure, the slice corresponding to the
 * plane containing the figure is extracted and then clipped with contour
 * defined by the figure. Planar figures need to be aligned along the main axes
 * of the image (axial, sagittal, coronal). Planar figures on arbitrary
 * rotated planes are not supported.
 *
 * For each operating mode (no masking, masking by image, masking by planar
 * figure), the calculated statistics and histogram are cached so that, when
 * switching back and forth between operation modes without modifying mask or
 * image, the information doesn't need to be recalculated.
 *
 * Note: currently time-resolved and multi-channel pictures are not properly
 * supported.
 */
  class MitkDiffusionCore_EXPORT PartialVolumeAnalysisHistogramCalculator : public itk::Object
  {
  public:

    enum
    {
      MASKING_MODE_NONE = 0,
      MASKING_MODE_IMAGE,
      MASKING_MODE_PLANARFIGURE
    };

    typedef mitk::Image::HistogramType HistogramType;
    typedef mitk::Image::HistogramType::ConstIterator HistogramConstIteratorType;

    struct Statistics
    {
      unsigned int N;
      double Min;
      double Max;
      double Mean;
      double Median;
      double Variance;
      double Sigma;
      double RMS;

      void Reset()
      {
        N = 0;
        Min = 0.0;
        Max = 0.0;
        Mean = 0.0;
        Median = 0.0;
        Variance = 0.0;
        Sigma = 0.0;
        RMS = 0.0;
      }
    };

    typedef Statistics StatisticsType;

    typedef itk::TimeStamp TimeStampType;
    typedef bool BoolType;

    typedef itk::Image< unsigned char, 3 > MaskImage3DType;
    typedef itk::Image< unsigned char, 2 > MaskImage2DType;

    typedef itk::Image< float, 2 > InternalImage2DType;

    mitkClassMacro( PartialVolumeAnalysisHistogramCalculator, itk::Object )
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** \brief Set image from which to compute statistics. */
    void SetImage( const mitk::Image *image );

    /** \brief Set binary image for masking. */
    void SetImageMask( const mitk::Image *imageMask );

    /** \brief Set planar figure for masking. */
    void SetPlanarFigure( const mitk::PlanarFigure *planarFigure );

    /** \brief Set image for which the same resampling will be applied.
    and available via GetAdditionalResampledImage() */
    void AddAdditionalResamplingImage( const mitk::Image *image );

    /** \brief Set/Get operation mode for masking */
    void SetMaskingMode( unsigned int mode );

    /** \brief Set/Get operation mode for masking */
    itkGetMacro( MaskingMode, unsigned int );

    /** \brief Set/Get operation mode for masking */
    void SetMaskingModeToNone();

    /** \brief Set/Get operation mode for masking */
    void SetMaskingModeToImage();

    /** \brief Set/Get operation mode for masking */
    void SetMaskingModeToPlanarFigure();

    /** \brief Set histogram number of bins. */
    void SetNumberOfBins( unsigned int number )
    {
      if(m_NumberOfBins != number)
      {
        m_NumberOfBins = number;
        SetModified();
      }
    }

    /** \brief Get histogram number of bins. */
    unsigned int GetNumberOfBins( )
    { return m_NumberOfBins; }

    /** \brief Set upsampling factor. */
    void SetUpsamplingFactor( float number )
    {
      if(m_UpsamplingFactor != number)
      {
        m_UpsamplingFactor = number;
        SetModified();
      }
    }

    /** \brief Get upsampling factor. */
    float GetUpsamplingFactor( )
    { return m_UpsamplingFactor; }

    /** \brief Set gaussian sigma. */
    void SetGaussianSigma( float number )
    {
      if(m_GaussianSigma != number)
      {
        m_GaussianSigma = number;
        SetModified();
      }
    }

    /** \brief Get thickness of planar figure. */
    unsigned int GetPlanarFigureThickness( )
    { return m_PlanarFigureThickness; }


    /** \brief Set thickness of planar figure. */
    void SetPlanarFigureThickness( unsigned int number )
    {
      if(m_PlanarFigureThickness != number)
      {
        m_PlanarFigureThickness = number;
        SetModified();
      }
    }

    /** \brief Get histogram number of bins. */
    float GetGaussianSigma( )
    { return m_GaussianSigma; }

    void SetModified();

    /** \brief Compute statistics (together with histogram) for the current
   * masking mode.
   *
   * Computation is not executed if statistics is already up to date. In this
   * case, false is returned; otherwise, true.*/
    virtual bool ComputeStatistics( );


    /** \brief Retrieve the histogram depending on the current masking mode. */
    const HistogramType *GetHistogram(  ) const;

    /** \brief Retrieve statistics depending on the current masking mode. */
    const Statistics &GetStatistics( ) const;

    const Image::Pointer GetInternalImage()
    {
      return m_InternalImage;
    }

    const Image::Pointer GetInternalAdditionalResampledImage(unsigned int i)
    {
      if(i < m_InternalAdditionalResamplingImages.size())
      {
        return m_InternalAdditionalResamplingImages[i];
      }
      else
      {
        return NULL;
      }
    }

    void SetForceUpdate(bool b)
    {
      m_ForceUpdate = b;
    }

  protected:

    PartialVolumeAnalysisHistogramCalculator();

    virtual ~PartialVolumeAnalysisHistogramCalculator();

    /** \brief Depending on the masking mode, the image and mask from which to
   * calculate statistics is extracted from the original input image and mask
   * data.
   *
   * For example, a when using a PlanarFigure as mask, the 2D image slice
   * corresponding to the PlanarFigure will be extracted from the original
   * image. If masking is disabled, the original image is simply passed
   * through. */
    void ExtractImageAndMask( );


    /** \brief If the passed vector matches any of the three principal axes
   * of the passed geometry, the ínteger value corresponding to the axis
   * is set and true is returned. */
    bool GetPrincipalAxis( const Geometry3D *geometry, Vector3D vector,
                           unsigned int &axis );

    template < typename TPixel, unsigned int VImageDimension >
        void InternalCalculateStatisticsUnmasked(
            const itk::Image< TPixel, VImageDimension > *image,
            Statistics &statistics,
            typename HistogramType::ConstPointer *histogram );

    template < typename TPixel, unsigned int VImageDimension >
        void InternalCalculateStatisticsMasked(
            const itk::Image< TPixel, VImageDimension > *image,
            itk::Image< unsigned char, VImageDimension > *maskImage,
            Statistics &statistics,
            typename HistogramType::ConstPointer *histogram );

    template < typename TPixel, unsigned int VImageDimension >
        void InternalCalculateMaskFromPlanarFigure(
            itk::Image< TPixel, VImageDimension > *image, unsigned int axis );

    template < typename TPixel, unsigned int VImageDimension >
        void InternalReorientImagePlane(
          const itk::Image< TPixel, VImageDimension > *image, mitk::BaseGeometry* imggeo, mitk::BaseGeometry* planegeo3D, int additionalIndex );

    template < typename TPixel, unsigned int VImageDimension >
        void InternalResampleImageFromMask(
            const itk::Image< TPixel, VImageDimension > *image, int additionalIndex );

    void InternalResampleImage(
        const MaskImage3DType *image/*, mitk::Geometry3D* imggeo*/ );

    template < typename TPixel, unsigned int VImageDimension >
        void InternalCropAdditionalImage(
            itk::Image< TPixel, VImageDimension > *image, int additionalIndex );

    void InternalMaskImage( mitk::Image *image );

    /** Connection from ITK to VTK */
    template <typename ITK_Exporter, typename VTK_Importer>
        void ConnectPipelines(ITK_Exporter exporter, VTK_Importer* importer)
    {
      importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());

      importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
      importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
      importer->SetSpacingCallback(exporter->GetSpacingCallback());
      importer->SetOriginCallback(exporter->GetOriginCallback());
      importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());

      importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());

      importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
      importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
      importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
      importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
      importer->SetCallbackUserData(exporter->GetCallbackUserData());
    }

    /** Connection from VTK to ITK */
    template <typename VTK_Exporter, typename ITK_Importer>
        void ConnectPipelines(VTK_Exporter* exporter, ITK_Importer importer)
    {
      importer->SetUpdateInformationCallback(exporter->GetUpdateInformationCallback());

      importer->SetPipelineModifiedCallback(exporter->GetPipelineModifiedCallback());
      importer->SetWholeExtentCallback(exporter->GetWholeExtentCallback());
      importer->SetSpacingCallback(exporter->GetSpacingCallback());
      importer->SetOriginCallback(exporter->GetOriginCallback());
      importer->SetScalarTypeCallback(exporter->GetScalarTypeCallback());

      importer->SetNumberOfComponentsCallback(exporter->GetNumberOfComponentsCallback());

      importer->SetPropagateUpdateExtentCallback(exporter->GetPropagateUpdateExtentCallback());
      importer->SetUpdateDataCallback(exporter->GetUpdateDataCallback());
      importer->SetDataExtentCallback(exporter->GetDataExtentCallback());
      importer->SetBufferPointerCallback(exporter->GetBufferPointerCallback());
      importer->SetCallbackUserData(exporter->GetCallbackUserData());
    }

    void UnmaskedStatisticsProgressUpdate();
    void MaskedStatisticsProgressUpdate();

    mitk::Image::ConstPointer m_Image;
    mitk::Image::ConstPointer m_ImageMask;
    mitk::PlanarFigure::ConstPointer m_PlanarFigure;

    HistogramType::ConstPointer m_ImageHistogram;
    HistogramType::ConstPointer m_MaskedImageHistogram;
    HistogramType::ConstPointer m_PlanarFigureHistogram;

    HistogramType::Pointer m_EmptyHistogram;

    StatisticsType m_ImageStatistics;
    StatisticsType m_MaskedImageStatistics;
    StatisticsType m_PlanarFigureStatistics;

    Statistics m_EmptyStatistics;

    unsigned int m_MaskingMode;
    bool m_MaskingModeChanged;

    mitk::Image::Pointer m_InternalImage;
    MaskImage3DType::Pointer m_InternalImageMask3D;
    MaskImage2DType::Pointer m_InternalImageMask2D;
    itk::ImageRegion<3> m_InternalMask3D;
    std::vector<mitk::Image::ConstPointer> m_AdditionalResamplingImages;
    std::vector<mitk::Image::Pointer> m_InternalAdditionalResamplingImages;

    TimeStampType m_ImageStatisticsTimeStamp;
    TimeStampType m_MaskedImageStatisticsTimeStamp;
    TimeStampType m_PlanarFigureStatisticsTimeStamp;

    BoolType m_ImageStatisticsCalculationTriggerBool;
    BoolType m_MaskedImageStatisticsCalculationTriggerBool;
    BoolType m_PlanarFigureStatisticsCalculationTriggerBool;

    unsigned int m_NumberOfBins;

    float m_UpsamplingFactor;

    float m_GaussianSigma;

    itk::ImageRegion<3> m_CropRegion;

    bool m_ForceUpdate;

    unsigned int m_PlanarFigureThickness;

  };

}

#endif // #define _MITK_PartialVolumeAnalysisHistogramCalculator_H
