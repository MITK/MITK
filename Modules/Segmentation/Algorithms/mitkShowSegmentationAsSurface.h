/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkShowSegmentationAsSurface_h
#define mitkShowSegmentationAsSurface_h

#include "mitkSegmentationSink.h"
#include "mitkSurface.h"
#include "mitkUIDGenerator.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  class MITKSEGMENTATION_EXPORT ShowSegmentationAsSurface : public SegmentationSink
  {
  public:
    mitkClassMacro(ShowSegmentationAsSurface, SegmentationSink);
    mitkAlgorithmNewMacro(ShowSegmentationAsSurface);

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
