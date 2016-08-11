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

#include "mitkUSDiPhASImageSource.h"
#include "Framework.IBMT.US.CWrapper.h"
#include "mitkImageReadAccessor.h"

mitk::USDiPhASImageSource::USDiPhASImageSource()
  : m_Image(mitk::Image::New()),
  m_ImageMutex(itk::FastMutexLock::New())
{

}

mitk::USDiPhASImageSource::~USDiPhASImageSource( )
{

}

void mitk::USDiPhASImageSource::GetNextRawImage( mitk::Image::Pointer& image)
{
  if ( image.IsNull() ) { image = mitk::Image::New(); }

  // write the data of m_Image to image
  if ( m_Image->IsInitialized() )
  {
    m_ImageMutex->Lock();

    // copy contents of the given image into the member variable
    image->Initialize(m_Image->GetPixelType(), m_Image->GetDimension(), m_Image->GetDimensions());
    mitk::ImageReadAccessor inputReadAccessor(m_Image, m_Image->GetSliceData(0,0,0));
    image->SetSlice(inputReadAccessor.GetData());
    image->SetGeometry(m_Image->GetGeometry());

    m_ImageMutex->Unlock();
  }
}

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
    int& imageBytesPerPixel,
    int& imageSetsTotal,

    double& timeStamp)
{
  /*if (imageData != nullptr && !m_Image.IsNull())
  {
    if ( m_ImageMutex.IsNotNull() ) { m_ImageMutex->Lock(); }

    // initialize mitk::Image with given image size on the first time
    if ( ! m_Image->IsInitialized() )
    {
      unsigned int dim[]={imageWidth, imageHeight}; // image dimensions

      m_Image->Initialize(mitk::MakeScalarPixelType<char>(), 2, dim);
    }

    // lock the image for writing an copy the given buffer into the image then
    m_Image->SetSlice(imageData);

    if ( m_ImageMutex.IsNotNull() ) { m_ImageMutex->Unlock(); }
  }*/
	MITK_INFO << "another info 4 u";
}


void UpdateImageGeometry(int imageWidth, int imageHeight)
{
  mitk::Vector3D spacing;
  spacing[0] = 1; //conversion: meters to millimeters
  spacing[1] = 1; //conversion: meters to millimeters
  spacing[2] = 1;
}