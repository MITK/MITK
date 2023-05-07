/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTemporalJoinImagesFilter_h
#define mitkTemporalJoinImagesFilter_h


#include "mitkImageToImageFilter.h"
#include "mitkCommon.h"

#include "MitkCoreExports.h"

namespace mitk
{
  /** Filter that takes n mitk images as inputs and fuse them to a new image (with n time points).
  Preconditions of this filter are, that all input images have the same pixel type and geometry.
  The sequence of frames in the output image is the same then the sequence of inputs.
  It no time bounds are defined the dynamic image will start at 0 ms and each time step has a duration
  of 1 ms.*/
  class MITKCORE_EXPORT TemporalJoinImagesFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(TemporalJoinImagesFilter, ImageToImageFilter);
    itkFactorylessNewMacro(TemporalJoinImagesFilter);

    typedef std::vector<mitk::TimePointType> TimeBoundsVectorType;

    itkGetConstMacro(FirstMinTimeBound, mitk::TimePointType);
    /**Set custom min time bound for the first time step.*/
    itkSetMacro(FirstMinTimeBound, mitk::TimePointType);

    itkGetConstMacro(MaxTimeBounds, TimeBoundsVectorType);
    /**Set custom max time bounds for all time steps.*/
    void SetMaxTimeBounds(const TimeBoundsVectorType &bounds);

  protected:
    TemporalJoinImagesFilter(){};
    ~TemporalJoinImagesFilter() override{};

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

  private:
    TimeBoundsVectorType m_MaxTimeBounds;
    mitk::TimePointType m_FirstMinTimeBound = 0.0;
  };
}


#endif
