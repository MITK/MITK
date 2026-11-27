/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCropTimestepsImageFilter_h
#define mitkCropTimestepsImageFilter_h

#include <MitkAlgorithmsExtExports.h>

#include <mitkSubImageSelector.h>

namespace mitk
{
  /**
   * \brief Crops timesteps from 2D+t and 3D+t images.
   *
   * This filter removes timesteps from the beginning and/or end of a 4D image.
   * A new image is created containing only the timesteps within the specified
   * range [LowerBoundaryTimestep, UpperBoundaryTimestep). The geometries and
   * properties of the input image are transferred to the output image.
   *
   * \pre The input image must have 4 dimensions and more than 1 timestep.
   *
   * \sa SubImageSelector
   * \sa ImageTimeSelector
   * \ingroup Process
   */
  class MITKALGORITHMSEXT_EXPORT CropTimestepsImageFilter : public SubImageSelector
  {
  public:
    mitkClassMacro(CropTimestepsImageFilter, SubImageSelector);
    itkFactorylessNewMacro(Self);

    /**
     * \brief Set the input image.
     *
     * \param[in] image The 4D image to crop.
     * \pre The image must have 4 dimensions.
     * \pre The image must have more than 1 timestep.
     * \pre The image must be valid (valid geometry and volume).
     */
    void SetInput(const InputImageType* image) override;

    /**
     * \brief Set the input image at the specified index (only index 0 is valid).
     *
     * \param[in] index Must be 0.
     * \param[in] image The 4D image to crop.
     * \throw mitk::Exception if index is not 0.
     */
    void SetInput(unsigned int index, const InputImageType* image) override;

    /**
     * \brief Set the first timestep to drop at the end (exclusive upper bound).
     *
     * Not set by default, which keeps all timesteps up to the end of the input.
     * A larger value than the input's number of timesteps is clamped for the
     * output; the setting itself is kept. Update() throws mitk::Exception if
     * LowerBoundaryTimestep is not smaller than the clamped upper boundary.
     *
     * \param[in] _arg The upper boundary timestep (exclusive).
     */
    itkSetMacro(UpperBoundaryTimestep, TimeStepType);

    /**
     * \brief Get the upper boundary timestep (exclusive).
     * \return The upper boundary timestep.
     */
    itkGetConstMacro(UpperBoundaryTimestep, TimeStepType);

    /**
     * \brief Set the first timestep to retain (inclusive lower bound, default: 0).
     *
     * Update() throws mitk::Exception if it is not smaller than the clamped
     * upper boundary.
     *
     * \param[in] _arg The lower boundary timestep (inclusive).
     */
    itkSetMacro(LowerBoundaryTimestep, TimeStepType);

    /**
     * \brief Get the lower boundary timestep (inclusive).
     * \return The lower boundary timestep.
     */
    itkGetConstMacro(LowerBoundaryTimestep, TimeStepType);

  private:
    using Superclass::SetInput;

    CropTimestepsImageFilter() = default;
    ~CropTimestepsImageFilter() override = default;

    void GenerateData() override;
    void VerifyInputInformation() const override;
    void VerifyInputImage(const mitk::Image* inputImage) const;
    void GenerateOutputInformation() override;
    mitk::SlicedData::RegionType ComputeDesiredRegion(TimeStepType upperBoundary) const;
    mitk::TimeGeometry::Pointer AdaptTimeGeometry(mitk::TimeGeometry::ConstPointer sourceGeometry, TimeStepType startTimestep, TimeStepType endTimestep) const;

    TimeStepType m_UpperBoundaryTimestep = std::numeric_limits<TimeStepType>::max();
    TimeStepType m_LowerBoundaryTimestep = 0;

    mitk::SlicedData::RegionType m_DesiredRegion;
  };
}

#endif
