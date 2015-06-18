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

#ifndef MITK_CALCULATE_SEGMENTATION_VOLUME_H_INCLUDET_WAD
#define MITK_CALCULATE_SEGMENTATION_VOLUME_H_INCLUDET_WAD

#include "mitkSegmentationSink.h"
#include <MitkSegmentationExports.h>
#include "mitkImageCast.h"

namespace mitk
{

class MITKSEGMENTATION_EXPORT CalculateSegmentationVolume : public SegmentationSink
{
  public:

    mitkClassMacro( CalculateSegmentationVolume, SegmentationSink )
    mitkAlgorithmNewMacro( CalculateSegmentationVolume );

  protected:

    CalculateSegmentationVolume();  // use smart pointers
    virtual ~CalculateSegmentationVolume();

    virtual bool ReadyToRun() override;

    virtual bool ThreadedUpdateFunction() override; // will be called from a thread after calling StartAlgorithm

    template < typename TPixel, unsigned int VImageDimension >
    void ItkImageProcessing( itk::Image< TPixel, VImageDimension >* itkImage, TPixel* dummy = NULL );

  private:

    unsigned int m_Volume;

    Vector3D m_CenterOfMass;
    Vector3D m_MinIndexOfBoundingBox;
    Vector3D m_MaxIndexOfBoundingBox;

};

} // namespace

#endif


