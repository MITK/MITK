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


#ifndef MITKTRACKINGTYPES_H_HEADER_INCLUDED_
#define MITKTRACKINGTYPES_H_HEADER_INCLUDED_

#include <itkPoint.h>
//#include <mitkVector.h>

namespace mitk 
{

  /**Documentation
   * \brief Error codes of NDI tracking devices
   */
    enum NDIErrorCode
    {
      NDIOKAY = 0,
      NDIERROR = 1,
      SERIALINTERFACENOTSET,
      SERIALSENDERROR,
      SERIALRECEIVEERROR,
      SROMFILETOOLARGE,
      SROMFILETOOSMALL,
      NDICRCERROR,                          // reply has crc error, local computer detected the error
      NDIINVALIDCOMMAND,
      NDICOMMANDTOOLONG,
      NDICOMMANDTOOSHORT,
      NDICRCDOESNOTMATCH,                   // command had crc error, tracking device detected the error
      NDITIMEOUT,
      NDIUNABLETOSETNEWCOMMPARAMETERS,
      NDIINCORRECTNUMBEROFPARAMETERS,
      NDIINVALIDPORTHANDLE,
      NDIINVALIDTRACKINGPRIORITY,
      NDIINVALIDLED,
      NDIINVALIDLEDSTATE,
      NDICOMMANDINVALIDINCURRENTMODE,
      NDINOTOOLFORPORT,
      NDIPORTNOTINITIALIZED,
      NDISYSTEMNOTINITIALIZED,
      NDIUNABLETOSTOPTRACKING,
      NDIUNABLETOSTARTTRACKING,
      NDIINITIALIZATIONFAILED,
      NDIINVALIDVOLUMEPARAMETERS,
      NDICANTSTARTDIAGNOSTICMODE,
      NDICANTINITIRDIAGNOSTICS,
      NDIFAILURETOWRITESROM,
      NDIENABLEDTOOLSNOTSUPPORTED,
      NDICOMMANDPARAMETEROUTOFRANGE,
      NDINOMEMORYAVAILABLE,
      NDIPORTHANDLENOTALLOCATED,
      NDIPORTHASBECOMEUNOCCUPIED,
      NDIOUTOFHANDLES,
      NDIINCOMPATIBLEFIRMWAREVERSIONS,
      NDIINVALIDPORTDESCRIPTION,
      NDIINVALIDOPERATIONFORDEVICE,
      NDIWARNING,
      NDIUNKNOWNERROR,
      NDIUNEXPECTEDREPLY,
      UNKNOWNHANDLERETURNED,
      TRACKINGDEVICERESET,
      TRACKINGDEVICENOTSET
    };

    /**Documentation
    * \brief identifier for tracking device
    */
    enum TrackingDeviceType
    {
      NDIPolaris,                ///< Polaris: optical Tracker from NDI
      NDIAurora,                 ///< Aurora: electromagnetic Tracker from NDI
      ClaronMicron,              ///< Micron Tracker: optical Tracker from Claron
      IntuitiveDaVinci,          ///< Intuitive Surgical: DaVinci Telemanipulator API Interface
      AscensionMicroBird,        ///< Ascension microBird / PCIBird family
      VirtualTrackingDevice,     ///< Virtual Tracking device class that produces random tracking coordinates
      TrackingSystemNotSpecified ///< entry for not specified or initialized tracking system
    };

    /**Documentation
    * \brief Error codes of NDI tracking devices
    */
    enum OperationMode
    {
      ToolTracking6D,
      ToolTracking5D,
      MarkerTracking3D,
      HybridTracking
    };

 
    /**
    * \brief Represents the setting of the tracking volume of a NDI tracking device. The tracking volume of
    * a tracking device itself (as 3d-Object) is represented by an instance of the class mitk::TrackingVolume
    */
    enum NDITrackingVolume
    {
      Standard,
      Pyramid
    };
    /**Documentation
    * \brief activation rate of IR illuminator for NDI Polaris tracking device
    */
    enum IlluminationActivationRate
    {
      Hz20 = 20,
      Hz30 = 30,
      Hz60 = 60
    };
    /**Documentation
    * \brief Data transfer mode for NDI tracking devices
    */
    enum DataTransferMode
    {
      TX = 0,
      BX = 1
    };
    /**Documentation
    * \brief Query mode for NDI tracking devices
    */
    enum PHSRQueryType
    {
      ALL         = 0x00,
      FREED       = 0x01,
      OCCUPIED    = 0x02,
      INITIALIZED = 0x03,
      ENABLED     = 0x04
    };


    typedef itk::Point<double> MarkerPointType;
    typedef std::vector<MarkerPointType> MarkerPointContainerType;

    /**
    * \brief Defines the tools (arms) of the daVinci system:
    * PSM1 - Patient side manipulator 1
    * PSM2 - Patient side manipulator 2
    * ECM - Endoscopic camera manipulator
    * MTML - Left master target manipulator
    * MTMR - Right master target manipulator
    * PSM  - Patient side manipulator 3 (if not existent, its data will always be zero)
    **/
    enum DaVinciToolType
    {
      PSM1 = 0,
      PSM2 = 1,
      ECM  = 2,
      MTML = 3,
      MTMR = 4,
      PSM  = 5,
      //UIEvents = 6,
    };
} // namespace mitk
#endif /* MITKTRACKINGTYPES_H_HEADER_INCLUDED_ */
