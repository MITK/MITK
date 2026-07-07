/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTemporalJoinImagesFilter_h
#define mitkTemporalJoinImagesFilter_h


#include <mitkImageToImageFilter.h>
#include <mitkCommon.h>

#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Filter that fuses n input images into a single image with n time points.
   *
   * This filter takes n mitk::Image instances as inputs and fuses them into a new
   * output image containing n time steps. The sequence of frames in the output image
   * matches the sequence of inputs.
   *
   * \pre All input images must have the same pixel type.
   * \pre All input images must have the same geometry.
   * \post If no time bounds are defined, the dynamic image starts at 0 ms and each
   *       time step has a duration of 1 ms.
   *
   * \ingroup Process
   */
  class MITKCORE_EXPORT TemporalJoinImagesFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(TemporalJoinImagesFilter, ImageToImageFilter);
    itkFactorylessNewMacro(TemporalJoinImagesFilter);

    typedef std::vector<mitk::TimePointType> TimeBoundsVectorType;

    itkGetConstMacro(FirstMinTimeBound, mitk::TimePointType);
    /** \brief Set custom minimum time bound for the first time step. */
    itkSetMacro(FirstMinTimeBound, mitk::TimePointType);

    itkGetConstMacro(MaxTimeBounds, TimeBoundsVectorType);
    /**
     * \brief Set custom maximum time bounds for all time steps.
     *
     * The size of \p bounds must match the number of filter inputs.
     *
     * \param[in] bounds Vector of maximum time bounds, one per time step.
     * \throw mitk::Exception if the size of bounds does not match the number of inputs.
     */
    void SetMaxTimeBounds(const TimeBoundsVectorType &bounds);

  protected:
    TemporalJoinImagesFilter(){};
    ~TemporalJoinImagesFilter() override{};

    /** \brief Request the largest possible region from all inputs. */
    void GenerateInputRequestedRegion() override;

    /** \brief Compute output information including time geometry and merged properties. */
    void GenerateOutputInformation() override;

    /** \brief Copy the volume data from each input into the corresponding time step of the output. */
    void GenerateData() override;

  private:
    TimeBoundsVectorType m_MaxTimeBounds;
    mitk::TimePointType m_FirstMinTimeBound = 0.0;
  };
}


#endif
