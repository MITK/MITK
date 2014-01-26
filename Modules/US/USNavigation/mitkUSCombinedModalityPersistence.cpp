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

#include "mitkUSCombinedModalityPersistence.h"
#include "mitkUSCombinedModality.h"

// Microservices
#include <usServiceReference.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

// Qt
#include <QSettings>
#include <QVector>
#include <QHash>
#include <QStringList>

//Q_DECLARE_METATYPE(QList<QStringList>)

mitk::USCombinedModalityPersistence::USCombinedModalityPersistence()
{
  us::ModuleContext* context = us::GetModuleContext();

  this->LoadStoredDevices();

  // to be notified about every register and unregister of an USDevice
  std::string filterExcludeCombinedModalities = "(&(" + us::ServiceConstants::OBJECTCLASS() + "="
      + "org.mitk.services.UltrasoundDevice)(!(" + mitk::USDevice::US_PROPKEY_CLASS + "="
      + mitk::USCombinedModality::DeviceClassIdentifier + ")))";
  context->AddServiceListener(this, &mitk::USCombinedModalityPersistence::OnServiceEvent,
    filterExcludeCombinedModalities);
  //context->AddServiceListener(this, &mitk::USCombinedModalityPersistence::OnServiceEvent,
  //  "(&(" + us::ServiceConstants::OBJECTCLASS() + "=" + us_service_interface_iid<mitk::USDevice>()
  //                            + ")(!(" + mitk::USDevice::US_PROPKEY_CLASS + "="
  //                            + mitk::USCombinedModality::DeviceClassIdentifier + ")))");

  // to be notified about every register and unregister of an NavigationDataSource
  context->AddServiceListener(this, &mitk::USCombinedModalityPersistence::OnServiceEvent,
    "(" + us::ServiceConstants::OBJECTCLASS() + "=" + us_service_interface_iid<mitk::NavigationDataSource>() + ")");
}

mitk::USCombinedModalityPersistence::~USCombinedModalityPersistence()
{
  this->StoreCurrentDevices();
}

void mitk::USCombinedModalityPersistence::OnServiceEvent(const us::ServiceEvent event)
{
  if ( event.GetType() == event.REGISTERED )
  {
    this->LoadStoredDevices();
  }
}

void mitk::USCombinedModalityPersistence::StoreCurrentDevices()
{
  QList<QVariant> devices;

  us::ModuleContext* context = us::GetModuleContext();

  std::string filterOnlyCombinedModalities = "(&(" + us::ServiceConstants::OBJECTCLASS() + "=" + "org.mitk.services.UltrasoundDevice)(" + mitk::USDevice::US_PROPKEY_CLASS + "=" + mitk::USCombinedModality::DeviceClassIdentifier + "))";
  std::vector<us::ServiceReference<USDevice> > services = context->GetServiceReferences<USDevice>(filterOnlyCombinedModalities);

  for ( std::vector<us::ServiceReference<USDevice> >::iterator it = services.begin();
        it != services.end(); ++it )
  {
    QStringList deviceStrings;
    mitk::USCombinedModality::Pointer currentDevice = dynamic_cast<mitk::USCombinedModality*>(context->GetService(*it));
    if ( currentDevice.IsNotNull() )
    {
      deviceStrings.push_back(QString::fromStdString(currentDevice->GetDeviceManufacturer()));
      deviceStrings.push_back(QString::fromStdString(currentDevice->GetDeviceModel()));

      mitk::NavigationDataSource::Pointer navigationDataSource = currentDevice->GetNavigationDataSource();
      if ( currentDevice.IsNull() ) { continue; }
      deviceStrings.push_back(QString::fromStdString(navigationDataSource->GetName()));

      mitk::USDevice::Pointer ultrasoundDevice = currentDevice->GetUltrasoundDevice();
      if ( ultrasoundDevice.IsNull() ) { continue; }
      deviceStrings.push_back(QString::fromStdString(ultrasoundDevice->GetDeviceManufacturer()));
      deviceStrings.push_back(QString::fromStdString(ultrasoundDevice->GetDeviceModel()));
      deviceStrings.push_back(QString::fromStdString(ultrasoundDevice->GetDeviceComment()));

      deviceStrings.push_back(QString::fromStdString(currentDevice->SerializeCalibration()));
    }

    devices.push_back(deviceStrings);
  }

  QSettings settings;
  settings.setValue("combined-modalities", devices);
}

void mitk::USCombinedModalityPersistence::LoadStoredDevices()
{
  QSettings settings;
  QList<QVariant> devices = settings.value("combined-modalities").value< QList<QVariant> >();

  for ( QList<QVariant>::iterator it = devices.begin();
        it != devices.end(); ++it )
  {
    QStringList stringList = it->toStringList();
    if (stringList.size() >= 7 && this->GetCombinedModality(stringList.at(0).toStdString(), stringList.at(1).toStdString()).IsNull())
    {
      mitk::NavigationDataSource::Pointer navigationDataSource = this->GetNavigationDataSource(stringList.at(2).toStdString());
      mitk::USDevice::Pointer usDevice = this->GetUSDevice(stringList.at(3).toStdString(), stringList.at(4).toStdString(), stringList.at(5).toStdString());

      if ( navigationDataSource.IsNotNull() && usDevice.IsNotNull() )
      {
        mitk::USCombinedModality::Pointer combinedModality = mitk::USCombinedModality::New(usDevice, navigationDataSource, stringList.at(0).toStdString(), stringList.at(1).toStdString());
        combinedModality->DeserializeCalibration(stringList.at(6).toStdString());
        combinedModality->Initialize();
        combinedModality->ConnectAsynchron();
      }
    }
  }
}

mitk::USCombinedModality::Pointer mitk::USCombinedModalityPersistence::GetCombinedModality(std::string manufacturer, std::string model)
{
  us::ModuleContext* context = us::GetModuleContext();

  std::string filterOnlyCombinedModalities = "(&(" + us::ServiceConstants::OBJECTCLASS() + "="
      + "org.mitk.services.UltrasoundDevice)(" + mitk::USDevice::US_PROPKEY_CLASS + "="
      + mitk::USCombinedModality::DeviceClassIdentifier + "))";
  std::vector<us::ServiceReference<USDevice> > services = context->GetServiceReferences<USDevice>(filterOnlyCombinedModalities);

  for ( std::vector<us::ServiceReference<USDevice> >::iterator it = services.begin();
        it != services.end(); ++it )
  {
    mitk::USCombinedModality::Pointer currentDevice = dynamic_cast<mitk::USCombinedModality*>(context->GetService(*it));
    if ( currentDevice.IsNotNull() )
    {
      if ( currentDevice->GetDeviceManufacturer() == manufacturer && currentDevice->GetDeviceModel() == model )
      {
        return currentDevice;
      }
    }
  }

  return 0;
}

mitk::USDevice::Pointer mitk::USCombinedModalityPersistence::GetUSDevice(std::string manufacturer, std::string model, std::string comment)
{
  us::ModuleContext* context = us::GetModuleContext();

  std::string filterExcludeCombinedModalities = "(&(" + us::ServiceConstants::OBJECTCLASS() + "="
      + "org.mitk.services.UltrasoundDevice)(!(" + mitk::USDevice::US_PROPKEY_CLASS + "="
      + mitk::USCombinedModality::DeviceClassIdentifier + ")))";
  std::vector<us::ServiceReference<USDevice> > services = context->GetServiceReferences<USDevice>(filterExcludeCombinedModalities);

  for ( std::vector<us::ServiceReference<USDevice> >::iterator it = services.begin();
        it != services.end(); ++it )
  {
    mitk::USDevice::Pointer currentDevice = dynamic_cast<mitk::USDevice*>(context->GetService(*it));
    if ( currentDevice.IsNotNull() )
    {
      if ( currentDevice->GetDeviceManufacturer() == manufacturer && currentDevice->GetDeviceModel() == model
           && currentDevice->GetDeviceComment() == comment )
      {
        return currentDevice;
      }
    }
  }

  return 0;
}

mitk::NavigationDataSource::Pointer mitk::USCombinedModalityPersistence::GetNavigationDataSource(std::string name)
{
  us::ModuleContext* context = us::GetModuleContext();

  std::vector<us::ServiceReference<mitk::NavigationDataSource> > services = context->GetServiceReferences<mitk::NavigationDataSource>();

  for ( std::vector<us::ServiceReference<mitk::NavigationDataSource> >::iterator it = services.begin();
        it != services.end(); ++it )
  {
    mitk::NavigationDataSource::Pointer currentDevice = dynamic_cast<mitk::NavigationDataSource*>(context->GetService(*it));
    if ( currentDevice.IsNotNull() )
    {
      if ( currentDevice->GetName() == name )
      {
        return currentDevice;
      }
    }
  }

  return 0;
}
