/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLabelSetImageToSurfaceThreadedFilter_h
#define mitkLabelSetImageToSurfaceThreadedFilter_h

#include "mitkSegmentationSink.h"
#include "mitkSurface.h"
#include <MitkMultilabelExports.h>

namespace mitk
{
  class MITKMULTILABEL_EXPORT LabelSetImageToSurfaceThreadedFilter : public SegmentationSink
  {
  public:
    mitkClassMacro(LabelSetImageToSurfaceThreadedFilter, SegmentationSink);
    mitkAlgorithmNewMacro(LabelSetImageToSurfaceThreadedFilter);

  protected:
    LabelSetImageToSurfaceThreadedFilter(); // use smart pointers
    ~LabelSetImageToSurfaceThreadedFilter() override;

    void Initialize(const NonBlockingAlgorithm *other = nullptr) override;
    bool ReadyToRun() override;

    bool ThreadedUpdateFunction() override; // will be called from a thread after calling StartAlgorithm

    void ThreadedUpdateSuccessful() override; // will be called from a thread after calling StartAlgorithm

  private:
    int m_RequestedLabel;
    Surface::Pointer m_Result;
  };

} // namespace

#endif
