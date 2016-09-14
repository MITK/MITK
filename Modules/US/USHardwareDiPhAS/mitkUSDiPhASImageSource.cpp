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
#include "mitkUSDiPhASDevice.h"


mitk::USDiPhASImageSource::USDiPhASImageSource(mitk::USDiPhASDevice* device)
	: m_Image(mitk::Image::New()),
	m_ImageMutex(itk::FastMutexLock::New()),
  m_device(device),
  startTime(((float)std::clock()) / CLOCKS_PER_SEC),
  useGUIOutPut(false),
  DataType(0)
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

  /*if (!useGUIOutPut && m_GUIOutput) {
    // Need to do this because the program initializes the GUI twice
    // this is probably an MITK bug, if it's fixed the timing becomes unneccesary
    float timePassed = ((float)std::clock()) / CLOCKS_PER_SEC - startTime;
    if (timePassed > 10)
    {
      useGUIOutPut = true;
    }
  }
  if (useGUIOutPut) {
    getSystemInfo(&BeamformerInfos);

    std::ostringstream s;
    s << "state info: PRF:" << BeamformerInfos.systemPRF << "Hz, datarate: " << BeamformerInfos.dataTransferRateMBit << "MBit/s";

    m_GUIOutput(QString::fromStdString(s.str()));
  }*/ //too performance-heavy
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
  bool writeImage = ((DataType == 0) && (imageData != nullptr)) || ((DataType == 1) && (rfDataArrayBeamformed != nullptr));
  if (!m_Image.IsNull() && writeImage)
  {
    if ( m_ImageMutex.IsNotNull() ) { m_ImageMutex->Lock(); }

    // initialize mitk::Image with given image size on the first time
    if ( ! m_Image->IsInitialized() )
    {
      UpdateImageDataType(imageHeight, imageWidth);
    }

    // lock the image for writing an copy the given buffer into the image then
    switch (DataType)
    {
    case 0: {
      m_Image->SetSlice(imageData);
      break;
    }
    case 1: {
      m_Image->SetSlice(rfDataArrayBeamformed);
      break;
    }
    }
    if ( m_ImageMutex.IsNotNull() ) { m_ImageMutex->Unlock(); }
  }
}

void mitk::USDiPhASImageSource::UpdateImageDataType(int imageHeight, int imageWidth)
{
  unsigned int dim[] = { imageWidth, imageHeight }; // image dimensions
  switch (DataType)
  {
    case 0: {
      m_Image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 2, dim);
      break;
    }
    case 1: {
      m_Image->Initialize(mitk::MakeScalarPixelType<short>(), 2, dim);
      break;
    }
  } // 0:imageData 1:beamformed
  UpdateImageGeometry(); // update the image geometry
  startTime = ((float)std::clock()) / CLOCKS_PER_SEC; //wait till the callback is available again
  useGUIOutPut = false;
}

void mitk::USDiPhASImageSource::UpdateImageGeometry()
{
  mitk::Vector3D spacing;
  float& recordTime        = m_device->GetScanMode().receivePhaseLengthSeconds;
  int& speedOfSound        = m_device->GetScanMode().averageSpeedOfSound;
  int& imageWidth          = m_device->GetScanMode().imageWidth;
  int& imageHeight         = m_device->GetScanMode().imageHeight;
  float& pitch             = m_device->GetScanMode().reconstructedLinePitchMmOrAngleDegree;
  int& reconstructionLines = m_device->GetScanMode().reconstructionLines;

  spacing[0] = (pitch*reconstructionLines)/imageWidth;
  spacing[1] = ((recordTime*speedOfSound/2)*1000)/imageHeight;
  spacing[2] = 1;

  m_ImageMutex->Lock();
  if (m_Image.IsNotNull() && (m_Image->GetGeometry() != NULL))
  {
	  m_Image->GetGeometry()->SetSpacing(spacing);
	  m_Image->GetGeometry()->Modified();
  }
  else
  {
	  MITK_WARN << "image or geometry was NULL, can't adapt geometry";
  }
  m_ImageMutex->Unlock();

  MITK_DEBUG << "UpdateImageGeometry called!";
  MITK_DEBUG << "depth in mm: " << (recordTime*speedOfSound / 2);
  MITK_DEBUG << "new spacing: " << spacing;
}

void mitk::USDiPhASImageSource::setDataType(int DataT)
{
  if (DataT != DataType)
  {
    DataType = DataT;
    UpdateImageDataType(m_device->GetScanMode().imageHeight, m_device->GetScanMode().imageWidth);
  }
}

std::function<void(QString)>  mitk::USDiPhASImageSource::m_GUIOutput = nullptr;

void mitk::USDiPhASImageSource::setGUIOutput(std::function<void(QString)> out)
{
  USDiPhASImageSource::m_GUIOutput = out;
}