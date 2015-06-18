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
#ifndef __mitkToFCompositeFilter_h
#define __mitkToFCompositeFilter_h

#include <mitkImage.h>
#include "mitkImageToImageFilter.h"
#include <MitkToFProcessingExports.h>
#include <cv.h>
#include <itkBilateralImageFilter.h>

typedef itk::Image<float, 2> ItkImageType2D;
typedef itk::Image<float, 3> ItkImageType3D;
typedef itk::BilateralImageFilter<ItkImageType2D,ItkImageType2D> BilateralFilterType;

namespace mitk
{
  /**
  * @brief Applies a common filter-pipeline to the first input of this filter
  *
  * This class intends to allow quick preprocessing of (ToF) range data. Input 0 of this filter, holding the range image,
  * is processed using the following image processing filters:
  * - threshold filter
  * - mask segmentation
  * - temporal median filter
  * - spatial median filter
  * - bilateral filter
  *
  * @ingroup ToFProcessing
  */
  class MITKTOFPROCESSING_EXPORT ToFCompositeFilter : public ImageToImageFilter
  {
  public:

    mitkClassMacro( ToFCompositeFilter , ImageToImageFilter );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    itkSetMacro(SegmentationMask,mitk::Image::Pointer);
    itkSetMacro(ApplyTemporalMedianFilter,bool);
    itkGetConstMacro(ApplyTemporalMedianFilter,bool);
    itkSetMacro(ApplyAverageFilter,bool);
    itkGetConstMacro(ApplyAverageFilter,bool);
    itkSetMacro(ApplyMedianFilter,bool);
    itkGetConstMacro(ApplyMedianFilter,bool);
    itkSetMacro(ApplyThresholdFilter,bool);
    itkGetConstMacro(ApplyThresholdFilter,bool);
    itkSetMacro(ApplyMaskSegmentation,bool);
    itkGetConstMacro(ApplyMaskSegmentation,bool);
    itkSetMacro(ApplyBilateralFilter,bool);
    itkGetConstMacro(ApplyBilateralFilter,bool);

     /*!
    \brief sets the input of this filter
    \param distanceImage input is the distance image of e.g. a ToF camera
    */
    virtual void SetInput( Image* distanceImage);

    /*!
    \brief sets the input of this filter at idx
    \param idx number of the current input
    \param distanceImage input is the distance image of e.g. a ToF camera
    */
    virtual void SetInput(unsigned int idx,  Image* distanceImage);

    /*!
    \brief returns the input of this filter
    */
    Image* GetInput();

    /*!
    \brief returns the input with id idx of this filter
    */
    Image* GetInput(unsigned int idx);
    /*!
    \brief Sets the parameter of the temporal median filter
    \param tmporalMedianFilterNumOfFrames number of frames to be considered for calulating the temporal median
    */
    void SetTemporalMedianFilterParameter(int tmporalMedianFilterNumOfFrames);
    /*!
    \brief Sets the parameters (lower, upper threshold) of the threshold filter
    \param min lower threshold of the threshold filter
    \param max upper threshold of the threshold filter
    */
    void SetThresholdFilterParameter(int min, int max);
    /*!
    \brief Sets the parameters (domain sigma, range sigma, kernel radius) of the bilateral filter
    \param domainSigma Parameter controlling the smoothing effect of the bilateral filter. Default value: 2
    \param rangeSigma Parameter controlling the edge preserving effect of the bilateral filter. Default value: 60
    \param kernelRadius radius of the filter mask of the bilateral filter
    */
    void SetBilateralFilterParameter(double domainSigma, double rangeSigma, int kernelRadius);

  protected:
    /*!
    \brief standard constructor
    */
    ToFCompositeFilter();
    /*!
    \brief standard destructor
    */
    ~ToFCompositeFilter();
    virtual void GenerateOutputInformation() override;
    /*!
    \brief method generating the output of this filter. Called in the updated process of the pipeline.
    This method generates the output of the ToFSurfaceSource: The generated surface of the 3d points
    */
    virtual void GenerateData() override;
    /**
    * \brief Create an output for each input
    *
    * This Method sets the number of outputs to the number of inputs
    * and creates missing outputs objects.
    * \warning any additional outputs that exist before the method is called are deleted
    */
    void CreateOutputsForAllInputs();
    /*!
    \brief Applies a mask and/or threshold segmentation to the input image.
    All pixels with values outside the mask, below the lower threshold (min) and above the upper threshold (max)
    are assigned the pixel value 0
    */
    void ProcessSegmentation(IplImage* inputIplImage);
    /*!
    \brief Applies the ITK bilateral filter to the input image
    See http://www.itk.org/Doxygen320/html/classitk_1_1BilateralImageFilter.html for more details.
    */
    ItkImageType2D::Pointer ProcessItkBilateralFilter(ItkImageType2D::Pointer inputItkImage);
    /*!
    \brief Applies the OpenCV bilateral filter to the input image.
    See http://opencv.willowgarage.com/documentation/c/image_filtering.html#smooth for more details
    */
    void ProcessCVBilateralFilter(IplImage* inputIplImage, IplImage* outputIplImage);
    /*!
    \brief Applies the OpenCV median filter to the input image.
    See http://opencv.willowgarage.com/documentation/c/image_filtering.html#smooth for more details
    */
    void ProcessCVMedianFilter(IplImage* inputIplImage, IplImage* outputIplImage, int radius = 3);
    /*!
    \brief Performs temporal median filter on an image given the number of frames to be considered
    */
    void ProcessStreamedQuickSelectMedianImageFilter(IplImage* inputIplImage);
    /*!
    \brief Quickselect algorithm
    * This Quickselect routine is based on the algorithm described in
    * "Numerical recipes in C", Second Edition,
    * Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
    * This code by Nicolas Devillard - 1998. Public domain.
    */
    float quick_select(float arr[], int n);
    /*!
    \brief Initialize and allocate a 2D ITK image of dimension m_ImageWidth*m_ImageHeight
    */
    void CreateItkImage(ItkImageType2D::Pointer &itkInputImage);

    mitk::Image::Pointer m_SegmentationMask; ///< mask image used for segmenting the image

    int m_ImageWidth; ///< x-dimension of the image
    int m_ImageHeight; ///< y-dimension of the image
    int m_ImageSize; ///< size of the image in bytes

    IplImage* m_IplDistanceImage; ///< OpenCV-representation of the distance image
    IplImage* m_IplOutputImage; ///< OpenCV-representation of the output image

    ItkImageType2D::Pointer m_ItkInputImage; ///< ITK representation of the distance image

    bool m_ApplyTemporalMedianFilter; ///< Flag indicating if the temporal median filter is currently active for processing the distance image
    bool m_ApplyAverageFilter; ///< Flag indicating if the average filter is currently active for processing the distance image
    bool m_ApplyMedianFilter; ///< Flag indicating if the spatial median filter is currently active for processing the distance image
    bool m_ApplyThresholdFilter; ///< Flag indicating if the threshold filter is currently active for processing the distance image
    bool m_ApplyMaskSegmentation; ///< Flag indicating if a mask segmentation is performed
    bool m_ApplyBilateralFilter; ///< Flag indicating if the bilateral filter is currently active for processing the distance image

    float** m_DataBuffer; ///< Buffer used for calculating the pixel-wise median over the last n (m_TemporalMedianFilterNumOfFrames) number of frames
    int m_DataBufferCurrentIndex; ///< Current index in the buffer of the temporal median filter
    int m_DataBufferMaxSize; ///< Maximal size for the buffer of the temporal median filter (m_DataBuffer)

    int m_TemporalMedianFilterNumOfFrames; ///< Number of frames to be used in the calculation of the temporal median
    int m_ThresholdFilterMin; ///< Lower threshold of the threshold filter. Pixels with values below will be assigned value 0 when applying the threshold filter
    int m_ThresholdFilterMax; ///< Lower threshold of the threshold filter. Pixels with values above will be assigned value 0 when applying the threshold filter
    double m_BilateralFilterDomainSigma; ///< Parameter of the bilateral filter controlling the smoothing effect of the filter. Default value: 2
    double m_BilateralFilterRangeSigma; ///< Parameter of the bilateral filter controlling the edge preserving effect of the filter. Default value: 60
    int m_BilateralFilterKernelRadius; ///< Kernel radius of the bilateral filter mask

  };
} //END mitk namespace
#endif
