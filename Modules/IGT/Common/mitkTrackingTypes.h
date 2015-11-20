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

#include <itkPoint.h>
#include <mitkColorProperty.h>
#include <vector>

namespace mitk
{
  typedef std::string TrackingDeviceType;

  static const std::string TRACKING_DEVICE_IDENTIFIER_AURORA = "NDI Aurora"; ///< Aurora: electromagnetic Tracker from NDI
  static const std::string TRACKING_DEVICE_IDENTIFIER_POLARIS ="NDI Polaris"; ///< Polaris: optical Tracker from NDI
  static const std::string TRACKING_DEVICE_IDENTIFIER_MICRON = "Claron Micron"; ///< Micron Tracker: optical Tracker from Claron
  static const std::string TRACKING_DEVICE_IDENTIFIER_MICROBIRD = "Ascension MicroBird"; ///< Ascension microBird / PCIBird family
  static const std::string TRACKING_DEVICE_IDENTIFIER_VIRTUAL = "Virtual Tracker"; ///< Virtual Tracking device class that produces random tracking coordinates
  static const std::string TRACKING_DEVICE_IDENTIFIER_OPTITRACK = "NP Optitrack"; ///< NaturalPoint: Optitrack optical Tracking System
  static const std::string TRACKING_DEVICE_IDENTIFIER_OPENIGTLINK = "Open IGT Link Tracking Device Connection"; ///< Device which is connected via open igt link
  static const std::string TRACKING_DEVICE_IDENTIFIER_UNSPECIFIED = "Tracking System Not Specified"; ///< entry for not specified or initialized tracking system
  static const std::string TRACKING_DEVICE_IDENTIFIER_INVALID = "Tracking System Invalid"; ///< entry for invalid state (mainly for testing)

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

  /** definition of colors for IGT */
  static mitk::Color IGTColor_WARNING = mitk::ColorProperty::New(1.0f, 0.0f, 0.0f)->GetColor();
  static mitk::Color IGTColor_VALID = mitk::ColorProperty::New(0.0f, 1.0f, 0.0f)->GetColor();
} // namespace mitk
#endif /* MITKTRACKINGTYPES_H_HEADER_INCLUDED_ */
