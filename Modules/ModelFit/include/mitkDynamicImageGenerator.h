/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef DYNAMICIMAGEGENERATOR_H
#define DYNAMICIMAGEGENERATOR_H


#include "mitkImageToImageFilter.h"

#include "MitkModelFitExports.h"

namespace mitk
{
  /** Filter that takes n inputs and fuse it to a dynamic image (with n time points).*/
  class MITKMODELFIT_EXPORT DynamicImageGenerationFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(DynamicImageGenerationFilter, ImageToImageFilter);
    itkFactorylessNewMacro(DynamicImageGenerationFilter);
    itkCloneMacro(Self);

    typedef std::vector<mitk::TimePointType> TimeBoundsVectorType;

    itkGetConstMacro(TimeBounds, TimeBoundsVectorType);
    void SetTimeBounds(const TimeBoundsVectorType &bounds);

  protected:
    DynamicImageGenerationFilter(){};
    ~DynamicImageGenerationFilter() override{};

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

  private:
    TimeBoundsVectorType m_TimeBounds;
  };
}


#endif // MODELSIGNALIMAGEGENERATOR_H
