/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCalculateSegmentationVolume_h
#define mitkCalculateSegmentationVolume_h

#include "mitkImageCast.h"
#include "mitkSegmentationSink.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  class MITKSEGMENTATION_EXPORT CalculateSegmentationVolume : public SegmentationSink
  {
  public:
    mitkClassMacro(CalculateSegmentationVolume, SegmentationSink);
    mitkAlgorithmNewMacro(CalculateSegmentationVolume);

  protected:
    CalculateSegmentationVolume(); // use smart pointers
    ~CalculateSegmentationVolume() override;

    bool ReadyToRun() override;

    bool ThreadedUpdateFunction() override; // will be called from a thread after calling StartAlgorithm

    template <typename TPixel, unsigned int VImageDimension>
    void ItkImageProcessing(itk::Image<TPixel, VImageDimension> *itkImage, TPixel *dummy = nullptr);

  private:
    unsigned int m_Volume;

    Vector3D m_CenterOfMass;
    Vector3D m_MinIndexOfBoundingBox;
    Vector3D m_MaxIndexOfBoundingBox;
  };

} // namespace

#endif
