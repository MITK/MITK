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


#ifndef MITKTRACKINGDEVICE_H_HEADER_INCLUDED_C1C2FCD2
#define MITKTRACKINGDEVICE_H_HEADER_INCLUDED_C1C2FCD2


#include "itkObject.h"
#include "mitkCommon.h"
#include "mitkTrackingTypes.h"
#include "itkFastMutexLock.h"
#include "mitkTrackingVolume.h"


namespace mitk {
    class TrackingTool; // interface for a tool that can be tracked by the TrackingDevice

    //##Documentation
    //## \brief Interface for all Tracking Devices
    //##
    //## Defines the methods that are common for all tracking devices.
    //## A User should never directly instantiate one of its subclasses.
    //## Instead, he should always instantiate an apropriate Configurator 
    //## object and get a reference of the type TrackingDevice from it.
    //## 
    //## @ingroup Tracking
    class TrackingDevice : public itk::Object
    {
    public:
      mitkClassMacro(TrackingDevice, itk::Object);

      enum TrackingDeviceMode {Setup, Ready, Tracking};   // Type for state variable. The trackingdevice is always in one of these states

      virtual bool OpenConnection() = 0;
      virtual bool CloseConnection() = 0;
      virtual bool StartTracking() = 0;
      virtual bool StopTracking();
      virtual TrackingTool* GetTool(unsigned int toolNumber) = 0;
      virtual unsigned int GetToolCount() const = 0;
      itkGetStringMacro(ErrorMessage);
      TrackingDeviceMode GetMode() const;
      
      TrackingVolume::Pointer GetTrackingVolume();

      itkGetConstMacro(Type,TrackingDeviceType);
      itkSetMacro(Type,TrackingDeviceType);

    protected:

      itkSetStringMacro(ErrorMessage);
      itkSetMacro(Mode,TrackingDeviceMode);


      TrackingDevice();
      virtual ~TrackingDevice();

      TrackingDeviceType m_Type;
      TrackingDeviceMode m_Mode;
      bool m_StopTracking;
      mitk::TrackingVolume::Pointer m_TrackingVolume;
      itk::FastMutexLock::Pointer m_StopTrackingMutex;
      itk::FastMutexLock::Pointer m_TrackingFinishedMutex;
      itk::FastMutexLock::Pointer m_ModeMutex;
      std::string m_ErrorMessage;
    };
} // namespace mitk

#endif /* MITKTRACKINGDEVICE_H_HEADER_INCLUDED_C1C2FCD2 */
