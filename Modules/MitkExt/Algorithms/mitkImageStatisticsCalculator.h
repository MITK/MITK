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


#ifndef _MITK_IMAGESTATISTICSCALCULATOR_H
#define _MITK_IMAGESTATISTICSCALCULATOR_H

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
 * of the image (transversal, sagittal, coronal). Planar figures on arbitrary
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
class MITKEXT_CORE_EXPORT ImageStatisticsCalculator : public itk::Object
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

  
  mitkClassMacro( ImageStatisticsCalculator, itk::Object );
  itkNewMacro( ImageStatisticsCalculator );

  /** \brief Set image from which to compute statistics. */
  void SetImage( const mitk::Image *image );

  /** \brief Set binary image for masking. */
  void SetImageMask( const mitk::Image *imageMask );

  /** \brief Set planar figure for masking. */
  void SetPlanarFigure( const mitk::PlanarFigure *planarFigure );


  /** \brief Set/Get operation mode for masking */
  itkSetMacro( MaskingMode, unsigned int );

  /** \brief Set/Get operation mode for masking */
  itkGetMacro( MaskingMode, unsigned int );

  /** \brief Set/Get operation mode for masking */
  void SetMaskingModeToNone();

  /** \brief Set/Get operation mode for masking */
  void SetMaskingModeToImage();

  /** \brief Set/Get operation mode for masking */
  void SetMaskingModeToPlanarFigure();



  /** \brief Compute statistics (together with histogram) for the current
   * masking mode.
   *
   * Computation is not executed if statistics is already up to date. In this
   * case, false is returned; otherwise, true.*/
  virtual bool ComputeStatistics( unsigned int timeStep = 0 );


  /** \brief Retrieve the histogram depending on the current masking mode. */
  const HistogramType *GetHistogram(  unsigned int timeStep = 0 ) const;

  /** \brief Retrieve statistics depending on the current masking mode. */
  const Statistics &GetStatistics( unsigned int timeStep = 0 ) const;


  
protected:
  typedef std::vector< HistogramType::ConstPointer > HistogramVectorType;
  typedef std::vector< Statistics > StatisticsVectorType;

  typedef std::vector< itk::TimeStamp > TimeStampVectorType;
  typedef std::vector< bool > BoolVectorType;



  typedef itk::Image< unsigned short, 3 > MaskImage3DType;
  typedef itk::Image< unsigned short, 2 > MaskImage2DType;

  ImageStatisticsCalculator();
  
  virtual ~ImageStatisticsCalculator();

  /** \brief Depending on the masking mode, the image and mask from which to
   * calculate statistics is extracted from the original input image and mask
   * data.
   *
   * For example, a when using a PlanarFigure as mask, the 2D image slice
   * corresponding to the PlanarFigure will be extracted from the original
   * image. If masking is disabled, the original image is simply passed
   * through. */
  void ExtractImageAndMask( unsigned int timeStep = 0 );


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
    itk::Image< unsigned short, VImageDimension > *maskImage,
    Statistics &statistics,
    typename HistogramType::ConstPointer *histogram );

  template < typename TPixel, unsigned int VImageDimension >
  void InternalCalculateMaskFromPlanarFigure(
    const itk::Image< TPixel, VImageDimension > *image, unsigned int axis );


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



  const mitk::Image *m_Image;

  const mitk::Image *m_ImageMask;

  mitk::PlanarFigure::ConstPointer m_PlanarFigure;

  HistogramVectorType m_ImageHistogramVector;
  HistogramVectorType m_MaskedImageHistogramVector;
  HistogramVectorType m_PlanarFigureHistogramVector;

  HistogramType::Pointer m_EmptyHistogram;


  StatisticsVectorType m_ImageStatisticsVector;
  StatisticsVectorType m_MaskedImageStatisticsVector;
  StatisticsVectorType m_PlanarFigureStatisticsVector;

  Statistics m_EmptyStatistics;

  unsigned int m_MaskingMode;

  
  mitk::Image::ConstPointer m_InternalImage;
  MaskImage3DType::Pointer m_InternalImageMask3D;
  MaskImage2DType::Pointer m_InternalImageMask2D;

  TimeStampVectorType m_ImageStatisticsTimeStampVector;
  TimeStampVectorType m_MaskedImageStatisticsTimeStampVector;
  TimeStampVectorType m_PlanarFigureStatisticsTimeStampVector;

  BoolVectorType m_ImageStatisticsCalculationTriggerVector;
  BoolVectorType m_MaskedImageStatisticsCalculationTriggerVector;
  BoolVectorType m_PlanarFigureStatisticsCalculationTriggerVector;

};

}

#endif // #define _MITK_IMAGESTATISTICSCALCULATOR_H
