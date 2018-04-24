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

    // TODO: implement setters and getters for all the parameters

  protected:
    PhotoacousticMotionCorrectionFilter();

    ~PhotoacousticMotionCorrectionFilter() override;

    void GenerateData() override;

    //##Description
    //## @brief Time when Header was last initialized
    /* itk::TimeStamp m_TimeOfHeaderInitialization; */

  private:
    // InputData
    mitk::Image::Pointer m_paImage, m_usImage;
    // Parameters
    double m_pyr_scale, m_poly_sigma;
    int m_levels, m_winsize, m_iterations, m_poly_n, m_flags, m_batch;
    /* // Stuff that OpenCV needs */
    cv::Mat m_startImage, m_stopImage, m_flow;
    /* cv::UMat m_startImageG, m_StopImageG, m_uflow; */
    // middle step conversion from MITK::image to cv::Mat
    // TODO: Note that there is always a float conversion inbetween
    itk::Image<float>::Pointer m_itkPaImage, m_itkUsImage;
  };
}
#endif
