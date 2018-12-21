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

#ifndef MITK_SHOW_SEGMENTATION_AS_SURFACE_H_INCLUDET_WAD
#define MITK_SHOW_SEGMENTATION_AS_SURFACE_H_INCLUDET_WAD

#include "mitkSegmentationSink.h"
#include "mitkSurface.h"
#include "mitkUIDGenerator.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  class MITKSEGMENTATION_EXPORT ShowSegmentationAsSurface : public SegmentationSink
  {
  public:
    mitkClassMacro(ShowSegmentationAsSurface, SegmentationSink) mitkAlgorithmNewMacro(ShowSegmentationAsSurface);

  protected:
    ShowSegmentationAsSurface(); // use smart pointers
    ~ShowSegmentationAsSurface() override;

    void Initialize(const NonBlockingAlgorithm *other = nullptr) override;
    bool ReadyToRun() override;

    bool ThreadedUpdateFunction() override; // will be called from a thread after calling StartAlgorithm

    void ThreadedUpdateSuccessful() override; // will be called from a thread after calling StartAlgorithm

  private:
    mitk::Surface::Pointer ConvertBinaryImageToSurface(mitk::Image::Pointer binaryImage);

    UIDGenerator m_UIDGeneratorSurfaces;

    std::vector<DataNode::Pointer> m_SurfaceNodes;
    bool m_IsLabelSetImage;
  };

} // namespace

#endif
