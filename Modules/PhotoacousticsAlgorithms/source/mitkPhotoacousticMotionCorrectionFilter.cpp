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
  m_Batch = 5;
  m_PyrScale = 0.5;
  m_Levels = 1;
  m_WinSize = 40;
  m_Iterations = 2;
  m_PolyN = 7;
  m_PolySigma = 1.5;
  m_Flags = 0;

  this->SetNumberOfIndexedInputs(2);
  this->SetNumberOfIndexedOutputs(2);
  this->SetNthOutput(0, mitk::Image::New());
  this->SetNthOutput(1, mitk::Image::New());
}

mitk::PhotoacousticMotionCorrectionFilter::
    ~PhotoacousticMotionCorrectionFilter() {}

// Setters and Getters
void mitk::PhotoacousticMotionCorrectionFilter::SetBatchSize(unsigned int batch) {
  m_Batch = batch;
}

unsigned int mitk::PhotoacousticMotionCorrectionFilter::GetBatchSize() {
  return m_Batch;
}

void mitk::PhotoacousticMotionCorrectionFilter::SetPyrScale(double pyr_scale) {
  m_PyrScale = pyr_scale;
}

double mitk::PhotoacousticMotionCorrectionFilter::GetPyrScale() {
  return m_PyrScale;
}

void mitk::PhotoacousticMotionCorrectionFilter::SetLevels(unsigned int levels) {
  m_Levels = levels;
}

unsigned int mitk::PhotoacousticMotionCorrectionFilter::GetLevels() {
  return m_Levels;
}

void mitk::PhotoacousticMotionCorrectionFilter::SetWindowSize(unsigned int winsize) {
  m_WinSize = winsize;
}

unsigned int mitk::PhotoacousticMotionCorrectionFilter::GetWindowSize() {
  return m_WinSize;
}

void mitk::PhotoacousticMotionCorrectionFilter::SetIterations(unsigned int iterations) {
  m_Iterations = iterations;
}

unsigned int mitk::PhotoacousticMotionCorrectionFilter::GetIterations() {
  return m_Iterations;
}

void mitk::PhotoacousticMotionCorrectionFilter::SetPolyN(unsigned int poly_n) {
  m_PolyN = poly_n;
}

unsigned int mitk::PhotoacousticMotionCorrectionFilter::GetPolyN() {
  return m_PolyN;
}

void mitk::PhotoacousticMotionCorrectionFilter::SetPolySigma(double poly_sigma) {
  m_PolySigma = poly_sigma;
}

double mitk::PhotoacousticMotionCorrectionFilter::GetPolySigma() {
  return m_PolySigma;
}

void mitk::PhotoacousticMotionCorrectionFilter::SetFlags(unsigned int flags) {
  m_Flags = flags;
}

unsigned int mitk::PhotoacousticMotionCorrectionFilter::GetFlags() {
  return m_Flags;
}

void mitk::PhotoacousticMotionCorrectionFilter::SetPaInput(const mitk::Image::Pointer input) {
  this->SetInput(0, input);
}

mitk::Image::Pointer mitk::PhotoacousticMotionCorrectionFilter::GetPaInput() {
  return this->GetInput(0);
}

void mitk::PhotoacousticMotionCorrectionFilter::SetUsInput(const mitk::Image::Pointer input) {
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

/*!
 * \brief Validate the input images
 *
 * The input images have to be non-empty, 3d and have to coincide in the length in each dimension. If any of these conditions are violated, the method will throw an @c invalid_argument exception.
 *
 * @param paImage A mitk image
 * @param usImage A mitk image
 * @warning If the two images are not 3d and do not coincide in the length in each dimension, this method will throw an @c invalid_argument exception.
 */
void mitk::PhotoacousticMotionCorrectionFilter::CheckInput(mitk::Image::Pointer paImage, mitk::Image::Pointer usImage) {
  // Check that we actually got some images
  if (!paImage || !usImage) {
    MITK_INFO << "We did not get two images!";
    throw std::invalid_argument("One of the images was NULL.");
  }

  // Check that the image dimensions are the same
  if (paImage->GetDimension() != 3 || usImage->GetDimension() != 3) {
    MITK_INFO << "Mismatching image dimensions detected in the motion "
      "compensation filter.";
    throw std::invalid_argument("Both images must have dimension 3.");
  }

  // Check that each dimension has the same size
  for (unsigned int i = 0; i < paImage->GetDimension(); i++) {
    if (paImage->GetDimensions()[i] != usImage->GetDimensions()[i]) {
      MITK_INFO << "Mismatching image dimensions detected in the motion "
        "compensation filter.";
      throw std::invalid_argument("Both images must have the same length in each dimension.");
    }
  }

}

/*!
 * \brief Assure that the output images have the same dimensions as the input images.
 *
 * The output images need to have the same dimensions as the input images. This will be checked here. If the dimensions do not match, the output will be reinitialized and the image data from the input images will be copied to the output images (in order to make sure that they have a valid data pointer).
 *
 * @param paInput Pointer to the photoacoustic input image
 * @param usInput Pointer to the ultrasonic input image
 * @param paOutput Pointer to the photoacoustic output image
 * @param usOutput Pointer to the ultrasonic output image
 */
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

/*!
 * \brief Copy the image data from the input image to the output image
 *
 * This method copys the image data from @p input to @p output. This method assumes that the dimensions of the two images match and will not test this.
 *
 * @param input A mitk image
 * @param output A mitk image
 */
void mitk::PhotoacousticMotionCorrectionFilter::SetOutputData(mitk::Image::Pointer input, mitk::Image::Pointer output) {
  output->Initialize(input);
  mitk::ImageReadAccessor accessor(input);
  output->SetImportVolume(accessor.GetData());
}

/*!
 * \brief This method performs the actual motion compensation.
 *
 * This method uses the ultrasonic input image @p usInput to compute the optical flow in the time series of 2d images. Then it compensates both the @p usInput and @p paInput for it and saves the result in @p usOutput and @p paOutput respectively. In the background the OpenCV Farneback algorithm is used for the flow determination.
 *
 * @param paInput The photoacoustic input image
 * @param usInput The ultrasonic input image
 * @param paOutput The photoacoustic output image
 * @param usOutput The ultrasonic output image
 */
void mitk::PhotoacousticMotionCorrectionFilter::PerformCorrection(mitk::Image::Pointer paInput, mitk::Image::Pointer usInput, mitk::Image::Pointer paOutput, mitk::Image::Pointer usOutput) {

  for(unsigned int i = 0; i < paInput->GetDimensions()[2]; i++) {

    // Get the 2d matrix from slice at i
    m_PaMatC = this->GetMatrix(paInput, i);
    m_UsMatC = this->GetMatrix(usInput, i);

    // Transform to cv::UMat
    m_PaMat = m_PaMatC.getUMat( cv::ACCESS_READ ).clone();
    m_UsMat = m_UsMatC.getUMat( cv::ACCESS_READ ).clone();

    // If batch size was set to 0, use one single batch for the whole data set.
    unsigned int batch;
    if (m_Batch == 0) {
      batch = paInput->GetDimensions()[2];
    } else {
      batch = m_Batch;
    }

    // At the beginning of a batch we set the new reference image and directly write the result images
    
    if (i % batch == 0) {
      m_UsRef = m_UsMat.clone();
      m_UsRes = m_UsMatC.clone();
      m_PaRes = m_PaMatC.clone();
    } else {
      cv::calcOpticalFlowFarneback(m_UsRef, m_UsMat, m_Flow, m_PyrScale, m_Levels,
                                   m_WinSize, m_Iterations, m_PolyN,
                                   m_PolySigma, m_Flags);

      // Apply the flow to the matrices
      cv::remap(m_PaMatC, m_PaRes, m_Flow, cv::noArray(), cv::INTER_LINEAR);
      cv::remap(m_UsMatC, m_UsRes, m_Flow, cv::noArray(), cv::INTER_LINEAR);
    }

    // Enter the matrix as a slice at position i into output
    this->EnterMatrixInPosition(m_PaRes, paOutput, i);
    this->EnterMatrixInPosition(m_UsRes, usOutput, i);
  }
}

/*!
 * \brief Extract a 2d slice as OpenCV matrix.
 *
 * This method extracts slice @p i from the 3-dimensional image @p input and converts it to a OpenCV matrix. Internally, the mitkImageToOpenCVImageFilter is used.
 * 
 * @param input A 3d image from which a slice is extracted as a 2d OpenCV matrix.
 * @param i Determines the slice to be extracted.
 * @return returns a OpenCV matrix containing the 2d slice.
 */

cv::Mat mitk::PhotoacousticMotionCorrectionFilter::GetMatrix(const mitk::Image::Pointer input, unsigned int i) {

  // Access slice i of image input
  mitk::ImageReadAccessor accessor(input, input->GetSliceData(i));
  mitk::Image::Pointer slice = mitk::Image::New();
  slice->Initialize(input->GetPixelType(), 2, input->GetDimensions());
  slice->SetVolume(accessor.GetData());

  // Transform the slice to matrix
  m_ImageToOpenCVFilter->SetImage(slice);
  return m_ImageToOpenCVFilter->GetOpenCVMat();
}

/*!
 * \brief Insert a OpenCV matrix as a slice into an image
 *
 * This method converts the 2d OpenCV matrix @p mat into an mitk image using the mitkOpenCVToMitkImageFilter. Afterwards it inserts the image as slice @p i into the 3d mitk image @p output.
 *
 * @param mat The matrix to be inserted as a slice
 * @param output The 3d image the matrix is inserted into
 * @param i The index of the slice to be replaced.
 */

void mitk::PhotoacousticMotionCorrectionFilter::EnterMatrixInPosition(cv::Mat mat, mitk::Image::Pointer output, unsigned int i) {

  m_OpenCVToImageFilter->SetOpenCVMat(mat);
  m_OpenCVToImageFilter->Update();
  mitk::Image::Pointer slice = m_OpenCVToImageFilter->GetOutput();

  mitk::ImageReadAccessor accessor(slice);
  output->SetSlice(accessor.GetData(), i);
}


/*!
 * \brief Apply OpenCV algorithm to compensate motion in a 2d image time series
 *
 * This method uses two 3d mitk images. Both will be interpreted as time series of 2d images. @c GetInput(0) should be a photoacoustic image whereas @c GetInput(1) should be an ultrasound image. The input will be validated and then converted to OpenCV matrices. In the end the Farneback algorithm will be used to compute the optical flow in consecutive images and compensate for this flow. The Output will be two 3d mitk images of the same dimensions as the input containing the compensated data.
 *
 * @warning The input images need to be 3-dimensional (with the same size in each dimension). Otherwise, an @c invalid_argument exception will be thrown.
 */
// TODO: remove debug messages
void mitk::PhotoacousticMotionCorrectionFilter::GenerateData() {
  MITK_INFO << "Start motion compensation.";

  mitk::Image::Pointer paInput = this->GetInput(0);
  mitk::Image::Pointer usInput = this->GetInput(1);
  mitk::Image::Pointer paOutput = this->GetOutput(0);
  mitk::Image::Pointer usOutput = this->GetOutput(1);

  // Check that we have two input images with agreeing dimensions
  try {
    this->CheckInput(paInput, usInput);
  }
  catch(std::invalid_argument& e) {
    throw e;
  }

  // Check the output images and (re-)initialize, if necessary.
  this->InitializeOutput(paInput, usInput, paOutput, usOutput);

  // m_ImageToOpenCVFilter->SetImage(paInput);
  this->PerformCorrection(paInput, usInput, paOutput, usOutput);

  MITK_INFO << "Motion compensation accomplished.";
}
