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
#include "mitkToFOpenCVImageGrabber.h"

// mitk includes
#include "mitkImageDataItem.h"
#include <mitkImageStatisticsHolder.h>
#include "mitkImageReadAccessor.h"

#include "vtkSmartPointer.h"
#include "vtkColorTransferFunction.h"
#include "vtkFloatArray.h"


namespace mitk
{
  ToFOpenCVImageGrabber::ToFOpenCVImageGrabber()
  {
    m_CurrentOpenCVIntensityImage = NULL;
    m_CurrentOpenCVAmplitudeImage = NULL;
    m_CurrentOpenCVDistanceImage = NULL;
    m_ImageType = 0;
    m_ImageDepth = IPL_DEPTH_32F;
    m_ImageGrabber = NULL;
  }

  ToFOpenCVImageGrabber::~ToFOpenCVImageGrabber()
  {
  }

  cv::Mat ToFOpenCVImageGrabber::GetImage()
  {
    m_ImageGrabber->Update();
    unsigned int numOfPixel = m_ImageGrabber->GetCaptureWidth()*m_ImageGrabber->GetCaptureHeight();
    // copy current mitk images
    unsigned int dimensions[4];
    dimensions[0] = this->m_ImageGrabber->GetCaptureWidth();
    dimensions[1] = this->m_ImageGrabber->GetCaptureHeight();
    dimensions[2] = 1;
    dimensions[3] = 1;

    // create single component float pixel type
    mitk::PixelType FloatType = MakeScalarPixelType<float>();

    ImageReadAccessor imgGrabAcc0(m_ImageGrabber->GetOutput(0), m_ImageGrabber->GetOutput(0)->GetSliceData());
    ImageReadAccessor imgGrabAcc1(m_ImageGrabber->GetOutput(1), m_ImageGrabber->GetOutput(1)->GetSliceData());
    ImageReadAccessor imgGrabAcc2(m_ImageGrabber->GetOutput(2), m_ImageGrabber->GetOutput(2)->GetSliceData());

    mitk::Image::Pointer currentMITKIntensityImage = mitk::Image::New();
    currentMITKIntensityImage->Initialize(FloatType, 2, dimensions);
    currentMITKIntensityImage->SetSlice((float*) imgGrabAcc2.GetData(),0,0,0);

    mitk::Image::Pointer currentMITKAmplitudeImage = mitk::Image::New();
    currentMITKAmplitudeImage->Initialize(FloatType, 2, dimensions);
    currentMITKAmplitudeImage->SetSlice((float*)imgGrabAcc1.GetData(),0,0,0);

    mitk::Image::Pointer currentMITKDistanceImage = mitk::Image::New();
    currentMITKDistanceImage->Initialize(FloatType, 2, dimensions);
    currentMITKDistanceImage->SetSlice((float*)imgGrabAcc0.GetData(),0,0,0);
    // copy mitk images to OpenCV images
    if (m_ImageDepth==IPL_DEPTH_32F)
    {
      if (m_ImageType==1)
      {
        ImageReadAccessor currentAmplAcc(currentMITKAmplitudeImage, currentMITKAmplitudeImage->GetSliceData(0, 0, 0));
        float* amplitudeFloatData = (float*) currentAmplAcc.GetData();
        memcpy(m_CurrentOpenCVAmplitudeImage->imageData,(unsigned char*)amplitudeFloatData,numOfPixel*sizeof(float));
        cv::Mat image(m_CurrentOpenCVAmplitudeImage);
        return image;
      }
      else if (m_ImageType==2)
      {
        ImageReadAccessor currentIntenAcc(currentMITKIntensityImage, currentMITKIntensityImage->GetSliceData(0, 0, 0));
        float* intensityFloatData = (float*) currentIntenAcc.GetData();
        memcpy(m_CurrentOpenCVIntensityImage->imageData,(unsigned char*)intensityFloatData,numOfPixel*sizeof(float));
        cv::Mat image(m_CurrentOpenCVIntensityImage);
        return image;
      }
      else
      {
        ImageReadAccessor currentDistAcc(currentMITKDistanceImage, currentMITKDistanceImage->GetSliceData(0, 0, 0));
        float* distanceFloatData = (float*) currentDistAcc.GetData();
        memcpy(m_CurrentOpenCVDistanceImage->imageData,(unsigned char*)distanceFloatData,numOfPixel*sizeof(float));
        cv::Mat image(m_CurrentOpenCVDistanceImage);
        return image;
      }
    }
    else
    {
      if (m_ImageType==1)
      {
        this->MapScalars(currentMITKAmplitudeImage, m_CurrentOpenCVAmplitudeImage);
        cv::Mat image(m_CurrentOpenCVAmplitudeImage);
        return image;
      }
      else if (m_ImageType==2)
      {
        this->MapScalars(currentMITKIntensityImage, m_CurrentOpenCVIntensityImage);
        cv::Mat image(m_CurrentOpenCVIntensityImage);
        return image;
      }
      else
      {
        this->MapScalars(currentMITKDistanceImage, m_CurrentOpenCVDistanceImage);
        cv::Mat image(m_CurrentOpenCVDistanceImage);
        return image;
      }
    }
  }

  void ToFOpenCVImageGrabber::SetImageType(unsigned int imageType)
  {
    m_ImageType = imageType;
  }

  void ToFOpenCVImageGrabber::SetImageDepth(unsigned int imageDepth)
  {
    m_ImageDepth = imageDepth;
  }

  void ToFOpenCVImageGrabber::SetToFImageGrabber(ToFImageGrabber::Pointer imageGrabber)
  {
    m_ImageGrabber = imageGrabber;
  }

  ToFImageGrabber::Pointer ToFOpenCVImageGrabber::GetToFImageGrabber()
  {
    return m_ImageGrabber;
  }

  void ToFOpenCVImageGrabber::StartCapturing()
  {
    if (m_ImageGrabber.IsNotNull())
    {
      m_ImageGrabber->ConnectCamera();
      //Initialize cv Images after the camera is conneceted and we know the resolution
      m_CurrentOpenCVIntensityImage = cvCreateImage(cvSize(m_ImageGrabber->GetCaptureWidth(), m_ImageGrabber->GetCaptureHeight()), m_ImageDepth, 1);
      m_CurrentOpenCVAmplitudeImage = cvCreateImage(cvSize(m_ImageGrabber->GetCaptureWidth(), m_ImageGrabber->GetCaptureHeight()), m_ImageDepth, 1);
      m_CurrentOpenCVDistanceImage = cvCreateImage(cvSize(m_ImageGrabber->GetCaptureWidth(), m_ImageGrabber->GetCaptureHeight()), m_ImageDepth, 1);
      m_ImageGrabber->StartCamera();
    }
  }

  void ToFOpenCVImageGrabber::StopCapturing()
  {
    if (m_ImageGrabber.IsNotNull())
    {
      m_ImageGrabber->StopCamera();
      m_ImageGrabber->DisconnectCamera();
    }
  }

  void ToFOpenCVImageGrabber::MapScalars( mitk::Image::Pointer mitkImage, IplImage* openCVImage)
  {
    unsigned int numOfPixel = m_ImageGrabber->GetCaptureWidth()*m_ImageGrabber->GetCaptureHeight();
    ImageReadAccessor imgAcc(mitkImage, mitkImage->GetSliceData(0, 0, 0));
    float* floatData = (float*)imgAcc.GetData();
    vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
    vtkSmartPointer<vtkFloatArray> floatArrayInt = vtkSmartPointer<vtkFloatArray>::New();
    floatArrayInt->Initialize();
    floatArrayInt->SetArray(floatData, numOfPixel, 0);
    mitk::ScalarType min = mitkImage->GetStatistics()->GetScalarValueMin();
    mitk::ScalarType max = mitkImage->GetStatistics()->GetScalarValueMaxNoRecompute();
    MITK_INFO<<"Minimum: "<<min;
    MITK_INFO<<"Maximum: "<<max;
    colorTransferFunction->RemoveAllPoints();
    colorTransferFunction->AddRGBPoint(min, 0, 0, 0);
    colorTransferFunction->AddRGBPoint(max, 1, 1, 1);
    colorTransferFunction->SetColorSpaceToHSV();
    colorTransferFunction->MapScalarsThroughTable(floatArrayInt, (unsigned char*)openCVImage->imageData, VTK_LUMINANCE);
  }

} // end namespace mitk
