/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITKPHOTOACOUSTICSMOTIONCORRECTIONFILTER_H_
#define _MITKPHOTOACOUSTICSMOTIONCORRECTIONFILTER_H_

#include "mitkImageToImageFilter.h"
#include <itkObject.h>

#include "opencv2/imgproc.hpp"
// TODO: Find out why build fails with this option or replace with something
// else
/* #include "opencv2/opencv.hpp" */
#include "opencv2/video/tracking.hpp"

#include "itkOpenCVImageBridge.h"

#include <MitkPhotoacousticsAlgorithmsExports.h>

#include "mitkImageCast.h"
#include <mitkImageToOpenCVImageFilter.h>
#include <mitkOpenCVToMitkImageFilter.h>
#include <mitkImageStatisticsHolder.h>

#define IMAGE_DIMENSION 3 /*!< All images need to have dimension 3*/
#define MAX_MATRIX 255.0 /*!< Rescaling constant to maximum character*/

namespace mitk
{
  /*!
  * \brief Class implementing a mitk::ImageToImageFilter for PAUS motion
  * correction.
  *
  *  The filter takes a stack of PA and US images. It then computes the optical
  * flow
  *  within the US image and compensates the PA and US images for the flow.
  * Afterwards it
  *  returns the stack of PA and US images.
  *
  * @see
  * https://docs.opencv.org/3.0-beta/modules/video/doc/motion_analysis_and_object_tracking.html#calcopticalflowfarneback
  */
  class MITKPHOTOACOUSTICSALGORITHMS_EXPORT PhotoacousticMotionCorrectionFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(PhotoacousticMotionCorrectionFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);

    // Setters and Getters for the class variables
    itkSetMacro(BatchSize, unsigned int);
    itkSetMacro(PyrScale, double);
    itkSetMacro(Levels, unsigned int);
    itkSetMacro(WinSize, unsigned int);
    itkSetMacro(Iterations, unsigned int);
    itkSetMacro(PolyN, unsigned int);
    itkSetMacro(PolySigma, double);
    itkSetMacro(Flags, unsigned int);
    itkGetConstMacro(BatchSize, unsigned int);
    itkGetConstMacro(PyrScale, double);
    itkGetConstMacro(Levels, unsigned int);
    itkGetConstMacro(WinSize, unsigned int);
    itkGetConstMacro(Iterations, unsigned int);
    itkGetConstMacro(PolyN, unsigned int);
    itkGetConstMacro(PolySigma, double);
    itkGetConstMacro(Flags, unsigned int);

    // Wrapper for SetInput, GetInput and GetOutput
    /*!
      * \brief Wrapper which sets the photoacoustic image as the correct input
      *
      * This method is a wrapper around the @c SetInput method. It is implemented
      * for convenience such that you do not have to remember which input is for
      * which image.
      *
      * @param input The photoacoustic image
      */
    void SetPaInput(mitk::Image::Pointer input);
    /*!
     * \brief Wrapper which gets the photoacoustic image out of the correct input
     *
     * This method is a wrapper around the @c GetInput method. It is implemented
     * for convenience such that you do not have to remember which input is for
     * which image.
     *
     * @return The photoacoustic image
     */
    mitk::Image::Pointer GetPaInput();
    /*!
     * \brief Wrapper which sets the ultrasonic image as the correct input
     *
     * This method is a wrapper around the @c SetInput method. It is implemented
     * for convenience such that you do not have to remember which input is for
     * which image.
     *
     * @param input The ultrasonic image
     */
    void SetUsInput(mitk::Image::Pointer input);
    /*!
     * \brief Wrapper which gets the ultrasonic image out of the correct input
     *
     * This method is a wrapper around the @c GetInput method. It is implemented
     * for convenience such that you do not have to remember which input is for
     * which image.
     *
     * @return The ultrasonic image
     */
    mitk::Image::Pointer GetUsInput();
    /*!
     * \brief Wrapper which gets the photoacoustic image out of the correct output
     *
     * This method is a wrapper around the @c GetOutput method. It is implemented
     * for convenience such that you do not have to remember which output is for
     * which image.
     *
     * @return The photoacoustic image
     */
    mitk::Image::Pointer GetPaOutput();
    /*!
     * \brief Wrapper which gets the ultrasonic image out of the correct output
     *
     * This method is a wrapper around the @c GetOutput method. It is implemented
     * for convenience such that you do not have to remember which output is for
     * which image.
     *
     * @return The ultrasonic image
     */
    mitk::Image::Pointer GetUsOutput();

  protected:
    PhotoacousticMotionCorrectionFilter();

    ~PhotoacousticMotionCorrectionFilter() override;

    /*!
     * \brief Apply OpenCV algorithm to compensate motion in a 2d image time
     * series
     *
     * This method uses two 3d mitk images. Both will be interpreted as time
     * series of 2d images. @c GetInput(0) should be a photoacoustic image whereas
     * @c GetInput(1) should be an ultrasound image. The input will be validated
     * and then converted to OpenCV matrices. In the end the Farneback algorithm
     * will be used to compute the optical flow in consecutive images and
     * compensate for this flow. The Output will be two 3d mitk images of the same
     * dimensions as the input containing the compensated data.
     *
     * @warning The input images need to be 3-dimensional (with the same size in
     * each dimension). Otherwise, an @c invalid_argument exception will be
     * thrown.
     * @throws invalid_argument
     */
    void GenerateData() override;
    /*!
     * \brief Validate the input images
     *
     * The input images have to be non-empty, 3d and have to coincide in the
     * length in each dimension. If any of these conditions are violated, the
     * method will throw an @c invalid_argument exception.
     *
     * @param paImage A mitk image
     * @param usImage A mitk image
     * @warning If the two images are not 3d and do not coincide in the length in
     * each dimension, this method will throw an @c invalid_argument exception.
     * @throws invalid_argument
     */
    void CheckInput(mitk::Image::Pointer paImage, mitk::Image::Pointer usImage);
    /*!
     * \brief Assure that the output images have the same dimensions as the input
     * images.
     *
     * The output images need to have the same dimensions as the input images.
     * This will be checked here. If the dimensions do not match, the output will
     * be reinitialized and the image data from the input images will be copied to
     * the output images (in order to make sure that they have a valid data
     * pointer).
     *
     * @param paInput Pointer to the photoacoustic input image
     * @param usInput Pointer to the ultrasonic input image
     * @param paOutput Pointer to the photoacoustic output image
     * @param usOutput Pointer to the ultrasonic output image
     */
    void InitializeOutputIfNecessary(mitk::Image::Pointer paInput,
                                     mitk::Image::Pointer usInput,
                                     mitk::Image::Pointer paOutput,
                                     mitk::Image::Pointer usOutput);
    /*!
     * \brief Copy the image data from the input image to the output image
     *
     * This method copys the image data from @p input to @p output. This method
     * assumes that the dimensions of the two images match and will not test this.
     *
     * @param input A mitk image
     * @param output A mitk image
     */
    void InitializeOutput(mitk::Image::Pointer input, mitk::Image::Pointer output);
    /*!
     * \brief This method performs the actual motion compensation.
     *
     * This method uses the ultrasonic input image @p usInput to compute the
     * optical flow in the time series of 2d images. Then it compensates both the
     * @p usInput and @p paInput for it and saves the result in @p usOutput and @p
     * paOutput respectively. In the background the OpenCV Farneback algorithm is
     * used for the flow determination.
     *
     * @param paInput The photoacoustic input image
     * @param usInput The ultrasonic input image
     * @param paOutput The photoacoustic output image
     * @param usOutput The ultrasonic output image
     */
    void PerformCorrection(mitk::Image::Pointer paInput,
                           mitk::Image::Pointer usInput,
                           mitk::Image::Pointer paOutput,
                           mitk::Image::Pointer usOutput);
    /*!
     * \brief Extract a 2d slice as OpenCV matrix.
     *
     * This method extracts slice @p i from the 3-dimensional image @p input and
     * converts it to a OpenCV matrix. Internally, the
     * mitkImageToOpenCVImageFilter is used.
     *
     * @param input A 3d image from which a slice is extracted as a 2d OpenCV
     * matrix.
     * @param i Determines the slice to be extracted.
     * @return returns a OpenCV matrix containing the 2d slice.
     */
    cv::Mat GetMatrix(const mitk::Image::Pointer input, unsigned int i);

    /*!
     * \brief Rescale matrix such that the values lie between 0 and 255
     *
     * This method rescales the matrix such that its values lie between 0 and 255. In order to do that it uses the maximum and the minimum of the input ultrasonic image.
     *
     * @warning This is a specialized method which does not perform the operation in general, but only if the matrix stems from the right ultrasonic image. Therefore, the method should only be called internally.
     *
     * @param mat The OpenCV matrix to be rescaled
     * @return The rescaled OpenCV matrix
     */
    cv::Mat FitMatrixToChar(cv::Mat mat);

    /*!
     * \brief Insert a OpenCV matrix as a slice into an image
     *
     * This method converts the 2d OpenCV matrix @p mat into an mitk image using
     * the mitkOpenCVToMitkImageFilter. Afterwards it inserts the image as slice
     * @p i into the 3d mitk image @p output.
     *
     * @param mat The matrix to be inserted as a slice
     * @param output The 3d image the matrix is inserted into
     * @param i The index of the slice to be replaced.
     */
    void InsertMatrixAsSlice(cv::Mat mat, mitk::Image::Pointer output, unsigned int i);

    /*!
     * \brief Compute the remapping map from an optical flow
     *
     * The optical flow cannot be used directly to compensate an image. Instead we have to generate an appropriate map.
     *
     * @param flow The optical flow which is the base for the remapping.
     * @return The remapping map.
     */
    cv::Mat ComputeFlowMap(cv::Mat flow);

  private:
    // Parameters
    double m_PyrScale;         /*!<  See @c pyr_scale in @c cv::calcOpticalFlowFarneback
         */
    double m_PolySigma;        /*!< See @c poly_sigma in @c cv::calcOpticalFlowFarneback
         */
    unsigned int m_Levels;     /*!< See @c levels in @c cv::calcOpticalFlowFarneback */
    unsigned int m_WinSize;    /*!< See @c winsize in @c cv::calcOpticalFlowFarneback */
    unsigned int m_Iterations; /*!< See @c iterations in @c cv::calcOpticalFlowFarneback */
    unsigned int m_PolyN;      /*!< See @c poly_n in @c cv::calcOpticalFlowFarneback */
    unsigned int m_Flags;      /*!< See @c flags in @c cv::calcOpticalFlowFarneback */
    unsigned int m_BatchSize;  /*!< Determines how many slices belong together and will be
                              motion compensated with regard to the first image in the
                              batch. If the variable is set to 0, the whole time series will
                              be processed as one batch. */
    float m_MaxValue; /*!< The maximum of the ultrasonic image*/
    float m_MinValue; /*!< The minimum of the ultrasonic image*/

    // Stuff that OpenCV needs
    cv::Mat m_UsRef; /*!< Contains the reference ultrasonic image to which the
                         motion compensation is compared to.*/
    cv::Mat m_Flow;  /*!< Contains the optical flow between @c m_UsRef and @c m_UsMat*/
    cv::Mat m_PaRes; /*!< Contains the motion compensated photoacoustic image*/
    cv::Mat m_UsRes; /*!< Contains the motion compensated ultrasonic image*/
    cv::Mat m_PaMat; /*!< Contains the latest photoacoustic image to be motion compensated*/
    cv::Mat m_UsMat; /*!< Contains the latest ultrasonic image on which the
                        optical flow is to be computed */
    cv::Mat m_Map; /*!< Contains the remapping map */

    mitk::OpenCVToMitkImageFilter::Pointer m_OpenCVToImageFilter =
      mitk::OpenCVToMitkImageFilter::New(); /*!< Filter which converts an OpenCV matrix to a mitk image */
    mitk::ImageToOpenCVImageFilter::Pointer m_ImageToOpenCVFilter =
      mitk::ImageToOpenCVImageFilter::New(); /*!< Filter which converts a mitk image to an OpenCV matrix */
  };
}
#endif
