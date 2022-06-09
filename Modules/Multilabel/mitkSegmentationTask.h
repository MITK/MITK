/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationTask_h
#define mitkSegmentationTask_h

#include <mitkBaseData.h>
#include <MitkMultilabelExports.h>

namespace mitk
{
  class MITKMULTILABEL_EXPORT SegmentationTask : public BaseData
  {
  public:
    mitkClassMacro(SegmentationTask, BaseData)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void SetRequestedRegionToLargestPossibleRegion() override;
    bool RequestedRegionIsOutsideOfTheBufferedRegion() override;
    bool VerifyRequestedRegion() override;
    void SetRequestedRegion(const itk::DataObject*) override;

  protected:
    mitkCloneMacro(Self)

    SegmentationTask();
    SegmentationTask(const Self& other);
    ~SegmentationTask() override;
  };
}

#endif
