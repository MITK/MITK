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
#include <mitkModuleContext.h>

//QT
#include <QMessageBox>


mitk::USDevicePersistence::USDevicePersistence() : m_devices("MITK US","Device Settings")
{
}

void mitk::USDevicePersistence::StoreCurrentDevices()
{
  mitk::ModuleContext* thisContext = mitk::GetModuleContext();

  std::list<mitk::ServiceReference> services = thisContext->GetServiceReferences<mitk::USDevice>();
  MITK_INFO << "Trying to save " << services.size() << " US devices.";
  int numberOfSavedDevices = 0;
  for(std::list<mitk::ServiceReference>::iterator it = services.begin(); it != services.end(); ++it)
  {
    mitk::USDevice::Pointer currentDevice = thisContext->GetService<mitk::USDevice>(*it);
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

void mitk::USDevicePersistence::RestoreLastDevices()
{
  int numberOfSavedDevices = m_devices.value("numberOfSavedDevices").toInt();

  for(int i=0; i<numberOfSavedDevices; i++)
  {
    // Try each device. If an exception occurs: Ignore device and notify user
    try
    {
      QString currentString = m_devices.value("device"+QString::number(i)).toString();
      mitk::USVideoDevice::Pointer currentDevice = StringToUSVideoDevice(currentString);
      currentDevice->Connect();
    }
    catch (...)
    {
      MITK_ERROR << "Error occured while loading a USVideoDevice from persistence. Device assumed corrupt, will be deleted.";
      QMessageBox::warning(NULL, "Could not load device" ,"A stored ultrasound device is corrupted and could not be loaded. The device will be deleted.");
    }
  }

  MITK_INFO << "Restoring " << numberOfSavedDevices << " US devices.";
}

QString mitk::USDevicePersistence::USVideoDeviceToString(mitk::USVideoDevice::Pointer d)
{

  QString manufacturer = d->GetDeviceManufacturer().c_str();
  QString model = d->GetDeviceModel().c_str();
  QString comment = d->GetDeviceComment().c_str();
  int source = d->GetDeviceID();
  std::string file = d->GetFilePath();
  if (file == "") file = "none";
  int greyscale = d->GetSource()->GetIsGreyscale();
  int resOverride = d->GetSource()->GetResolutionOverride();
  int resWidth = d->GetSource()->GetResolutionOverrideWidth();
  int resHight = d->GetSource()->GetResolutionOverrideHeight();
  int cropRight = d->GetCropArea().cropRight;
  int cropLeft = d->GetCropArea().cropLeft;
  int cropBottom = d->GetCropArea().cropBottom;
  int cropTop = d->GetCropArea().cropTop;
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
                       + QString::number(cropRight) + seperator
                       + QString::number(cropLeft) + seperator
                       + QString::number(cropBottom) + seperator
                       + QString::number(cropTop)
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
  mitk::USDevice::USImageCropArea cropArea;
  cropArea.cropRight = (QString(data.at(9).c_str())).toInt();
  cropArea.cropLeft = (QString(data.at(10).c_str())).toInt();
  cropArea.cropBottom = (QString(data.at(11).c_str())).toInt();
  cropArea.cropTop = (QString(data.at(12).c_str())).toInt();

  // Assemble Metadata
  mitk::USImageMetadata::Pointer metadata = mitk::USImageMetadata::New();
  metadata->SetDeviceManufacturer(manufacturer);
  metadata->SetDeviceComment(comment);
  metadata->SetDeviceModel(model);
  metadata->SetProbeName("");
  metadata->SetZoom("");

  // Create Device
  mitk::USVideoDevice::Pointer returnValue;
  if (file == "none")
  {
    returnValue = mitk::USVideoDevice::New(source, metadata);
  }
  else
  {
    returnValue = mitk::USVideoDevice::New(file, metadata);
  }

  // Set Video Options
  returnValue->GetSource()->SetColorOutput(!greyscale);

  // If Resolution override is activated, apply it
  if (resOverride)
    {
    returnValue->GetSource()->OverrideResolution(resWidth, resHight);
    returnValue->GetSource()->SetResolutionOverride(true);
    }

  // Set Crop Area
  returnValue->SetCropArea(cropArea);

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
