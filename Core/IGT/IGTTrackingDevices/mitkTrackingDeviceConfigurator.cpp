/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTrackingDeviceConfigurator.h"

mitk::TrackingDeviceConfigurator::TrackingDeviceConfigurator():itk::Object()
{
}

mitk::TrackingDeviceConfigurator::~TrackingDeviceConfigurator()
{
}


mitk::TrackingDevice* mitk::TrackingDeviceConfigurator::CreateTrackingDevice()
{
  return NULL;
}

void mitk::TrackingDeviceConfigurator::SetDeviceType(mitk::TrackingDeviceType dev)
{

}

void mitk::TrackingDeviceConfigurator::SetPortNumber(mitk::PortNumber port)
{

}

void mitk::TrackingDeviceConfigurator::SetBaudRate(mitk::BaudRate baud)
{

}

void mitk::TrackingDeviceConfigurator::SetDataBits(mitk::DataBits bits)
{

}

void mitk::TrackingDeviceConfigurator::SetParity(mitk::Parity p)
{

}

void mitk::TrackingDeviceConfigurator::SetStopBits(mitk::StopBits bits)
{

}

void mitk::TrackingDeviceConfigurator::SetHardwareHandshake(mitk::HardwareHandshake shake)
{

}

void mitk::TrackingDeviceConfigurator::SetIlluminationActivationRate(mitk::IlluminationActivationRate hz)
{

}

void mitk::TrackingDeviceConfigurator::SetDataTransferMode(mitk::DataTransferMode mode)
{

}

void mitk::TrackingDeviceConfigurator::SetOperationMode(mitk::OperationMode mode)
{

}

void mitk::TrackingDeviceConfigurator::Add6DTool(std::string* toolName, std::string* romFile)
{

}

void mitk::TrackingDeviceConfigurator::Add5DTool(std::string* toolName, std::string* romFile)
{

}
