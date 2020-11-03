/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSDevicePersistence.h"

//Microservices
#include <usServiceReference.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

#include <iostream>

mitk::USDevicePersistence::USDevicePersistence() : m_devices("MITK US", "Device Settings")
{
}

void mitk::USDevicePersistence::StoreCurrentDevices()
{
  us::ModuleContext* thisContext = us::GetModuleContext();

  std::vector<us::ServiceReference<USDevice> > services = thisContext->GetServiceReferences<USDevice>();
  MITK_INFO << "Trying to save " << services.size() << " US devices.";
  int numberOfSavedDevices = 0;
  for (std::vector<us::ServiceReference<USDevice> >::iterator it = services.begin(); it != services.end(); ++it)
  {
    mitk::USDevice::Pointer currentDevice = thisContext->GetService(*it);
    //check if it is a USVideoDevice
    if (currentDevice->GetDeviceClass() == "org.mitk.modules.us.USVideoDevice")
    {
      mitk::USVideoDevice::Pointer currentVideoDevice = dynamic_cast<mitk::USVideoDevice*>(currentDevice.GetPointer());
      QString identifier = "device" + QString::number(numberOfSavedDevices);
      m_devices.setValue(identifier, USVideoDeviceToString(currentVideoDevice));
      numberOfSavedDevices++;
    }

    else
    {
      MITK_WARN << "Saving of US devices of the type " << currentDevice->GetDeviceClass() << " is not supported at the moment. Skipping device.";
    }
  }
  m_devices.setValue("numberOfSavedDevices", numberOfSavedDevices);
  MITK_INFO << "Successfully saved " << numberOfSavedDevices << " US devices.";
}

std::vector<mitk::USDevice::Pointer> mitk::USDevicePersistence::RestoreLastDevices()
{
  std::vector<mitk::USDevice::Pointer> devices;

  int numberOfSavedDevices = m_devices.value("numberOfSavedDevices").toInt();

  for (int i = 0; i < numberOfSavedDevices; i++)
  {
    // Try each device. If an exception occurs: Ignore device and notify user
    try
    {
      QString currentString = m_devices.value("device" + QString::number(i)).toString();
      mitk::USDevice::Pointer currentDevice = dynamic_cast<mitk::USDevice*>(StringToUSVideoDevice(currentString).GetPointer());
      //currentDevice->Initialize();
      devices.push_back(currentDevice.GetPointer());
    }
    catch (...)
    {
      MITK_ERROR << "Error occured while loading a USVideoDevice from persistence. Device assumed corrupt, will be deleted.";
      //QMessageBox::warning(nullptr, "Could not load device" ,"A stored ultrasound device is corrupted and could not be loaded. The device will be deleted.");
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
  if (!d->GetIsSourceFile()) file = "none"; //if GetIsSourceFile is true, the device plays back a file

  mitk::USImageVideoSource::Pointer imageSource = dynamic_cast<mitk::USImageVideoSource*>(d->GetUSImageSource().GetPointer());
  if (!imageSource)
  {
    MITK_ERROR << "There is no USImageVideoSource at the current device.";
    mitkThrow() << "There is no USImageVideoSource at the current device.";
  }

  int greyscale = imageSource->GetIsGreyscale();
  int resOverride = imageSource->GetResolutionOverride();
  int resWidth = imageSource->GetResolutionOverrideWidth();
  int resHight = imageSource->GetResolutionOverrideHeight();

  QString probes = ""; //ACV$100%1%1%0$120%2%2%0$140%2%2%5!BDW$90%1%1%2$100%1%1%8!CSV$50%1%2%3$60%2%2%5

  char probesSeperator = '!';
  std::vector<mitk::USProbe::Pointer> allProbesOfDevice = d->GetAllProbes();
  if (allProbesOfDevice.size() > 0)
  {
    for (std::vector<mitk::USProbe::Pointer>::iterator it = allProbesOfDevice.begin(); it != allProbesOfDevice.end(); it++)
    {
      if (it == allProbesOfDevice.begin())
      { // if it is the first element there is no need for the probes seperator
        probes = probes + USProbeToString(*it);
      }
      else
      {
        probes = probes + probesSeperator + USProbeToString(*it);
      }
    }
  }

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
    + probes
    ;

  MITK_INFO << "Output String: " << returnValue.toStdString();
  return returnValue;
}

QString mitk::USDevicePersistence::USProbeToString(mitk::USProbe::Pointer p)
{
  QString probe = QString::fromStdString(p->GetName());
  QString croppingSeparator = QString(",");
  probe = probe + croppingSeparator + QString::number(p->GetProbeCropping().top)
                + croppingSeparator + QString::number(p->GetProbeCropping().right)
                + croppingSeparator + QString::number(p->GetProbeCropping().bottom)
                + croppingSeparator + QString::number(p->GetProbeCropping().left)
                + croppingSeparator;

  char depthSeperator = '$';
  char spacingSeperator = '%';
  std::map<int, mitk::Vector3D> depthsAndSpacing = p->GetDepthsAndSpacing();
  if (depthsAndSpacing.size() > 0)
  {
    for (std::map<int, mitk::Vector3D>::iterator it = depthsAndSpacing.begin(); it != depthsAndSpacing.end(); it++){
      probe = probe + depthSeperator + QString::number(it->first) + spacingSeperator + QString::number(it->second[0])
        + spacingSeperator + QString::number(it->second[1]) + spacingSeperator + QString::number(it->second[2]);
    }
  }
  return probe;
}

mitk::USVideoDevice::Pointer mitk::USDevicePersistence::StringToUSVideoDevice(QString s)
{
  MITK_INFO << "Input String: " << s.toStdString();
  std::vector<std::string> data;
  std::string seperators = "|";
  std::string text = s.toStdString();
  split(text, seperators, data);
  if (data.size() != 10)
  {
    MITK_ERROR << "Cannot parse US device! (Size: " << data.size() << ")";
    return mitk::USVideoDevice::New("INVALID", "INVALID", "INVALID");
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
  if (!imageSource)
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

  std::string probes = data.at(9);
  std::string probesSeperator = "!";
  std::vector<std::string> probesVector;
  split(probes, probesSeperator, probesVector);
  for (std::vector<std::string>::iterator it = probesVector.begin(); it != probesVector.end(); it++)
  {
    mitk::USProbe::Pointer probe = StringToUSProbe(*it);
    returnValue->AddNewProbe(probe);
  }

  return returnValue;
}

mitk::USProbe::Pointer mitk::USDevicePersistence::StringToUSProbe(std::string s)
{
  mitk::USProbe::Pointer probe = mitk::USProbe::New();
  std::string croppingSeparator = ",";
  std::string spacingSeperator = "%";
  std::string depthSeperator = "$";
  std::vector<std::string> probeCropping;
  split(s, croppingSeparator, probeCropping);

  std::vector<std::string> depthsWithSpacings;
  split(s, depthSeperator, depthsWithSpacings);

  //The first entry of the probeCropping vector is the name of the ultrasound probe:
  std::string probeName = probeCropping.at(0);
  probe->SetName(probeName);
  //The entries 1, 2, 3 and 4 of the probeCropping vector are the cropping top,
  // right, bottom and left:
  if( probeCropping.size() >= 6 )
  {
    QString top = QString::fromStdString(probeCropping.at(1));
    QString right = QString::fromStdString(probeCropping.at(2));
    QString bottom = QString::fromStdString(probeCropping.at(3));
    QString left = QString::fromStdString(probeCropping.at(4));
    probe->SetProbeCropping(top.toUInt(), bottom.toUInt(), left.toUInt(), right.toUInt());
  }

  for (std::vector<std::string>::iterator it = depthsWithSpacings.begin(); it != depthsWithSpacings.end(); it++)
  {
    //The first element is the name of the probe and the cropping entries.
    //other elements are the scanning depths of the probe and the spacing
    if (it != depthsWithSpacings.begin())
    {
      std::vector<std::string> spacings;
      split(*it, spacingSeperator, spacings);
      mitk::Vector3D spacing;
      double x;
      double y;
      double z;
      int depth;
      try
      {
        x = spacingToDouble(spacings.at(1));
        y = spacingToDouble(spacings.at(2));
        z = spacingToDouble(spacings.at(3));
      }
      catch (const mitk::Exception& e)
      {
        MITK_ERROR << e.GetDescription() << "Spacing of " << probe->GetName() << " at depth " << spacings.at(0) << " will be set to default value 1,1,0.";
        x = 1;
        y = 1;
        z = 1;
      }
      spacing[0] = x;
      spacing[1] = y;
      spacing[2] = z;

      try
      {
        depth = depthToInt(spacings.at(0));
      }
      catch (const mitk::Exception& e)
      {
        MITK_ERROR << probe->GetName() << ": " << e.GetDescription();
        continue;
      }
      probe->SetDepthAndSpacing(depth, spacing);
    }
  }
  return probe;
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

double mitk::USDevicePersistence::spacingToDouble(std::string s)
{
  std::istringstream i(s);
  double x;
  if (!(i >> x))
  {
    //something went wrong because the string contains characters which can not be convertet into double
    mitkThrow() << "An error occured while trying to recover the spacing.";
  }
  return x;
}

int mitk::USDevicePersistence::depthToInt(std::string s)
{
  std::istringstream i(s);
  int x;
  if (!(i >> x))
  {
    //something went wrong because the string contains characters which can not be convertet into int
    mitkThrow() << "An error occured while trying to recover the scanning depth. " << s << " is not a valid scanning depth. ";
  }
  return x;
}
