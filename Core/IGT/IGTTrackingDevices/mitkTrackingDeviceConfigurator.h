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

#ifndef MITKTRACKINGDEVICECONFIGURATOR_H_HEADER_INCLUDED_C1C2FCD2
#define MITKTRACKINGDEVICECONFIGURATOR_H_HEADER_INCLUDED_C1C2FCD2

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>

#include "mitkTrackingTypes.h"
//ToDoMBISB #include "mitkNDITrackingDevice.h"

namespace mitk {
    class TrackingDevice;

    //##Documentation
    //## \brief This class constructs a working tracking component. 
    //##
    //## To use the tracking component, always create a TrackingDeviceConfigurator
    //## object and use it to setup the tracking component.
    //## When you are ready, use the CreateTrackingDevice() method retrieve a 
    //## pointer to a ready to use tracking component.
    //## The retrieved pointer must be stored in a mitk::TrackingDevice::Pointer
    //##
    //## @ingroup Tracking
    class TrackingDeviceConfigurator : public itk::Object
    {
    public:
      
      mitkClassMacro(TrackingDeviceConfigurator, itk::Object);
      itkNewMacro(Self);

      virtual void SetDeviceType(const TrackingDeviceType dev);
      virtual void SetPortNumber(const PortNumber port);
      virtual void SetBaudRate(const BaudRate baud);
      virtual void SetDataBits(const DataBits bits);
      virtual void SetParity(const Parity p);
      virtual void SetStopBits(const StopBits bits);
      virtual void SetHardwareHandshake(const HardwareHandshake shake);
      virtual void SetIlluminationActivationRate(const IlluminationActivationRate hz);
      virtual void SetDataTransferMode(const DataTransferMode mode);
      virtual void SetOperationMode(const OperationMode mode);
      virtual void Add6DTool(std::string* toolName, std::string* romFile);
      virtual void Add5DTool(std::string* toolName, std::string* romFile);
      
      virtual TrackingDevice* CreateTrackingDevice();

    protected:
      TrackingDeviceConfigurator();
      virtual ~TrackingDeviceConfigurator();

      //ToDoMBISB NDITrackingDevice::Pointer m_TrackingDevice;
    };
} // namespace mitk

#endif /* MITKTRACKINGDEVICECONFIGURATOR_H_HEADER_INCLUDED_C1C2FCD2 */
