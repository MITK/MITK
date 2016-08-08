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

#include "mitkUSFraunhoferImageSource.h"
#include "mitkUSFraunhoferSDKHeader.h"
#include "mitkImageReadAccessor.h"

mitk::USFraunhoferImageSource::USFraunhoferImageSource()
  : m_Image(mitk::Image::New())
{

}

mitk::USFraunhoferImageSource::~USFraunhoferImageSource( )
{

}

void mitk::USFraunhoferImageSource::GetNextRawImage( mitk::Image::Pointer& image)
{
   if(image.IsNull()) { 
      image = mitk::Image::New(); 
   }
   
}

void newDataCallback(
    short* rfDataChannelData,
    int channelDataChannelsPerDataset,
    int channelDataSamplesPerChannel,
    int channelDataTotalDatasets,

    short* rfDataArrayBeamformed,
    int beamformedLines,
    int beamformedSamples,
    int beamformedTotalDatasets,

    unsigned char* imageData,
    int imageWidth,
    int imageHeight,
    int imageBytesPerPixel,
    int imageSetsTotal,

    double timeStamp)
{
   if (imageData != nullptr)
      {
      }
}