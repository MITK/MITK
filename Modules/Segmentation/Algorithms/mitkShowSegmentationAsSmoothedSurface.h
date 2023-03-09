/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkShowSegmentationAsSmoothedSurface_h
#define mitkShowSegmentationAsSmoothedSurface_h

#include "mitkSegmentationSink.h"
#include <MitkSegmentationExports.h>
#include <mitkSurface.h>

namespace mitk
{
  class MITKSEGMENTATION_EXPORT ShowSegmentationAsSmoothedSurface : public SegmentationSink
  {
  public:
    mitkClassMacro(ShowSegmentationAsSmoothedSurface, SegmentationSink);
    mitkAlgorithmNewMacro(ShowSegmentationAsSmoothedSurface);

  protected:
    void Initialize(const NonBlockingAlgorithm *other = nullptr) override;
    bool ReadyToRun() override;
    bool ThreadedUpdateFunction() override;
    void ThreadedUpdateSuccessful() override;

  private:
    ShowSegmentationAsSmoothedSurface();
    ~ShowSegmentationAsSmoothedSurface() override;

    Surface::Pointer m_Surface;
  };
}

#endif
