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

#include "mitkUSDevicePersistence.h"

//Microservices
#include <usServiceReference.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

mitk::USDevicePersistence::USDevicePersistence() : m_devices("MITK US","Device Settings")
{
}

void mitk::USDevicePersistence::StoreCurrentDevices()
{
  us::ModuleContext* thisContext = us::GetModuleContext();

  std::vector<us::ServiceReference<USDevice> > services = thisContext->GetServiceReferences<USDevice>();
  MITK_INFO << "Trying to save " << services.size() << " US devices.";
  int numberOfSavedDevices = 0;
  for(std::vector<us::ServiceReference<USDevice> >::iterator it = services.begin(); it != services.end(); ++it)
  {
    mitk::USDevice::Pointer currentDevice = thisContext->GetService(*it);
    //check if it is a USVideoDevice
    if (currentDevice->GetDeviceClass() == "org.mitk.modules.us.USVideoDevice")
    {
      mitk::USVideoDevice::Pointer currentVideoDevice = dynamic_cast<mitk::USVideoDevice*>(currentDevice.GetPointer());
      QString identifier = "device" + QString::number(numberOfSavedDevices);
      m_devices.setValue(identifier,USVideoDeviceToString(currentVideoDevice));
      numberOfSavedDevices++;
    }

    else
    {
      MITK_WARN << "Saving of US devices of the type " << currentDevice->GetDeviceClass() << " is not supported at the moment. Skipping device.";
    }
  }
  m_devices.setValue("numberOfSavedDevices",numberOfSavedDevices);
  MITK_INFO << "Successfully saved " << numberOfSavedDevices << " US devices.";
}

std::vector<mitk::USDevice::Pointer> mitk::USDevicePersistence::RestoreLastDevices()
{
  std::vector<mitk::USDevice::Pointer> devices;

  int numberOfSavedDevices = m_devices.value("numberOfSavedDevices").toInt();

  for(int i=0; i<numberOfSavedDevices; i++)
  {
    // Try each device. If an exception occurs: Ignore device and notify user
    try
    {
      QString currentString = m_devices.value("device"+QString::number(i)).toString();
      mitk::USDevice::Pointer currentDevice = dynamic_cast<mitk::USDevice*>(StringToUSVideoDevice(currentString).GetPointer());
      //currentDevice->Initialize();
      devices.push_back(currentDevice.GetPointer());
    }
    catch (...)
    {
      MITK_ERROR << "Error occured while loading a USVideoDevice from persistence. Device assumed corrupt, will be deleted.";
      //QMessageBox::warning(NULL, "Could not load device" ,"A stored ultrasound device is corrupted and could not be loaded. The device will be deleted.");
    }
  }

  MITK_INFO << "Restoring " << numberOfSavedDevices << " US devices.";

  return devices;
}

QString mitk::USDevicePersistence::USVideoDeviceToString(mitk::USVideoDevice::Pointer d)
{
  QString manufacturer = d->GetManufacturer().c_str();
  QString model = d->GetName().c_str();
  QString comment = d->GetComment().c_str();
  int source = d->GetDeviceID();
  std::string file = d->GetFilePath();
  if (file == "") file = "none";

  mitk::USImageVideoSource::Pointer imageSource = dynamic_cast<mitk::USImageVideoSource*>(d->GetUSImageSource().GetPointer());
  if ( ! imageSource )
  {
    MITK_ERROR << "There is no USImageVideoSource at the current device.";
    mitkThrow() << "There is no USImageVideoSource at the current device.";
  }

  int greyscale = imageSource->GetIsGreyscale();
  int resOverride = imageSource->GetResolutionOverride();
  int resWidth = imageSource->GetResolutionOverrideWidth();
  int resHight = imageSource->GetResolutionOverrideHeight();

  mitk::USImageVideoSource::USImageRoi roi = imageSource->GetRegionOfInterest();

  char seperator = '|';

  QString returnValue = manufacturer + seperator
                       + model + seperator
                       + comment + seperator
                       + QString::number(source) + seperator
                       + file.c_str() + seperator
                       + QString::number(greyscale) + seperator
                       + QString::number(resOverride) + seperator
                       + QString::number(resWidth) + seperator
                       + QString::number(resHight) + seperator
                       + QString::number(roi.topLeftX) + seperator
                       + QString::number(roi.topLeftY) + seperator
                       + QString::number(roi.bottomRightX) + seperator
                       + QString::number(roi.bottomRightY)
                       ;

  MITK_INFO << "Output String: " << returnValue.toStdString();
  return returnValue;
}

mitk::USVideoDevice::Pointer mitk::USDevicePersistence::StringToUSVideoDevice(QString s)
{
  MITK_INFO << "Input String: " << s.toStdString();
  std::vector<std::string> data;
  std::string seperators = "|";
  std::string text = s.toStdString();
  split(text,seperators,data);
  if(data.size() != 13)
  {
    MITK_ERROR << "Cannot parse US device! (Size: " << data.size() << ")";
    return mitk::USVideoDevice::New("INVALID","INVALID","INVALID");
  }

  std::string manufacturer = data.at(0);
  std::string model = data.at(1);
  std::string comment = data.at(2);
  int source = (QString(data.at(3).c_str())).toInt();
  std::string file = data.at(4);
  bool greyscale = (QString(data.at(5).c_str())).toInt();
  bool resOverride = (QString(data.at(6).c_str())).toInt();
  int resWidth = (QString(data.at(7).c_str())).toInt();
  int resHight = (QString(data.at(8).c_str())).toInt();
  mitk::USImageVideoSource::USImageRoi cropArea;
  cropArea.topLeftX = (QString(data.at(9).c_str())).toInt();
  cropArea.topLeftY = (QString(data.at(10).c_str())).toInt();
  cropArea.bottomRightX = (QString(data.at(11).c_str())).toInt();
  cropArea.bottomRightY = (QString(data.at(12).c_str())).toInt();

  // Create Device
  mitk::USVideoDevice::Pointer returnValue;
  if (file == "none")
  {
    returnValue = mitk::USVideoDevice::New(source, manufacturer, model);
    returnValue->SetComment(comment);
  }
  else
  {
    returnValue = mitk::USVideoDevice::New(file, manufacturer, model);
    returnValue->SetComment(comment);
  }

  mitk::USImageVideoSource::Pointer imageSource =
    dynamic_cast<mitk::USImageVideoSource*>(returnValue->GetUSImageSource().GetPointer());
  if ( ! imageSource )
  {
    MITK_ERROR << "There is no USImageVideoSource at the current device.";
    mitkThrow() << "There is no USImageVideoSource at the current device.";
  }

  // Set Video Options
  imageSource->SetColorOutput(!greyscale);

  // If Resolution override is activated, apply it
  if (resOverride)
    {
    imageSource->OverrideResolution(resWidth, resHight);
    imageSource->SetResolutionOverride(true);
    }

  // Set Crop Area
  imageSource->SetRegionOfInterest(cropArea);

  return returnValue;
}

void mitk::USDevicePersistence::split(std::string& text, std::string& separators, std::vector<std::string>& words)
{
  int n = text.length();
  int start, stop;

  start = text.find_first_not_of(separators);
  while ((start >= 0) && (start < n))
  {
    stop = text.find_first_of(separators, start);
    if ((stop < 0) || (stop > n)) stop = n;
    words.push_back(text.substr(start, stop - start));
    start = text.find_first_not_of(separators, stop + 1);
  }
}