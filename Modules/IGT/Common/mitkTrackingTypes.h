/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTrackingTypes_h
#define mitkTrackingTypes_h

#include <itkPoint.h>
#include <mitkColorProperty.h>
#include <vector>

namespace mitk
{
  typedef std::string TrackingDeviceType;

  /**
   * /brief This structure defines key variables of a device model and type.
   * Line is (usually) identical with the TrackingDeviceName and can be used to compare TrackingDevices (string).
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
  static mitk::Color IGTColor_INVALID = mitk::ColorProperty::New(1.0f, 1.0f, 1.0f)->GetColor();
} // namespace mitk
#endif
