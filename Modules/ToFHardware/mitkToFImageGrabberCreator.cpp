/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "mitkToFImageGrabberCreator.h"
#include "mitkToFImageGrabber.h"
#include "mitkToFCameraPMDCamCubeDevice.h"
#include "mitkToFCameraPMDRawDataCamCubeDevice.h"
#include "mitkToFCameraPMDCamBoardDevice.h"
#include "mitkToFCameraPMDRawDataCamBoardDevice.h"
#include "mitkToFCameraPMDO3Device.h"
#include "mitkToFCameraPMDPlayerDevice.h"
#include "mitkToFCameraPMDMITKPlayerDevice.h"
#include "mitkToFCameraMITKPlayerDevice.h"
#include "mitkToFCameraMESASR4000Device.h"
#include "mitkKinectDevice.h"

namespace mitk
{

  ToFImageGrabberCreator::Pointer ToFImageGrabberCreator::s_Instance = NULL;

  ToFImageGrabberCreator::ToFImageGrabberCreator()
  {
    m_ToFImageGrabber = mitk::ToFImageGrabber::New();
  }

  ToFImageGrabberCreator::~ToFImageGrabberCreator()
  {
  }

  ToFImageGrabberCreator* ToFImageGrabberCreator::GetInstance()
  {
    if ( !ToFImageGrabberCreator::s_Instance )
    {
      s_Instance = ToFImageGrabberCreator::New();
    }
    return s_Instance;
  }

  ToFImageGrabber::Pointer ToFImageGrabberCreator::GetPMDCamCubeImageGrabber()
  {
    m_ToFCameraDevice = mitk::ToFCameraPMDCamCubeDevice::New();
    m_ToFImageGrabber->SetCameraDevice(m_ToFCameraDevice);
    return m_ToFImageGrabber;
  }

  ToFImageGrabber::Pointer ToFImageGrabberCreator::GetPMDRawDataCamCubeImageGrabber()
  {
    m_ToFCameraDevice = mitk::ToFCameraPMDRawDataCamCubeDevice::New();
    m_ToFImageGrabber->SetCameraDevice(m_ToFCameraDevice);
    return m_ToFImageGrabber;
  }

  ToFImageGrabber::Pointer ToFImageGrabberCreator::GetPMDO3ImageGrabber()
  {
    m_ToFCameraDevice = mitk::ToFCameraPMDO3Device::New();
    m_ToFImageGrabber->SetCameraDevice(m_ToFCameraDevice);
    return m_ToFImageGrabber;
  }

  ToFImageGrabber::Pointer ToFImageGrabberCreator::GetPMDCamBoardImageGrabber()
  {
    m_ToFCameraDevice = mitk::ToFCameraPMDCamBoardDevice::New();
    m_ToFImageGrabber->SetCameraDevice(m_ToFCameraDevice);
    return m_ToFImageGrabber;
  }

  ToFImageGrabber::Pointer ToFImageGrabberCreator::GetPMDRawDataCamBoardImageGrabber()
  {
    m_ToFCameraDevice = mitk::ToFCameraPMDRawDataCamBoardDevice::New();
    m_ToFImageGrabber->SetCameraDevice(m_ToFCameraDevice);
    return m_ToFImageGrabber;
  }

  ToFImageGrabber::Pointer ToFImageGrabberCreator::GetPMDPlayerImageGrabber()
  {
    m_ToFCameraDevice = mitk::ToFCameraPMDPlayerDevice::New();
    m_ToFImageGrabber->SetCameraDevice(m_ToFCameraDevice);
    return m_ToFImageGrabber;
  }

  ToFImageGrabber::Pointer ToFImageGrabberCreator::GetPMDMITKPlayerImageGrabber()
  {
    m_ToFCameraDevice = mitk::ToFCameraPMDMITKPlayerDevice::New();
    m_ToFImageGrabber->SetCameraDevice(m_ToFCameraDevice);
    return m_ToFImageGrabber;
  }

  ToFImageGrabber::Pointer ToFImageGrabberCreator::GetMITKPlayerImageGrabber()
  {
    m_ToFCameraDevice = mitk::ToFCameraMITKPlayerDevice::New();
    m_ToFImageGrabber->SetCameraDevice(m_ToFCameraDevice);
    return m_ToFImageGrabber;
  }

  ToFImageGrabber::Pointer ToFImageGrabberCreator::GetMESASR4000ImageGrabber()
  {
    m_ToFCameraDevice = mitk::ToFCameraMESASR4000Device::New();
    m_ToFImageGrabber->SetCameraDevice(m_ToFCameraDevice);
    return m_ToFImageGrabber;
  }

  ToFImageGrabber::Pointer ToFImageGrabberCreator::GetKinectImageGrabber()
  {
    m_ToFCameraDevice = mitk::KinectDevice::New();
    m_ToFImageGrabber->SetCameraDevice(m_ToFCameraDevice);
    return m_ToFImageGrabber;
  }
}
