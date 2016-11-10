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
  m_UseBModeFilterModified(false),
  m_UseBModeFilterNext(false)
{
  m_ImageMutex = itk::FastMutexLock::New();

  m_BufferSize = 100;

  m_LastWrittenImage = m_BufferSize - 1;
  m_ImageBuffer.insert(m_ImageBuffer.end(), m_BufferSize, nullptr);
}

mitk::USDiPhASImageSource::~USDiPhASImageSource( )
{
}

void mitk::USDiPhASImageSource::GetNextRawImage( mitk::Image::Pointer& image)
{
  // we get this image pointer from the USDevice and write into it the data we got from the DiPhAS API

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

  image = &(*m_ImageBuffer[m_LastWrittenImage]);
  //m_ImageBuffer.at(m_LastWrittenImage) = nullptr;
  
  if (image != nullptr)
  {
    // do image processing before displaying it
    if (useBModeFilter && m_DataType == DataType::Beamformed_Short)
    {
      // apply BmodeFilter to the image
      image = ApplyBmodeFilter(image);
    }
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
  //MITK_INFO << "Applying BMode Filter";
  // the image needs to be of floating point type for the envelope filter to work; the casting is done automatically by the CastToItkImage
  typedef itk::Image< float, 3 > itkInputImageType;
  typedef itk::Image< short, 3 > itkOutputImageType;
  typedef itk::PhotoacousticBModeImageFilter < itkInputImageType, itkOutputImageType > PhotoacousticBModeImageFilter;

  PhotoacousticBModeImageFilter::Pointer photoacousticBModeFilter = PhotoacousticBModeImageFilter::New();
  itkInputImageType::Pointer itkImage;

  mitk::CastToItkImage(inputImage, itkImage);
  photoacousticBModeFilter->SetInput(itkImage);
  photoacousticBModeFilter->SetDirection(0);
  //MITK_INFO << "Done Applying BMode Filter";
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
  cout << "call";
  bool writeImage = ((m_DataType == DataType::Image_uChar) && (imageData != nullptr)) || ((m_DataType == DataType::Beamformed_Short) && (rfDataArrayBeamformed != nullptr)) && !m_Image.IsNull();
  if (writeImage)
  {
    // create a new image and initialize it
    mitk::Image::Pointer image = mitk::Image::New();
    cout << m_LastWrittenImage << '\n';

    switch (m_DataType)
    {
      case DataType::Image_uChar: {
        m_ImageDimensions[0] = imageWidth;
        m_ImageDimensions[1] = imageHeight;
        m_ImageDimensions[2] = imageSetsTotal;
        image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, m_ImageDimensions);
        break;
      }
      case DataType::Beamformed_Short: {
        m_ImageDimensions[0] = beamformedLines;
        m_ImageDimensions[1] = beamformedSamples;
        m_ImageDimensions[2] = beamformedTotalDatasets;
        image->Initialize(mitk::MakeScalarPixelType<short>(), 3, m_ImageDimensions);
        break;
      }
    }
    image->GetGeometry()->SetSpacing(m_ImageSpacing);
    image->GetGeometry()->Modified();

    // write the given buffer into the image
    switch (m_DataType)
    {
    case DataType::Image_uChar: {
        for (int i = 0; i < imageSetsTotal; i++) {
          image->SetSlice(&imageData[i*imageHeight*imageWidth], i);
        }
        break;
      }

    case DataType::Beamformed_Short: {
        short* flipme = new short[beamformedLines*beamformedSamples*beamformedTotalDatasets];
        int pixelsPerImage = beamformedLines*beamformedSamples;

        for (int currentSet = 0; currentSet < beamformedTotalDatasets; currentSet++)
        {
        
            for (int sample = 0; sample < beamformedSamples; sample++)
            {
              for (int line = 0; line < beamformedLines; line++)
              {
                flipme[sample*beamformedLines + line + pixelsPerImage*currentSet]
                  = rfDataArrayBeamformed[line*beamformedSamples + sample + pixelsPerImage*currentSet];
              }
            } // the beamformed pa image is flipped by 90 degrees; we need to flip it manually
        }
        
        for (int i = 0; i < beamformedTotalDatasets; i++) {
          image->SetSlice(&flipme[i*beamformedLines*beamformedSamples], i);
          // set every image to a different slice
        }

        delete[] flipme;
        break;
      }
    }

    // if the user decides to start recording, we feed the vector the generated images
    if (currentlyRecording) {
      for (int index = 0; index < image->GetDimension(2); ++index)
      {
        if (m_Image->IsSliceSet(index))
        {
          m_recordedImages.push_back(Image::New());
          m_recordedImages.back()->Initialize(image->GetPixelType(), 2, image->GetDimensions());
          m_recordedImages.back()->SetGeometry(image->GetGeometry());

          mitk::ImageReadAccessor inputReadAccessor(image, image->GetSliceData(index));
          m_recordedImages.back()->SetSlice(inputReadAccessor.GetData());
        }
      }
    }
    m_ImageBuffer.at((m_LastWrittenImage+1)%m_BufferSize) = image;
    m_LastWrittenImage = (m_LastWrittenImage + 1) % m_BufferSize;
    cout << "end" << '\n';
  }
}

void mitk::USDiPhASImageSource::UpdateImageGeometry()
{
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
      m_ImageSpacing[0] = pitch * reconstructionLines / imageWidth;
      m_ImageSpacing[1] = recordTime * speedOfSound / 2 * 1000 / imageHeight;
      break;
    }
    case DataType::Beamformed_Short : {
      int& imageWidth = reconstructionLines;
      int& imageHeight = m_device->GetScanMode().reconstructionSamplesPerLine;
      m_ImageSpacing[0] = pitch;
      m_ImageSpacing[1] = recordTime * speedOfSound / 2 * 1000 / imageHeight;
      break;
    }
  }
  m_ImageSpacing[2] = 0.6;

  MITK_INFO << "Retreaving Image Geometry Information for Spacing " << m_ImageSpacing[0] << " ... " << m_ImageSpacing[1] << " ... " << m_ImageSpacing[2] << " ...[DONE]";
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
      break;
    case DataType::Beamformed_Short :
      MITK_INFO << "samples: " << m_device->GetScanMode().reconstructionSamplesPerLine << " lines: " << m_device->GetScanMode().reconstructionLines;
      break;
    }
  }
}

void mitk::USDiPhASImageSource::SetGUIOutput(std::function<void(QString)> out)
{
  USDiPhASImageSource::m_GUIOutput = out;
  startTime = ((float)std::clock()) / CLOCKS_PER_SEC; //wait till the callback is available again
  useGUIOutPut = false;
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
  unsigned int width  = 32;
  unsigned int height = 32;
  unsigned int events = m_device->GetScanMode().transmitEventsCount + 1; // the laser event is not included in the transmitEvents, so we add 1 here

  if (m_DataType == DataType::Beamformed_Short)
  {
    width = m_device->GetScanMode().reconstructionLines;
    height = m_device->GetScanMode().reconstructionSamplesPerLine;
  }
  else if (m_DataType == DataType::Image_uChar)
  {
    width = m_device->GetScanMode().imageWidth;
    height = m_device->GetScanMode().imageHeight;
  }

  unsigned int dimLaser[] = { width, height, m_recordedImages.size() / events};
  unsigned int dimSound[] = { width, height, m_recordedImages.size() / events * (events-1)};

  LaserImage->Initialize(m_recordedImages.back()->GetPixelType(), 3, dimLaser);
  LaserImage->SetGeometry(m_recordedImages.back()->GetGeometry());

  SoundImage->Initialize(m_recordedImages.back()->GetPixelType(), 3, dimSound);
  SoundImage->SetGeometry(m_recordedImages.back()->GetGeometry());

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
  unsigned int width  = 32;
  unsigned int height = 32;
  unsigned int events = m_device->GetScanMode().transmitEventsCount;

  if (m_DataType == DataType::Beamformed_Short)
  {
    width = m_device->GetScanMode().reconstructionLines;
    height = m_device->GetScanMode().reconstructionSamplesPerLine;
  }
  else if (m_DataType == DataType::Image_uChar)
  {
    width = m_device->GetScanMode().imageWidth;
    height = m_device->GetScanMode().imageHeight;
  }

  unsigned int dimSound[] = { width, height, m_recordedImages.size()};

  SoundImage->Initialize(m_recordedImages.back()->GetPixelType(), 3, dimSound);
  SoundImage->SetGeometry(m_recordedImages.back()->GetGeometry());

  for (int index = 0; index < m_recordedImages.size(); ++index)
  {
    mitk::ImageReadAccessor inputReadAccessor(m_recordedImages.at(index));
    SoundImage->SetSlice(inputReadAccessor.GetData(), index);
  }
}