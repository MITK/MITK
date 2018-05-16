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

  this->SetNumberOfIndexedInputs(2);
  this->SetNumberOfIndexedOutputs(2);
  this->SetNthOutput(0, mitk::Image::New());
  this->SetNthOutput(1, mitk::Image::New());
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

// TODO: Find out how to throw the right errors
void mitk::PhotoacousticMotionCorrectionFilter::CheckInput(mitk::Image::Pointer paImage, mitk::Image::Pointer usImage) {
  // Check that we actually got some images
  if (!paImage || !usImage) {
    // TODO: Throw some error here
    MITK_INFO << "We did not get two images!";
  }

  // Check that the image dimensions are the same
  if (paImage->GetDimension() != usImage->GetDimension() &&
      usImage->GetDimension() == 3) {
    MITK_INFO << "Mismatching image dimensions detected in the motion "
      "compensation filter.";
    // TODO: Throw some error here
  }

  // Check that each dimension has the same size
  for (unsigned int i = 0; i < paImage->GetDimension(); i++) {
    if (paImage->GetDimensions()[i] != usImage->GetDimensions()[i]) {
      MITK_INFO << "Mismatching image dimensions detected in the motion "
        "compensation filter.";
      // TODO: Throw some error here
    }
  }

}

void mitk::PhotoacousticMotionCorrectionFilter::InitializeOutput(mitk::Image::Pointer paInput, mitk::Image::Pointer usInput, mitk::Image::Pointer paOutput, mitk::Image::Pointer usOutput) {
  if (paOutput->GetDimension() != 3) {
    MITK_INFO << "I jump in here.";
    this->SetOutputData(paInput, paOutput);
    this->SetOutputData(usInput, usOutput);
  }

  for (unsigned int i = 0; i < usOutput->GetDimension(); i++) {
    if (usOutput->GetDimensions()[i] != usInput->GetDimensions()[i]) {
      this->SetOutputData(paInput, paOutput);
      this->SetOutputData(usInput, usOutput);
      break;
   }
  }
}

void mitk::PhotoacousticMotionCorrectionFilter::SetOutputData(mitk::Image::Pointer input, mitk::Image::Pointer output) {
  output->Initialize(input);
  mitk::ImageReadAccessor accessor(input);
  output->SetImportVolume(accessor.GetData());
}

void mitk::PhotoacousticMotionCorrectionFilter::GenerateData() {
  MITK_INFO << "Start motion compensation.";

  auto paInput = this->GetInput(0);
  auto usInput = this->GetInput(1);
  auto paOutput = this->GetOutput(0);
  auto usOutput = this->GetOutput(1);

  // Check that we have two images with agreeing dimensions
  this->CheckInput(paInput, usInput);

  // Check the output images and (re-)initialize, if necessary.
  this->InitializeOutput(paInput, usInput, paOutput, usOutput);

  // MITK_INFO << "Output: " << paOutput->GetPixelType(0);
  // TODO: remove debug messages

  // MITK_INFO << "Input: " << usInput;
  // MITK_INFO << "Output: " << usOutput;
  // MITK_INFO << "Output2: " << paOutput;
  MITK_INFO << "We succeeded in running through the whole thing!";
}
