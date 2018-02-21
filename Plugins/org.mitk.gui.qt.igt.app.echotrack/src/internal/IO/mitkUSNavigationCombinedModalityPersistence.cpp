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

#include "mitkUSNavigationCombinedModalityPersistence.h"
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

#include "internal/org_mbi_gui_qt_usnavigation_Activator.h"

mitk::USNavigationCombinedModalityPersistence::USNavigationCombinedModalityPersistence()
{
  us::ModuleContext* context = us::GetModuleContext();

  this->LoadStoredDevices();

  // to be notified about every register and unregister of an USDevice
  std::string filterExcludeCombinedModalities = "(&(" + us::ServiceConstants::OBJECTCLASS() + "="
      + "org.mitk.services.UltrasoundDevice)(!(" + mitk::USDevice::GetPropertyKeys().US_PROPKEY_CLASS + "="
      + mitk::USCombinedModality::DeviceClassIdentifier + ")))";
  context->AddServiceListener(this, &mitk::USNavigationCombinedModalityPersistence::OnServiceEvent,
    filterExcludeCombinedModalities);

  // to be notified about every register and unregister of an NavigationDataSource
  context->AddServiceListener(this, &mitk::USNavigationCombinedModalityPersistence::OnServiceEvent,
    "(" + us::ServiceConstants::OBJECTCLASS() + "=" + us_service_interface_iid<mitk::NavigationDataSource>() + ")");
}

mitk::USNavigationCombinedModalityPersistence::~USNavigationCombinedModalityPersistence()
{
  //this->StoreCurrentDevices(); //disabled because persistence is buggy
}

void mitk::USNavigationCombinedModalityPersistence::OnServiceEvent(const us::ServiceEvent event)
{
  if ( event.GetType() == event.REGISTERED )
  {
    //this->LoadStoredDevices(); //disabled because persistence is buggy
  }
}

void mitk::USNavigationCombinedModalityPersistence::StoreCurrentDevices()
{
  QList<QVariant> devices;

  us::ModuleContext* context = us::GetModuleContext();

  // get all combined modality from the service registry
  std::string filterOnlyCombinedModalities = "(&(" + us::ServiceConstants::OBJECTCLASS() + "=" + "org.mitk.services.UltrasoundDevice)(" + mitk::USDevice::GetPropertyKeys().US_PROPKEY_CLASS + "=" + mitk::USCombinedModality::DeviceClassIdentifier + "))";
  std::vector<us::ServiceReference<USDevice> > services = context->GetServiceReferences<USDevice>(filterOnlyCombinedModalities);

  for ( std::vector<us::ServiceReference<USDevice> >::iterator it = services.begin();
        it != services.end(); ++it )
  {
    QStringList deviceStrings;
    mitk::USCombinedModality::Pointer currentDevice = dynamic_cast<mitk::USCombinedModality*>(context->GetService(*it));
    if ( currentDevice.IsNotNull() )
    {
      // save manufacturer and model strig of the combined modality
      deviceStrings.push_back(QString::fromStdString(currentDevice->GetUltrasoundDevice()->GetManufacturer()));
      deviceStrings.push_back(QString::fromStdString(currentDevice->GetUltrasoundDevice()->GetName()));

      // save name of the navigation data source
      mitk::NavigationDataSource::Pointer navigationDataSource = currentDevice->GetNavigationDataSource();
      if ( currentDevice.IsNull() ) { continue; }
      deviceStrings.push_back(QString::fromStdString(navigationDataSource->GetName()));

      // save manufacturer, model and comment of the ultrasound device
      mitk::USDevice::Pointer ultrasoundDevice = currentDevice->GetUltrasoundDevice();
      if ( ultrasoundDevice.IsNull() ) { continue; }
      deviceStrings.push_back(QString::fromStdString(ultrasoundDevice->GetManufacturer()));
      deviceStrings.push_back(QString::fromStdString(ultrasoundDevice->GetName()));
      deviceStrings.push_back(QString::fromStdString(ultrasoundDevice->GetComment()));

      // save calibration of the combined modality
      deviceStrings.push_back(QString::fromStdString(currentDevice->SerializeCalibration()));
    }

    devices.push_back(deviceStrings);
  }

  // store everything in QSettings
  QSettings settings;
  settings.setValue("combined-modalities", devices);
}

void mitk::USNavigationCombinedModalityPersistence::LoadStoredDevices()
{
  // load everything from QSettings
  QSettings settings;
  QList<QVariant> devices = settings.value("combined-modalities").value< QList<QVariant> >();

  for ( QList<QVariant>::iterator it = devices.begin();
        it != devices.end(); ++it )
  {
    // get the saved strings for the combined modality (there must be at least
    // the seven strings, which where saved before)
    QStringList stringList = it->toStringList();

    // test if the combined modality wasn't restored before
    if (stringList.size() >= 7 && this->GetCombinedModality(stringList.at(0).toStdString(), stringList.at(1).toStdString()).IsNull())
    {
      // try to get matching navigation data source and ultrasound device
      mitk::NavigationDataSource::Pointer navigationDataSource = this->GetNavigationDataSource(stringList.at(2).toStdString());
      mitk::USDevice::Pointer usDevice = this->GetUSDevice(stringList.at(3).toStdString(), stringList.at(4).toStdString(), stringList.at(5).toStdString());

      // create the combined modality if matching navigation data source and
      // ultrasound device werde found
      if ( navigationDataSource.IsNotNull() && usDevice.IsNotNull() )
      {
        mitk::USCombinedModality::Pointer combinedModality = mitk::USCombinedModality::New(usDevice, navigationDataSource, false);
        combinedModality->DeserializeCalibration(stringList.at(6).toStdString());
        combinedModality->GetUltrasoundDevice()->Initialize();
      }
    }
  }
}

mitk::USCombinedModality::Pointer mitk::USNavigationCombinedModalityPersistence::GetCombinedModality(std::string manufacturer, std::string model)
{
  us::ModuleContext* context = us::GetModuleContext();

  std::string filterOnlyCombinedModalities = "(&(" + us::ServiceConstants::OBJECTCLASS() + "="
      + "org.mitk.services.UltrasoundDevice)(" + mitk::USDevice::GetPropertyKeys().US_PROPKEY_CLASS + "="
      + mitk::USCombinedModality::DeviceClassIdentifier + "))";
  std::vector<us::ServiceReference<USDevice> > services = context->GetServiceReferences<USDevice>(filterOnlyCombinedModalities);

  for ( std::vector<us::ServiceReference<USDevice> >::iterator it = services.begin();
        it != services.end(); ++it )
  {
    mitk::USCombinedModality::Pointer currentDevice = dynamic_cast<mitk::USCombinedModality*>(context->GetService(*it));
    if ( currentDevice.IsNotNull() )
    {
      if ( currentDevice->GetUltrasoundDevice()->GetManufacturer() == manufacturer && currentDevice->GetUltrasoundDevice()->GetName() == model )
      {
        return currentDevice;
      }
    }
  }

  return 0;
}

mitk::USDevice::Pointer mitk::USNavigationCombinedModalityPersistence::GetUSDevice(std::string manufacturer, std::string model, std::string comment)
{
  us::ModuleContext* context = us::GetModuleContext();

  std::string filterExcludeCombinedModalities = "(&(" + us::ServiceConstants::OBJECTCLASS() + "="
      + "org.mitk.services.UltrasoundDevice)(!(" + mitk::USDevice::GetPropertyKeys().US_PROPKEY_CLASS + "="
      + mitk::USCombinedModality::DeviceClassIdentifier + ")))";
  std::vector<us::ServiceReference<USDevice> > services = context->GetServiceReferences<USDevice>(filterExcludeCombinedModalities);

  for ( std::vector<us::ServiceReference<USDevice> >::iterator it = services.begin();
        it != services.end(); ++it )
  {
    mitk::USDevice::Pointer currentDevice = dynamic_cast<mitk::USDevice*>(context->GetService(*it));
    if ( currentDevice.IsNotNull() )
    {
      if ( currentDevice->GetManufacturer() == manufacturer && currentDevice->GetName() == model
           && currentDevice->GetComment() == comment )
      {
        return currentDevice;
      }
    }
  }

  return 0;
}

mitk::NavigationDataSource::Pointer mitk::USNavigationCombinedModalityPersistence::GetNavigationDataSource(std::string name)
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
