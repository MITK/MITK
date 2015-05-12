/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITK_SHOW_SEGMENTATION_AS_SMOOTHED_SURFACE_H
#define MITK_SHOW_SEGMENTATION_AS_SMOOTHED_SURFACE_H

#include <MitkSegmentationExports.h>
#include "mitkSegmentationSink.h"
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
    ~ShowSegmentationAsSmoothedSurface();

    Surface::Pointer m_Surface;
  };
}

#endif
