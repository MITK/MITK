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

#ifndef MITKUSDiPhASImageSource_H_HEADER_INCLUDED_
#define MITKUSDiPhASImageSource_H_HEADER_INCLUDED_

#include "mitkUSImageSource.h"
#include "Framework.IBMT.US.CWrapper.h"

#include "itkFastMutexLock.h"

namespace mitk {
/**
  * \brief Implementation of mitk::USImageSource for DiPhAS API devices.
  * The method mitk::USImageSource::GetNextRawImage() is implemented for
  * getting images from the DiPhAS API.
  *
  * The image data is given to this class from the DiPhAS API by calling 
  * a callback method that writes the image data to an mitk::image
  */
class USDiPhASImageSource : public USImageSource
{
public:
  mitkClassMacro(USDiPhASImageSource, USImageSource);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /**
    * Implementation of the superclass method. Returns the pointer
    * to the mitk::Image filled by DiPhAS API callback.
    */
  virtual void GetNextRawImage( mitk::Image::Pointer& );

  /**
    * The API calls this function to pass the image data to the
	* user; here the m_Image is updated
    */
void mitk::USDiPhASImageSource::ImageDataCallback(
    short* rfDataChannelData,
    int& channelDataChannelsPerDataset,
    int& channelDataSamplesPerChannel,
    int& channelDataTotalDatasets,

    short* rfDataArrayBeamformed,
    int& beamformedLines,
    int& beamformedSamples,
    int& beamformedTotalDatasets,

    unsigned char* imageData,
    int& imageWidth,
    int& imageHeight,
	int& imagePixelFormat,
    int& imageSetsTotal,

    double& timeStamp);

protected:
  USDiPhASImageSource( );
  virtual ~USDiPhASImageSource( );
  
  void UpdateImageGeometry();
  
  mitk::Image::Pointer             m_Image;
  itk::FastMutexLock::Pointer      m_ImageMutex;
};
} // namespace mitk

#endif // MITKUSDiPhASImageSource_H
