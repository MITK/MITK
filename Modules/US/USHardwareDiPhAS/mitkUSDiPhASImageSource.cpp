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

// std dependencies
#include <ctime>

// mitk dependencies
#include "mitkUSDiPhASDevice.h"
#include "mitkUSDiPhASImageSource.h"
#include <mitkIOUtil.h>
#include "mitkUSDiPhASBModeImageFilter.h"
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"

// itk dependencies
#include "itkImage.h"
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkCropImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkIntensityWindowingImageFilter.h"


mitk::USDiPhASImageSource::USDiPhASImageSource(mitk::USDiPhASDevice* device)
	: m_Image(mitk::Image::New()),
  m_device(device),
  startTime(((float)std::clock()) / CLOCKS_PER_SEC),
  useGUIOutPut(false),
  m_DataType(DataType::Image_uChar),
  m_GUIOutput(nullptr),
  useBModeFilter(false),
  currentlyRecording(false),
  m_DataTypeModified(true),
  m_DataTypeNext(DataType::Image_uChar),
  m_UseBModeFilterModified(true),
  m_UseBModeFilterNext(true)
{
}

mitk::USDiPhASImageSource::~USDiPhASImageSource( )
{
}

void mitk::USDiPhASImageSource::GetNextRawImage( mitk::Image::Pointer& image)
{
  // we get this image pointer from the USDevice and write into it the data we got from the DiPhAS API

  m_ImageMutex.lock();
  MITK_INFO << "GNRI: LOCK";
  if (m_Image->IsInitialized())
  {
    //initialize the image the first time
    if (image.IsNull()) {
      image = mitk::Image::New();
      image->Initialize(m_Image->GetPixelType(), m_Image->GetDimension(), m_Image->GetDimensions());
    }

    // write the data of m_Image to image if it changed
    if (image->GetPixelType() != m_Image->GetPixelType() ||
      image->GetDimensions() != m_Image->GetDimensions()) {
      image = mitk::Image::New();
      image->Initialize(m_Image->GetPixelType(), m_Image->GetDimension(), m_Image->GetDimensions());
    }

    // if DataType == 0 : if imageData is given just bypass the filter here, as imageData is already enveloped.
    if (!useBModeFilter || m_DataType == 0)
    {
      // copy contents of the given image into the member variable
      mitk::ImageReadAccessor inputReadAccessor(m_Image, m_Image->GetVolumeData());
      image->SetVolume(inputReadAccessor.GetData());
    }
    else {
      // feed the m_image to the BMode filter and grab it back; 
      if (m_DataType == DataType::Beamformed_Short) {
        image = ApplyBmodeFilter(m_Image);
      }
    }
    // always copy the geometry from the m_Image
    image->SetGeometry(m_Image->GetGeometry());
  }

  if (!useGUIOutPut && m_GUIOutput) {
    // Need to do this because the program initializes the GUI twice
    // this is probably a bug in UltrasoundSupport, if it's fixed the timing becomes unneccesary
    float timePassed = ((float)std::clock()) / CLOCKS_PER_SEC - startTime;
    if (timePassed > 10)
    {
      useGUIOutPut = true;
    }
  }
  if (useGUIOutPut) {
    // pass some beamformer state infos to the GUI
    getSystemInfo(&BeamformerInfos);

    std::ostringstream s;
    s << "state info: PRF:" << BeamformerInfos.systemPRF << "Hz, datarate: " << BeamformerInfos.dataTransferRateMBit << "MBit/s";

    m_GUIOutput(QString::fromStdString(s.str()));
  }

  m_ImageMutex.unlock();
  MITK_INFO << "GNRI: UNLOCK";
}

mitk::Image::Pointer mitk::USDiPhASImageSource::ApplyBmodeFilter2d(mitk::Image::Pointer inputImage)
{
  // we use this seperate ApplyBmodeFilter Method for processing of images that are to be saved later on (see SetRecordingStatus(bool)).

  // the image needs to be of floating point type for the envelope filter to work; the casting is done automatically by the CastToItkImage
  typedef itk::Image< float, 2 > itkInputImageType;
  typedef itk::Image< short, 2 > itkOutputImageType;
  typedef itk::PhotoacousticBModeImageFilter < itkInputImageType, itkOutputImageType > PhotoacousticBModeImageFilter;

  PhotoacousticBModeImageFilter::Pointer photoacousticBModeFilter = PhotoacousticBModeImageFilter::New();
  itkInputImageType::Pointer itkImage;

  mitk::CastToItkImage(inputImage, itkImage);
  photoacousticBModeFilter->SetInput(itkImage);
  photoacousticBModeFilter->SetDirection(0);
  return mitk::GrabItkImageMemory(photoacousticBModeFilter->GetOutput());
}

mitk::Image::Pointer mitk::USDiPhASImageSource::ApplyBmodeFilter(mitk::Image::Pointer inputImage)
{
  MITK_INFO << "Applying BMode Filter";
  // the image needs to be of floating point type for the envelope filter to work; the casting is done automatically by the CastToItkImage
  typedef itk::Image< float, 3 > itkInputImageType;
  typedef itk::Image< short, 3 > itkOutputImageType;
  typedef itk::PhotoacousticBModeImageFilter < itkInputImageType, itkOutputImageType > PhotoacousticBModeImageFilter;

  PhotoacousticBModeImageFilter::Pointer photoacousticBModeFilter = PhotoacousticBModeImageFilter::New();
  itkInputImageType::Pointer itkImage;

  mitk::CastToItkImage(inputImage, itkImage);
  photoacousticBModeFilter->SetInput(itkImage);
  photoacousticBModeFilter->SetDirection(0);
  MITK_INFO << "Done Applying BMode Filter";
  return mitk::GrabItkImageMemory(photoacousticBModeFilter->GetOutput());
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
  bool writeImage = ((m_DataType == DataType::Image_uChar) && (imageData != nullptr)) || ((m_DataType == DataType::Beamformed_Short) && (rfDataArrayBeamformed != nullptr)) && !m_Image.IsNull();
  if (writeImage)
  {
    m_ImageMutex.lock();
    MITK_INFO << "CB: LOCK";

    if (m_DataTypeModified)
    {
      SetDataType(m_DataTypeNext);
      m_DataTypeModified = false;
    }

    if (m_UseBModeFilterModified)
    {
      SetUseBModeFilter(m_UseBModeFilterNext);
      m_UseBModeFilterModified = false;
    }

    // lock the image for writing an copy the given buffer into the image then
    switch (m_DataType)
    {
    case DataType::Image_uChar: {
        // initialize mitk::Image with given image size on the first time
        if (!m_Image->IsInitialized())
        {
          UpdateImageDataType(imageHeight, imageWidth);     // update data type and image pixel dimensions
        }
        for (int i = 0; i < imageSetsTotal; i++) {
          m_Image->SetSlice(&imageData[i*imageHeight*imageWidth], i);
        }
        break;
      }

    case DataType::Beamformed_Short: {
        short* flipme = new short[beamformedLines*beamformedSamples*beamformedTotalDatasets];
        int pixelsPerImage = beamformedLines*beamformedSamples;

        /*MITK_INFO << "Debug Garbage:";
        MITK_INFO << "Datasets: " << beamformedTotalDatasets;
        MITK_INFO << "Beamformed Lines: " << beamformedLines;
        MITK_INFO << "Beamformed Samples: " << beamformedSamples;*/
        // initialize mitk::Image with given image size on the first time
        if (!m_Image->IsInitialized())
        {
          UpdateImageDataType(beamformedSamples, beamformedLines);     // update data type and image pixel dimensions
        }

        for (int currentSet = 0; currentSet < beamformedTotalDatasets; currentSet++)
        {
        
            for (int sample = 0; sample < beamformedSamples; sample++)
            {
              for (int line = 0; line < beamformedLines; line++)
              {
                //if (currentSet == 0)
                //{
                  flipme[sample*beamformedLines + line + pixelsPerImage*currentSet]
                    = rfDataArrayBeamformed[line*beamformedSamples + sample + pixelsPerImage*currentSet];
                //}
                //else
                //{
                //  flipme[sample*beamformedLines + line + pixelsPerImage*currentSet]
                //    = rfDataArrayBeamformed[sample*beamformedLines + line + pixelsPerImage*currentSet];
                //}
              }
            } // the beamformed pa image is flipped by 90 degrees; we need to flip it manually
          
        }
        
        for (int i = 0; i < beamformedTotalDatasets; i++) {
          m_Image->SetSlice(&flipme[i*beamformedLines*beamformedSamples], i);
          // set every image to a different slice
        }
        delete[] flipme;
        break;
      }
    }

    // if the user decides to start recording, we feed the vector the generated images
    if (currentlyRecording) {
      for (int index = 0; index < m_Image->GetDimension(2); ++index)
      {
        if (m_Image->IsSliceSet(index))
        {
          m_recordedImages.push_back(Image::New());
          m_recordedImages.back()->Initialize(m_Image->GetPixelType(), 2, m_Image->GetDimensions());
          m_recordedImages.back()->SetGeometry(m_Image->GetGeometry());

          mitk::ImageReadAccessor inputReadAccessor(m_Image, m_Image->GetSliceData(index));
          m_recordedImages.back()->SetSlice(inputReadAccessor.GetData());
        }
      }
    }

    //MITK_INFO << "CB: UNLOCK";
    m_ImageMutex.unlock();
  }
}

void mitk::USDiPhASImageSource::UpdateImageDataType(int imageHeight, int imageWidth)
{
  
  int addEvents = 0;
  if (m_device->IsInterleaved())
    addEvents = 1;
  unsigned int dim[] = { imageWidth, imageHeight, m_device->GetScanMode().transmitEventsCount + addEvents }; // image dimensions; every image needs a seperate slice!

  
  m_Image = mitk::Image::New();

  MITK_INFO << "Initializing image...";
  switch (m_DataType)
  {
  case DataType::Image_uChar : {
      m_Image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dim);
      break;
    }
  case DataType::Beamformed_Short: {
      m_Image->Initialize(mitk::MakeScalarPixelType<short>(), 3, dim);
      break;
    }
  } // 0:imageData 1:beamformed
  MITK_INFO << "Initializing image...[DOINE]";

  MITK_INFO << "Updating geometry...";
  UpdateImageGeometry();                            // update the image geometry
  MITK_INFO << "Updating geometry...[DOINE]";

  startTime = ((float)std::clock()) / CLOCKS_PER_SEC; //wait till the callback is available again
  useGUIOutPut = false;
}

void mitk::USDiPhASImageSource::UpdateImageGeometry()
{
  mitk::Vector3D spacing;
  MITK_INFO << "Retreaving Image Geometry Information for Spacing...";
  float& recordTime        = m_device->GetScanMode().receivePhaseLengthSeconds;
  int& speedOfSound        = m_device->GetScanMode().averageSpeedOfSound;
  float& pitch             = m_device->GetScanMode().reconstructedLinePitchMmOrAngleDegree;
  int& reconstructionLines = m_device->GetScanMode().reconstructionLines;

  switch (m_DataType)
  {
  case DataType::Image_uChar : {
    int& imageWidth = m_device->GetScanMode().imageWidth;
    int& imageHeight = m_device->GetScanMode().imageHeight;
    spacing[0] = pitch * reconstructionLines / imageWidth;
    spacing[1] = recordTime * speedOfSound / 2 * 1000 / imageHeight;
    break;
  }
  case DataType::Beamformed_Short : {
    int& imageWidth = reconstructionLines;
    int& imageHeight = m_device->GetScanMode().reconstructionSamplesPerLine;
    spacing[0] = pitch;
    spacing[1] = recordTime * speedOfSound / 2 * 1000 / imageHeight;
    break;
  }
  } // 0:imageData 1:beamformed

  MITK_INFO << "Retreaving Image Geometry Information for Spacing " << spacing[0] << " ... " << spacing[1] << " ... " << spacing[2] << " ...[DONE]";

  spacing[2] = 0.6;
  //recalculate correct spacing

  if (m_Image.IsNotNull() && (m_Image->GetGeometry() != NULL))
  {
	  m_Image->GetGeometry()->SetSpacing(spacing);
	  m_Image->GetGeometry()->Modified();
  }
  else
  {
	  MITK_WARN << "image or geometry was NULL, can't adapt geometry";
  }
}

void mitk::USDiPhASImageSource::ModifyDataType(DataType DataT)
{
  m_DataTypeModified = true;
  m_DataTypeNext = DataT;
}

void mitk::USDiPhASImageSource::ModifyUseBModeFilter(bool isSet)
{
  m_UseBModeFilterModified = true;
  m_UseBModeFilterNext = isSet;
}

void mitk::USDiPhASImageSource::SetDataType(DataType DataT)
{
  if (DataT != m_DataType)
  {
    m_DataType = DataT;
    MITK_INFO << "Setting new DataType..." << DataT;
    switch (m_DataType)
    {
    case DataType::Image_uChar :
      MITK_INFO << "height: " << m_device->GetScanMode().imageHeight << " width: " << m_device->GetScanMode().imageWidth;
      UpdateImageDataType(m_device->GetScanMode().imageHeight, m_device->GetScanMode().imageWidth);
      break;
    case DataType::Beamformed_Short :
      MITK_INFO << "samples: " << m_device->GetScanMode().reconstructionSamplesPerLine << " lines: " << m_device->GetScanMode().reconstructionLines;
      UpdateImageDataType(m_device->GetScanMode().reconstructionSamplesPerLine, m_device->GetScanMode().reconstructionLines);
      break;
    }
    MITK_INFO << "Setting new DataType...[DOINE]";
  }
}

void mitk::USDiPhASImageSource::SetGUIOutput(std::function<void(QString)> out)
{
  USDiPhASImageSource::m_GUIOutput = out;
}

void mitk::USDiPhASImageSource::SetUseBModeFilter(bool isSet)
{
  useBModeFilter = isSet;
}

// this is just a little function to set the filenames below right
inline void replaceAll(std::string& str, const std::string& from, const std::string& to) {
  if (from.empty())
    return;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
}

void mitk::USDiPhASImageSource::SetRecordingStatus(bool record)
{
  // start the recording process
  if (record)
  {
    currentlyRecording = true;
    m_recordedImages.clear();  // we make sure there are no leftovers
  }
  // save images, end recording, and clean up
  else
  {
    currentlyRecording = false;

    // get the time and date, put them into a nice string and create a folder for the images
    time_t time = std::time(nullptr);
    time_t* timeptr = &time;
    std::string currentDate = std::ctime(timeptr);
    replaceAll(currentDate, ":", "-");
    currentDate.pop_back();
    std::string MakeFolder = "mkdir \"c:/DiPhASImageData/" + currentDate + "\"";
    system(MakeFolder.c_str());

    // if checked, we add the bmode filter here
    for (int index = 0; index < m_recordedImages.size(); ++index)
    {
      if (m_DataType == 1 && useBModeFilter)
        m_recordedImages.at(index) = ApplyBmodeFilter2d(m_recordedImages.at(index));
    }

    Image::Pointer LaserImage = Image::New();
    Image::Pointer SoundImage = Image::New();
    std::string pathLaser = "c:\\DiPhASImageData\\" + currentDate + "\\" + "LaserImages" + ".nrrd";
    std::string pathSound = "c:\\DiPhASImageData\\" + currentDate + "\\" + "USImages" + ".nrrd";
    
    // order the images and save them
    if (m_device->GetScanMode().beamformingAlgorithm == (int)Beamforming::Interleaved_OA_US) // save a LaserImage if we used interleaved mode
    {
      OrderImagesInterleaved(LaserImage, SoundImage);
      mitk::IOUtil::Save(SoundImage, pathSound);
      mitk::IOUtil::Save(LaserImage, pathLaser);
    }
    else if (m_device->GetScanMode().beamformingAlgorithm == (int)Beamforming::PlaneWaveCompound) // save no LaserImage if we used US only mode
    {
      OrderImagesUltrasound(SoundImage);
      mitk::IOUtil::Save(SoundImage, pathSound);
    }

    m_recordedImages.clear(); // clean up the images
  }
}


void mitk::USDiPhASImageSource::OrderImagesInterleaved(Image::Pointer LaserImage, Image::Pointer SoundImage)
{
  unsigned int width  = m_device->GetScanMode().imageWidth;
  unsigned int height = m_device->GetScanMode().imageHeight;
  unsigned int events = m_device->GetScanMode().transmitEventsCount + 1; // the laser event is not included in the transmitEvents, so we add 1 here

  unsigned int dimLaser[] = { width, height, m_recordedImages.size() / events};
  unsigned int dimSound[] = { width, height, m_recordedImages.size() / events * (events-1)};

  LaserImage->Initialize(m_Image->GetPixelType(), 3, dimLaser);
  LaserImage->SetGeometry(m_Image->GetGeometry());

  SoundImage->Initialize(m_Image->GetPixelType(), 3, dimSound);
  SoundImage->SetGeometry(m_Image->GetGeometry());

  for (int index = 0; index < m_recordedImages.size(); ++index)
  {
    mitk::ImageReadAccessor inputReadAccessor(m_recordedImages.at(index));
    if (index % events == 0)
    {
      LaserImage->SetSlice(inputReadAccessor.GetData(), index / events);
    }
    else
    {
      SoundImage->SetSlice(inputReadAccessor.GetData(), ((index - (index % events)) / events) + (index % events)-1);
    }
  }
}

void mitk::USDiPhASImageSource::OrderImagesUltrasound(Image::Pointer SoundImage)
{
  unsigned int width  = m_device->GetScanMode().imageWidth;
  unsigned int height = m_device->GetScanMode().imageHeight;
  unsigned int events = m_device->GetScanMode().transmitEventsCount;

  unsigned int dimSound[] = { width, height, m_recordedImages.size()};

  SoundImage->Initialize(m_Image->GetPixelType(), 3, dimSound);
  SoundImage->SetGeometry(m_Image->GetGeometry());

  for (int index = 0; index < m_recordedImages.size(); ++index)
  {
    mitk::ImageReadAccessor inputReadAccessor(m_recordedImages.at(index));
    SoundImage->SetSlice(inputReadAccessor.GetData(), index);
  }
}