/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBoundingShapeCropper_h
#define mitkBoundingShapeCropper_h

#include <MitkBoundingShapeExports.h>
#include <mitkBoundingShapeCropper.h>
#include <mitkCommon.h>
#include <mitkGeometryData.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageToImageFilter.h>

#include <itkImage.h>

namespace mitk
{
  /** \brief Crops or masks an image using a bounding box defined by GeometryData.
   *
   * This filter takes an mitk::Image and an mitk::GeometryData as inputs and produces
   * a cropped or masked image as output.
   *
   * Two modes of operation are supported:
   * - **Cropping** (UseWholeInputRegion = false): The output image is reduced to the size
   *   of the bounding box.
   * - **Masking** (UseWholeInputRegion = true): The output image retains the original
   *   dimensions, but pixels outside the bounding box are set to the OutsideValue.
   *
   * For 4D images, optionally only a single time step can be processed via
   * UseCropTimeStepOnly and CurrentTimeStep.
   *
   * \pre The input image and a valid GeometryData must be set before calling Update().
   *
   * \sa GeometryData, ImageToImageFilter, BoundingShapeInteractor
   * \ingroup Process
   */
  class MITKBOUNDINGSHAPE_EXPORT BoundingShapeCropper : public ImageToImageFilter
  {
  public:
    mitkClassMacro(BoundingShapeCropper, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Set the geometry of the bounding object used for cropping/masking.
     *
     * \param[in] geometry The GeometryData defining the bounding box region.
     *
     * \pre \p geometry is not null and has a valid BaseGeometry.
     */
    void SetGeometry(const mitk::GeometryData *geometry);

    /** \brief Set the pixel value assigned to voxels outside the bounding box during masking.
     *
     * Only used when UseWholeInputRegion is \c true (masking mode). Default: 0.
     *
     * \param[in] _arg The scalar value for outside pixels.
     */
    itkSetMacro(OutsideValue, ScalarType);

    /** \brief Get the current outside pixel value used for masking.
     *
     * \return The scalar value assigned to pixels outside the bounding box.
     */
    itkGetMacro(OutsideValue, ScalarType);

    /** \brief Set whether the full input region is preserved (masking mode).
     *
     * \param[in] _arg If \c true, the output has the same dimensions as the input and
     *                 outside pixels are set to OutsideValue (masking). If \c false,
     *                 the output is cropped to the bounding box size.
     */
    itkSetMacro(UseWholeInputRegion, bool);

    /** \brief Get whether masking mode is active.
     *
     * \return \c true if masking mode is active, \c false for cropping mode.
     */
    itkGetMacro(UseWholeInputRegion, bool);

    /** \brief Set the time step to process for 4D images.
     *
     * Only relevant when UseCropTimeStepOnly is \c true.
     *
     * \param[in] _arg The zero-based time step index.
     */
    itkSetMacro(CurrentTimeStep, ScalarType);

    /** \brief Get the current time step index.
     *
     * \return The zero-based time step index.
     */
    itkGetMacro(CurrentTimeStep, ScalarType);

    /** \brief Set whether to process only a single time step.
     *
     * \param[in] _arg If \c true, only the time step specified by CurrentTimeStep is
     *                 cropped/masked. If \c false, all time steps are processed.
     */
    itkSetMacro(UseCropTimeStepOnly, bool);

    /** \brief Get whether only a single time step is processed.
     *
     * \return \c true if only the CurrentTimeStep is processed.
     */
    itkGetMacro(UseCropTimeStepOnly, bool);

  protected:
    BoundingShapeCropper();
    ~BoundingShapeCropper() override;

    virtual const PixelType GetOutputPixelType();

    /**
    * @brief Reimplemented from ImageToImageFilter
    */
    void GenerateInputRequestedRegion() override;
    void GenerateOutputInformation() override;
    void GenerateData() override;

    /**
    * @brief Template Function for cropping and masking images with scalar pixel type
    */
    template <typename TPixel, unsigned int VImageDimension>
    void CutImage(itk::Image<TPixel, VImageDimension> *inputItkImage, int timeStep);

    /**
    *@brief Process the image and create the output
     **/
    virtual void ComputeData(mitk::Image *input3D, int boTimeStep);

    // virtual void ComputeData(mitk::MultiLabelSegmentation* image, int boTimeStep);

  private:
    /**
    *@brief GeometryData Type to capsulate all necessary components of the bounding object
     **/
    mitk::GeometryData::Pointer m_Geometry;

    /**
    * @brief scalar value for outside pixels (default: 0)
    */
    ScalarType m_OutsideValue;

    /**
    * @brief Use m_UseCropTimeStepOnly for only cropping a single time step(default: \a false)
    */
    bool m_UseCropTimeStepOnly;

    /**
    * @brief Current time step displayed
    */
    int m_CurrentTimeStep;
    /**
    * @brief Use m_UseWholeInputRegion for deciding whether a cropping or masking will be performed
    */
    bool m_UseWholeInputRegion;
    /**
    * @brief Select single input image in a timeseries
    */
    mitk::ImageTimeSelector::Pointer m_InputTimeSelector;
    /**
    * @brief Select single output image in a timeseries
    */
    mitk::ImageTimeSelector::Pointer m_OutputTimeSelector;
    /**
    * @brief Region of input needed for cutting
    */
    typedef itk::ImageRegion<5> RegionType;

    mitk::SlicedData::RegionType m_InputRequestedRegion;
    /**
    * @brief Time when Header was last initialized
    **/
    itk::TimeStamp m_TimeOfHeaderInitialization;
  };
} // namespace mitk

#endif
