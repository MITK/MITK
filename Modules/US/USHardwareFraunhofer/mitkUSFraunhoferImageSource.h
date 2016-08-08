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

#ifndef MITKUSFraunhoferImageSource_H_HEADER_INCLUDED_
#define MITKUSFraunhoferImageSource_H_HEADER_INCLUDED_

#include "mitkUSImageSource.h"
#include "mitkUSFraunhoferSDKHeader.h"

#include "itkFastMutexLock.h"

namespace mitk {
/**
  * \brief Implementation of mitk::USImageSource for Fraunhofer API devices.
  * The method mitk::USImageSource::GetNextRawImage() is implemented for
  * getting images from the Fraunhofer API.
  *
  * A method for connecting this ImageSource to the Fraunhofer API is
  * implemented (mitk::USFraunhoferImageSource::CreateAndConnectConverterPlugin()).
  * This method is available for being used by mitk::USFraunhoferDevice.
  */
class USFraunhoferImageSource : public USImageSource
{
public:
  mitkClassMacro(USFraunhoferImageSource, USImageSource);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /**
    * Implementation of the superclass method. Returns the pointer
    * to the mitk::Image filled by Fraunhofer API callback.
    */
  virtual void GetNextRawImage( mitk::Image::Pointer& );

  /**
    * The API calls this function to pass the image data to the
	* user; here the m_Image is updated
    */
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

      double timeStamp);

protected:
  USFraunhoferImageSource( );
  virtual ~USFraunhoferImageSource( );
  mitk::Image::Pointer                        m_Image;
};
} // namespace mitk

#endif // MITKUSFraunhoferImageSource_H
