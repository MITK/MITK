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

#include "./mitkPhotoacousticMotionCorrectionFilter.h"
#include <mitkImageReadAccessor.h>

mitk::PhotoacousticMotionCorrectionFilter::
    PhotoacousticMotionCorrectionFilter() {
  // Set the defaults for the OpticalFlowFarneback algorithm
  // The values are taken directly out of Thomas's
  // US-CV-Based-Optical-Flow-Carotis.ipyn
  m_batch = 5;
  m_pyr_scale = 0.5;
  m_levels = 1;
  m_winsize = 40;
  m_iterations = 2;
  m_poly_n = 7;
  m_poly_sigma = 1.5;
  m_flags = 0;

  SetNumberOfIndexedInputs(2);
  SetNumberOfIndexedOutputs(2);
}

mitk::PhotoacousticMotionCorrectionFilter::
    ~PhotoacousticMotionCorrectionFilter() {}

// void mitk::PhotoacousticMotionCorrectionFilter::SetInput(Image::Pointer
// paImage,
//                                                          Image::Pointer
//                                                          usImage)
// {
//   m_paImage = paImage;
//   m_usImage = usImage;
// }

void mitk::PhotoacousticMotionCorrectionFilter::GenerateData() {
  MITK_INFO << "Start motion compensation.";
  
  m_paImage = this->GetInput(0);
  m_usImage = this->GetInput(1);

  // Check that we actually got some images
  if (!m_paImage || !m_usImage) {
    // TODO: Throw some error here
  }
  // Check that the image dimensions are the same
  if (m_paImage->GetDimension() != m_usImage->GetDimension() &&
      m_usImage->GetDimension() == 3) {
    MITK_INFO << "Mismatching image dimensions detected in the motion "
                 "compensation filter.";
    // TODO: Throw some error here
  }
  for (unsigned int i = 0; i < m_paImage->GetDimension(); i++) {
    if (m_paImage->GetDimensions()[i] != m_usImage->GetDimensions()[i]) {
      MITK_INFO << "Mismatching image dimensions detected in the motion "
                   "compensation filter.";
      // TODO: Throw some error here
    }
  }

  // Initialize output images
  if (!m_paCompensated) {
    m_paCompensated = mitk::Image::New();
  }
  if (!m_usCompensated) {
    m_usCompensated = mitk::Image::New();
  }
  m_paCompensated->Initialize(m_paImage->GetPixelType(),
                              m_paImage->GetDimension(),
                              m_paImage->GetDimensions());
  m_usCompensated->Initialize(m_usImage->GetPixelType(),
                              m_usImage->GetDimension(),
                              m_usImage->GetDimensions());

  // TODO: remove debug messages

  // Initialize the slices
  mitk::Image::Pointer pa_slice = mitk::Image::New();
  mitk::Image::Pointer us_slice = mitk::Image::New();
  pa_slice->Initialize(m_paImage->GetPixelType(), 2,
                       m_paImage->GetDimensions());
  us_slice->Initialize(m_usImage->GetPixelType(), 2,
                       m_usImage->GetDimensions());

  MITK_INFO << "Start iteration.";
  // Iterate over all the slices
  for (unsigned int i = 0; i < m_paImage->GetDimensions()[2]; i++) {

    // Get a read accessor for each slice
    mitk::ImageReadAccessor pa_accessor(m_paImage, m_paImage->GetSliceData(i));
    mitk::ImageReadAccessor us_accessor(m_paImage, m_usImage->GetSliceData(i));

    // Write the correct image data into the slice
    pa_slice->SetImportVolume(pa_accessor.GetData());
    us_slice->SetImportVolume(us_accessor.GetData());

    // Convert them first to an itk::Image and then to a cv::Mat
    mitk::CastToItkImage(pa_slice, m_itkPaImage);
    mitk::CastToItkImage(us_slice, m_itkUsImage);
    MITK_INFO << "Generate Matrix.";

    m_PaMatC = itk::OpenCVImageBridge::ITKImageToCVMat<itk::Image<float, 2>>(
      m_itkPaImage);
    m_UsMatC = itk::OpenCVImageBridge::ITKImageToCVMat<itk::Image<float, 2>>(
        m_itkUsImage);

    m_PaMat = m_PaMatC.getUMat( cv::ACCESS_READ );
    m_UsMat = m_UsMatC.getUMat( cv::ACCESS_READ );

    MITK_INFO << "Matrix generated.";
    // At the beginning of a batch we set new references and the compensation
    // can be skipped.
    // TODO: handle m_batch == 0
    // if (i % m_batch == 0) {
    //   m_UsRef = m_UsMat;
    //   m_UsRes = m_UsMatC;
    //   m_PaRes = m_PaMatC;
    //   continue;
    // } else {
    //   // Calculate the flow using the Farneback algorithm
    //   // TODO: flags hard coded to 0, rethink.
    //   cv::calcOpticalFlowFarneback(m_UsRef, m_UsMat, m_Flow, m_pyr_scale, m_levels,
    //                                m_winsize, m_iterations, m_poly_n,
    //                                m_poly_sigma, 0);

    //   // Apply flow to the matrices
    //   cv::remap(m_PaMatC, m_PaRes, m_Flow, cv::noArray(), cv::INTER_LINEAR);
    //   cv::remap(m_UsMatC, m_UsRes, m_Flow, cv::noArray(), cv::INTER_LINEAR);
    // }

    // TODO: Actually do something, not just retransform
    m_PaRes = m_PaMatC;
    m_UsRes = m_UsMatC;
    
    m_OpenCVToImageFilter->SetOpenCVMat(m_PaRes);
    m_OpenCVToImageFilter->Update();
    pa_slice = m_OpenCVToImageFilter->GetOutput();
    mitk::ImageReadAccessor pa_slice_accessor(pa_slice);
    m_paCompensated->SetSlice(pa_slice_accessor.GetData(), i);
    m_OpenCVToImageFilter->SetOpenCVMat(m_UsRes);
    m_OpenCVToImageFilter->Update();
    us_slice = m_OpenCVToImageFilter->GetOutput();
    mitk::ImageReadAccessor us_slice_accessor(us_slice);
    m_paCompensated->SetSlice(us_slice_accessor.GetData(), i);
}

  this->SetNthOutput(1, m_usCompensated);
  this->SetNthOutput(0, m_paCompensated);
  MITK_INFO << "We succeeded in running through the whole thing!";
}
