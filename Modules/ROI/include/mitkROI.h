/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROI_h
#define mitkROI_h

#include <mitkBaseData.h>
#include <MitkROIExports.h>

namespace mitk
{
  class MITKROI_EXPORT ROI : public BaseData
  {
  public:
    mitkClassMacro(ROI, BaseData)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void SetRequestedRegionToLargestPossibleRegion() override;
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
    bool VerifyRequestedRegion() override;
    void SetRequestedRegion(const itk::DataObject* data) override;

  protected:
    mitkCloneMacro(Self)

    ROI();
    ROI(const Self& other);
    ~ROI() override;
  };
}

#endif
