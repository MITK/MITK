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

#ifndef _MITKPHOTOACOUSTICSMOTIONCORRECTIONFILTER_H_
#define _MITKPHOTOACOUSTICSMOTIONCORRECTIONFILTER_H_

#include "mitkImageToImageFilter.h"
#include <itkObject.h>

#include "opencv2/imgproc.hpp"
// TODO: Find out why build fails with this option or replace with something else
/* #include "opencv2/opencv.hpp" */
#include "opencv2/video/tracking.hpp"

#include "itkOpenCVImageBridge.h"

#include <MitkPhotoacousticsAlgorithmsExports.h>

#include "mitkImageCast.h"
#include <mitkOpenCVToMitkImageFilter.h>
#include <mitkImageToOpenCVImageFilter.h>

namespace mitk
{
  /*!
  * \brief Class implementing a mitk::ImageToImageFilter for PAUS motion correction.
  *
  *  The filter takes a stack of PA and US images. It then computes the optical flow
  *  within the US image and compensates the PA and US images for the flow. Afterwards it
  *  returns the stack of PA and US images.
  *
  * @param m_Batch Determines how many slices belong together and will be motion compensated with regard to the first image in the batch. If the variable is set to 0, the whole time series will be processed as one batch.
  * @param m_PyrScale See @c pyr_scale in @c cv::calcOpticalFlowFarneback
  * @param m_Levels See @c levels in @c cv::calcOpticalFlowFarneback
  * @param m_WinSize See @c winsize in @c cv::calcOpticalFlowFarneback
  * @param m_Iterations See @c iterations in @c cv::calcOpticalFlowFarneback
  * @param m_PolyN See @c poly_n in @c cv::calcOpticalFlowFarneback
  * @param m_PolySigma See @c poly_sigma in @c cv::calcOpticalFlowFarneback
  * @param m_Flags See @c flags in @c cv::calcOpticalFlowFarneback
  * @see https://docs.opencv.org/3.0-beta/modules/video/doc/motion_analysis_and_object_tracking.html#calcopticalflowfarneback
  */
  class MITKPHOTOACOUSTICSALGORITHMS_EXPORT PhotoacousticMotionCorrectionFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(PhotoacousticMotionCorrectionFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);

    // Setters and Getters for the class variables
    itkSetMacro(Batch, unsigned int);
    itkSetMacro(PyrScale, double);
    itkSetMacro(Levels, unsigned int);
    itkSetMacro(WinSize, unsigned int);
    itkSetMacro(Iterations, unsigned int);
    itkSetMacro(PolyN, unsigned int);
    itkSetMacro(PolySigma, double);
    itkSetMacro(Flags, unsigned int);
    itkGetMacro(Batch, unsigned int);
    itkGetMacro(PyrScale, double);
    itkGetMacro(Levels, unsigned int);
    itkGetMacro(WinSize, unsigned int);
    itkGetMacro(Iterations, unsigned int);
    itkGetMacro(PolyN, unsigned int);
    itkGetMacro(PolySigma, double);
    itkGetMacro(Flags, unsigned int);

    // Wrapper for SetInput, GetInput and GetOutput
    void SetPaInput(mitk::Image::Pointer);
    mitk::Image::Pointer GetPaInput();
    void SetUsInput(mitk::Image::Pointer);
    mitk::Image::Pointer GetUsInput();
    mitk::Image::Pointer GetPaOutput();
    mitk::Image::Pointer GetUsOutput();

  protected:
    PhotoacousticMotionCorrectionFilter();

    ~PhotoacousticMotionCorrectionFilter() override;

    void GenerateData() override;
    void CheckInput(mitk::Image::Pointer paImage, mitk::Image::Pointer usImage);
    void InitializeOutput(mitk::Image::Pointer paInput, mitk::Image::Pointer usInput, mitk::Image::Pointer paOutput, mitk::Image::Pointer usOutput);
    void SetOutputData(mitk::Image::Pointer input, mitk::Image::Pointer output);
    void PerformCorrection(mitk::Image::Pointer paInput, mitk::Image::Pointer usInput, mitk::Image::Pointer paOutput, mitk::Image::Pointer usOutput);
    cv::Mat GetMatrix(const mitk::Image::Pointer input, unsigned int i);
    void EnterMatrixInPosition(cv::Mat mat, mitk::Image::Pointer output, unsigned int i);
     //##Description
    //## @brief Time when Header was last initialized
    /* itk::TimeStamp m_TimeOfHeaderInitialization; */

  private:
    // InputData
    /* mitk::Image::Pointer m_paImage, m_usImage, m_paCompensated, m_usCompensated; */
    // Parameters
    double m_PyrScale, m_PolySigma;
    unsigned int m_Levels, m_WinSize, m_Iterations, m_PolyN, m_Flags, m_Batch;
    /* // Stuff that OpenCV needs */
    cv::UMat m_UsRef, m_PaMat, m_UsMat, m_Flow;
    cv::Mat m_PaRes, m_UsRes, m_PaMatC, m_UsMatC;
    // middle step conversion from MITK::image to cv::Mat
    // TODO: Note that there is always a float conversion inbetween
    mitk::OpenCVToMitkImageFilter::Pointer m_OpenCVToImageFilter = mitk::OpenCVToMitkImageFilter::New();
    mitk::ImageToOpenCVImageFilter::Pointer m_ImageToOpenCVFilter = mitk::ImageToOpenCVImageFilter::New();
  };
}
#endif
