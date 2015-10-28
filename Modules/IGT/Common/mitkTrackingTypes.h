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


#ifndef MITKTRACKINGTYPES_H_HEADER_INCLUDED_
#define MITKTRACKINGTYPES_H_HEADER_INCLUDED_

#include <MitkIGTExports.h>

#include <itkPoint.h>
#include <mitkColorProperty.h>
#include <vector>

namespace mitk
{

    /**Documentation
    * \brief identifier for tracking device. The way it is currently used
    * represents a product line rather than an actal type. Refactoring is a future option.
    */
    enum TrackingDeviceType
    {
      NDIPolaris,                 ///< Polaris: optical Tracker from NDI
      NDIAurora,                  ///< Aurora: electromagnetic Tracker from NDI
      ClaronMicron,               ///< Micron Tracker: optical Tracker from Claron
      IntuitiveDaVinci,           ///< Intuitive Surgical: DaVinci Telemanipulator API Interface
      AscensionMicroBird,         ///< Ascension microBird / PCIBird family
      VirtualTracker,             ///< Virtual Tracking device class that produces random tracking coordinates
      TrackingSystemNotSpecified, ///< entry for not specified or initialized tracking system
      TrackingSystemInvalid,      ///< entry for invalid state (mainly for testing)
      NPOptitrack,                          ///< NaturalPoint: Optitrack optical Tracking System
      OpenIGTLinkTrackingDeviceConnection   ///< Device which is connected via open igt link
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
  * /brief This structure defines key variables of a device model and type.
  * It is specifically used to find out which models belong to which vendor, and what volume
  * to use for a specific Model. Leaving VolumeModelLocation set to null will instruct the Generator
  * to generate a field to the best of his ability. HardwareCode stands for a hexadecimal string,
  * that represents the tracking volume. "X" stands for "hardwarecode is not known" or "tracking device has
  * no hardware code". For NDI devices it is used in the SetVolume() Method in  mitkNDITrackingDevice.cpp.
  * The Pyramid Volume has the hardwarecode "4", but it is not supported yet.
  */
  struct TrackingDeviceData {
    TrackingDeviceType Line;
    std::string Model;
    std::string VolumeModelLocation;
    std::string HardwareCode;
    };


  /**
  * Here all supported devices are defined. Dont forget to introduce new Devices into the TrackingDeviceList Array at the bottom!
  * If a model does not have a corresponding tracking volume yet, pass an empty string to denote "No Model". pass "cube" to render
  * a default cube of 400x400 px. You can define additional magic strings in the TrackingVolumeGenerator.
  */

  /**
  * /brief Returns the device Data set matching the model name or the invalid device, if none was found
  */
  MITKIGT_EXPORT TrackingDeviceData GetDeviceDataByName(const std::string& modelName);

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

    /** definition of a colors for IGT */
    static mitk::Color IGTColor_WARNING = mitk::ColorProperty::New(1.0f, 0.0f, 0.0f)->GetColor();
    static mitk::Color IGTColor_VALID = mitk::ColorProperty::New(0.0f, 1.0f, 0.0f)->GetColor();
} // namespace mitk
#endif /* MITKTRACKINGTYPES_H_HEADER_INCLUDED_ */
