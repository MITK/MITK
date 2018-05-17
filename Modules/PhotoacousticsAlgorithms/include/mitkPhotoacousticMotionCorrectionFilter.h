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
  *  TODO: Write down all the parameters needed.
  *  The filter takes a stack of PA and US images. It then computes the optical flow
  *  within the US image and compensates the PA images for the flow. Afterwards it
  *  returns the stack of PA images.
  */
  class MITKPHOTOACOUSTICSALGORITHMS_EXPORT PhotoacousticMotionCorrectionFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(PhotoacousticMotionCorrectionFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);
    /* itkCloneMacro(Self); */

    /* void SetInput(Image::Pointer paImage, Image::Pointer usImage); */

    void SetBatchSize(unsigned int);
    unsigned int GetBatchSize();
    void SetPyrScale(double);
    double GetPyrScale();
    void SetLevels(unsigned int);
    unsigned int GetLevels();
    void SetWindowSize(unsigned int);
    unsigned int GetWindowSize();
    void SetIterations(unsigned int);
    unsigned int GetIterations();
    void SetPolyN(unsigned int);
    unsigned int GetPolyN();
    void SetPolySigma(double);
    double GetPolySigma();
    void SetFlags(unsigned int);
    unsigned int GetFlags();

    // Wrapper for SetInput, GetInput and GetOutput
    void SetPaInput(const mitk::Image::Pointer);
    mitk::Image::Pointer GetPaInput();
    void SetUsInput(const mitk::Image::Pointer);
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
    double m_pyr_scale, m_poly_sigma;
    unsigned int m_levels, m_winsize, m_iterations, m_poly_n, m_flags, m_batch;
    /* // Stuff that OpenCV needs */
    cv::UMat m_UsRef, m_PaMat, m_UsMat, m_Flow;
    cv::Mat m_PaRes, m_UsRes, m_PaMatC, m_UsMatC;
    // middle step conversion from MITK::image to cv::Mat
    // TODO: Note that there is always a float conversion inbetween
    itk::Image<float>::Pointer m_itkPaImage, m_itkUsImage;
    mitk::OpenCVToMitkImageFilter::Pointer m_OpenCVToImageFilter = mitk::OpenCVToMitkImageFilter::New();
    mitk::ImageToOpenCVImageFilter::Pointer m_ImageToOpenCVFilter = mitk::ImageToOpenCVImageFilter::New();
  };
}
#endif
