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

#include "mitkTrackingTypes.h"

namespace mitk
{

  std::vector<TrackingDeviceData> GetDeviceDataForLine(TrackingDeviceType Type)
  {
    std::vector<TrackingDeviceData> Result;
    int size = (sizeof (TrackingDeviceList) / sizeof*(TrackingDeviceList));
    for(int i=0; i < size; i++)
    {
      if(TrackingDeviceList[i].Line == Type ) Result.push_back(TrackingDeviceList[i]);
    }
    return Result;
  }

  TrackingDeviceData GetFirstCompatibleDeviceDataForLine(TrackingDeviceType Type)
  {
    return GetDeviceDataForLine(Type).front();
  }

  TrackingDeviceData GetDeviceDataByName(const std::string& modelName)
  {
    int size = (sizeof (TrackingDeviceList) / sizeof*(TrackingDeviceList));
    for(int i=0; i < size; i++)
    {
      if(TrackingDeviceList[i].Model == modelName) return TrackingDeviceList[i];
    }
    return DeviceDataInvalid;
  }

} // namespace mitk
