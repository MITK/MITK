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

mitk::PhotoacousticMotionCorrectionFilter::PhotoacousticMotionCorrectionFilter()
{
  // Set the defaults for the OpticalFlowFarneback algorithm
  // The values are taken directly out of Thomas's US-CV-Based-Optical-Flow-Carotis.ipyn
  m_batch = 5;
  m_pyr_scale = 0.5;
  m_levels = 1;
  m_winsize = 40;
  m_iterations = 2;
  m_poly_n = 7;
  m_poly_sigma = 1.5;
  m_flags = 0;

  SetNumberOfIndexedInputs(2);
  SetNumberOfIndexedOutputs(1);
}

mitk::PhotoacousticMotionCorrectionFilter::~PhotoacousticMotionCorrectionFilter()
{
}

// void mitk::PhotoacousticMotionCorrectionFilter::SetInput(Image::Pointer paImage,
//                                                          Image::Pointer usImage)
// {
//   m_paImage = paImage;
//   m_usImage = usImage;
// }

void mitk::PhotoacousticMotionCorrectionFilter::GenerateData()
{
  auto input1 = this->GetInput(0);
  auto input2 = this->GetInput(1);

  if(input1 && input2)
  {
    m_paImage = input1;
    m_usImage = input2;

    mitk::CastToItkImage(m_paImage, m_itkPaImage);
    // TODO: Make sure that only 2d images enter here
    m_startImage = itk::OpenCVImageBridge::ITKImageToCVMat< itk::Image<float, 2> >(m_itkPaImage);
  }
}
