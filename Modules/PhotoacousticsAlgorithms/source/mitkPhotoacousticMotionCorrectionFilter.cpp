/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "./mitkPhotoacousticMotionCorrectionFilter.h"
#include <mitkImageReadAccessor.h>

mitk::PhotoacousticMotionCorrectionFilter::
    PhotoacousticMotionCorrectionFilter() {
  // Set the defaults for the OpticalFlowFarneback algorithm
  // The values are taken directly out of Thomas's
  // US-CV-Based-Optical-Flow-Carotis.ipyn
  m_BatchSize = 5;
  m_PyrScale = 0.5;
  m_Levels = 1;
  m_WinSize = 40;
  m_Iterations = 2;
  m_PolyN = 7;
  m_PolySigma = 1.5;
  m_Flags = 0;

  m_MaxValue = 255.0;
  m_MinValue = 0.0;

  this->SetNumberOfIndexedInputs(2);
  this->SetNumberOfIndexedOutputs(2);
  this->SetNthOutput(0, mitk::Image::New());
  this->SetNthOutput(1, mitk::Image::New());
}

mitk::PhotoacousticMotionCorrectionFilter::
    ~PhotoacousticMotionCorrectionFilter() {}

void mitk::PhotoacousticMotionCorrectionFilter::SetPaInput(
    mitk::Image::Pointer input) {
  this->SetInput(0, input);
}

mitk::Image::Pointer mitk::PhotoacousticMotionCorrectionFilter::GetPaInput() {
  return this->GetInput(0);
}

void mitk::PhotoacousticMotionCorrectionFilter::SetUsInput(
    mitk::Image::Pointer input) {
  this->SetInput(1, input);
}

mitk::Image::Pointer mitk::PhotoacousticMotionCorrectionFilter::GetUsInput() {
  return this->GetInput(1);
}

mitk::Image::Pointer mitk::PhotoacousticMotionCorrectionFilter::GetPaOutput() {
  return this->GetOutput(0);
}

mitk::Image::Pointer mitk::PhotoacousticMotionCorrectionFilter::GetUsOutput() {
  return this->GetOutput(1);
}

void mitk::PhotoacousticMotionCorrectionFilter::CheckInput(
    mitk::Image::Pointer paImage, mitk::Image::Pointer usImage) {
  // Check that we actually got some images
  if (!paImage || !usImage) {
    MITK_ERROR << "We did not get two images!";
    throw std::invalid_argument("One of the images was NULL.");
  }

  // Check that the image dimensions are the same
  if (paImage->GetDimension() != IMAGE_DIMENSION || usImage->GetDimension() != IMAGE_DIMENSION) {
    MITK_ERROR << "Mismatching image dimensions detected in the motion "
                 "compensation filter.";
    throw std::invalid_argument("Both images must have dimension 3.");
  }

  // Check that each dimension has the same size
  for (unsigned int i = 0; i < paImage->GetDimension(); ++i) {
    if (paImage->GetDimensions()[i] != usImage->GetDimensions()[i]) {
      MITK_ERROR << "Mismatching image dimensions detected in the motion "
                   "compensation filter.";
      throw std::invalid_argument(
          "Both images must have the same length in each dimension.");
    }
  }
}

void mitk::PhotoacousticMotionCorrectionFilter::InitializeOutputIfNecessary(
    mitk::Image::Pointer paInput, mitk::Image::Pointer usInput,
    mitk::Image::Pointer paOutput, mitk::Image::Pointer usOutput) {
  if (paOutput->GetDimension() != IMAGE_DIMENSION) {
    this->InitializeOutput(paInput, paOutput);
    this->InitializeOutput(usInput, usOutput);
  }

  for (unsigned int i = 0; i < usOutput->GetDimension(); ++i) {
    if (usOutput->GetDimensions()[i] != usInput->GetDimensions()[i]) {
      this->InitializeOutput(paInput, paOutput);
      this->InitializeOutput(usInput, usOutput);
      break;
    }
  }
}

void mitk::PhotoacousticMotionCorrectionFilter::InitializeOutput(
    mitk::Image::Pointer input, mitk::Image::Pointer output) {
  output->Initialize(input);
  mitk::ImageReadAccessor accessor(input);
  output->SetImportVolume(accessor.GetData());
}

void mitk::PhotoacousticMotionCorrectionFilter::PerformCorrection(
    mitk::Image::Pointer paInput, mitk::Image::Pointer usInput,
    mitk::Image::Pointer paOutput, mitk::Image::Pointer usOutput) {

  // If batch size was set to 0, use one single batch for the whole data set.
  unsigned int batch;
  if (m_BatchSize == 0) {
    batch = paInput->GetDimensions()[IMAGE_DIMENSION - 1];
  } else {
    batch = m_BatchSize;
  }

  for (unsigned int i = 0; i < paInput->GetDimensions()[IMAGE_DIMENSION - 1]; ++i) {

    // Get the 2d matrix from slice at i
    m_PaMat = this->GetMatrix(paInput, i);
    m_UsMat = this->GetMatrix(usInput, i);

    // At the beginning of a batch we set the new reference image and directly
    // write the result images
    if (i % batch == 0) {
      // Rescale reference to full char resolution
      m_UsRef = this->FitMatrixToChar(m_UsMat);
      m_UsRes = m_UsMat.clone();
      m_PaRes = m_PaMat.clone();
    } else {
      cv::Mat UsMatRescaled = this->FitMatrixToChar(m_UsMat);
      cv::calcOpticalFlowFarneback(m_UsRef, UsMatRescaled, m_Flow, m_PyrScale,
                                   m_Levels, m_WinSize, m_Iterations, m_PolyN,
                                   m_PolySigma, m_Flags);

      m_Map = this->ComputeFlowMap(m_Flow);

      // Apply the flow to the matrices
      cv::remap(m_PaMat, m_PaRes, m_Map, cv::noArray(), cv::INTER_LINEAR);
      cv::remap(m_UsMat, m_UsRes, m_Map, cv::noArray(), cv::INTER_LINEAR);
    }

    // Enter the matrix as a slice at position i into output
    this->InsertMatrixAsSlice(m_PaRes, paOutput, i);
    this->InsertMatrixAsSlice(m_UsRes, usOutput, i);
  }
}

// Copied from https://stackoverflow.com/questions/17459584/opencv-warping-image-based-on-calcopticalflowfarneback
cv::Mat mitk::PhotoacousticMotionCorrectionFilter::ComputeFlowMap(cv::Mat flow) {
  cv::Mat map(flow.size(), flow.type());

  for (int y = 0; y < map.rows; ++y) {
    for(int x = 0; x < map.cols; ++x) {
      cv::Point2f f = flow.at<cv::Point2f>(y,x);
      map.at<cv::Point2f>(y,x) = cv::Point2f(x + f.x, y + f.y);
    }
  }

  return map;
}

cv::Mat mitk::PhotoacousticMotionCorrectionFilter::FitMatrixToChar(cv::Mat mat) {

  if (m_MaxValue == m_MinValue) {

    return mat.clone();
  }

  return MAX_MATRIX*(mat.clone() - m_MinValue) / (m_MaxValue - m_MinValue);
}

cv::Mat mitk::PhotoacousticMotionCorrectionFilter::GetMatrix(
    const mitk::Image::Pointer input, unsigned int i) {

  // Access slice i of image input
  mitk::ImageReadAccessor accessor(input, input->GetSliceData(i));
  mitk::Image::Pointer slice = mitk::Image::New();
  slice->Initialize(input->GetPixelType(), IMAGE_DIMENSION - 1, input->GetDimensions());
  slice->SetVolume(accessor.GetData());

  // Transform the slice to matrix
  m_ImageToOpenCVFilter->SetImage(slice);
  return m_ImageToOpenCVFilter->GetOpenCVMat();
}

void mitk::PhotoacousticMotionCorrectionFilter::InsertMatrixAsSlice(
    cv::Mat mat, mitk::Image::Pointer output, unsigned int i) {

  m_OpenCVToImageFilter->SetOpenCVMat(mat);
  m_OpenCVToImageFilter->Update();
  mitk::Image::Pointer slice = m_OpenCVToImageFilter->GetOutput();

  mitk::ImageReadAccessor accessor(slice);
  output->SetSlice(accessor.GetData(), i);
}

// TODO: remove debug messages
void mitk::PhotoacousticMotionCorrectionFilter::GenerateData() {
  MITK_INFO << "Start motion compensation.";

  mitk::Image::Pointer paInput = this->GetInput(0);
  mitk::Image::Pointer usInput = this->GetInput(1);
  mitk::Image::Pointer paOutput = this->GetOutput(0);
  mitk::Image::Pointer usOutput = this->GetOutput(1);

  // Check that we have two input images with agreeing dimensions
  this->CheckInput(paInput, usInput);

  // Check the output images and (re-)initialize, if necessary.
  this->InitializeOutputIfNecessary(paInput, usInput, paOutput, usOutput);

  // Set Max and Min of ultrasonic image
  this->m_MaxValue = usInput->GetStatistics()->GetScalarValueMax();
  this->m_MinValue = usInput->GetStatistics()->GetScalarValueMin();

  // m_ImageToOpenCVFilter->SetImage(paInput);
  this->PerformCorrection(paInput, usInput, paOutput, usOutput);

  MITK_INFO << "Motion compensation accomplished.";
}
