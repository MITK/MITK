/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryRegistryStrategy.h"

#include "berryCoreException.h"
#include "berryExtensionRegistry.h"
#include "berryExtensionType.h"
#include "berryRegistryConstants.h"
#include "berryRegistryContributor.h"
#include "berryRegistryMessages.h"
#include "berryRegistrySupport.h"
#include "berryStatus.h"

#include <QXmlSimpleReader>

namespace berry {

RegistryStrategy::RegistryStrategy(const QList<QString>& storageDirs, const QList<bool>& cacheReadOnly)
  : storageDirs(storageDirs), cacheReadOnly(cacheReadOnly)
{
}

RegistryStrategy::~RegistryStrategy()
{
}

int RegistryStrategy::GetLocationsLength() const
{
  return storageDirs.size();
}

QString RegistryStrategy::GetStorage(int index) const
{
  return storageDirs[index];
}

bool RegistryStrategy::IsCacheReadOnly(int index) const
{
  if (!cacheReadOnly.empty())
    return cacheReadOnly[index];
  return true;
}

void RegistryStrategy::Log(const SmartPointer<IStatus>& status)
{
  RegistrySupport::Log(status, QString());
}

QString RegistryStrategy::Translate(const QString& key, QTranslator* resources)
{
  return RegistrySupport::Translate(key, resources);
}

void RegistryStrategy::OnStart(IExtensionRegistry* registry, bool loadedFromCache)
{
  // The default implementation
}

void RegistryStrategy::OnStop(IExtensionRegistry* registry)
{
  // The default implementation
}

QObject* RegistryStrategy::CreateExecutableExtension(const SmartPointer<RegistryContributor>& contributor,
                                           const QString& className, const QString& overridenContributorName)
{
  QObject* result = NULL;

  QString typeName = contributor->GetActualName() + "_" + className;
  int extensionTypeId = ExtensionType::type(typeName.toAscii().data());
  if (extensionTypeId == 0)
  {
    QString message = QString("Contributor \"%1\" was unable to find class \"%2\"."
                              " The class was either not registered via "
                              "BERRY_REGISTER_EXTENSION_CLASS(type, pluginContext) "
                              "or you forgot to run Qt's moc on the header file.").arg(contributor->GetActualName()).arg(className);
    IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, RegistryMessages::OWNER_NAME,
                                       RegistryConstants::PLUGIN_ERROR, message, BERRY_STATUS_LOC));
    throw CoreException(status);
  }
  else
  {
    try
    {
      result = ExtensionType::construct(extensionTypeId);
    }
    catch (const ctkException& e)
    {
      QString message = QString("Contributor \"%1\" was unable to instantiate class \"%2\".")
          .arg(contributor->GetActualName()).arg(className);
      IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, RegistryMessages::OWNER_NAME,
                                         RegistryConstants::PLUGIN_ERROR, message, e, BERRY_STATUS_LOC));
      throw CoreException(status);
    }
    catch (const std::exception& e)
    {
      QString message = QString("Contributor \"%1\" was unable to instantiate class \"%2\". Error: \"%3\"")
          .arg(contributor->GetActualName()).arg(className).arg(QString(e.what()));
      IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, RegistryMessages::OWNER_NAME,
                                         RegistryConstants::PLUGIN_ERROR, message, BERRY_STATUS_LOC));
      throw CoreException(status);
    }
  }

  return result;
}

//void RegistryStrategy::ScheduleChangeEvent(const QList<IRegistryEventListener>& listeners,
//                                           const QHash<QString, CombinedEventDelta>& deltas,
//                                           IExtensionRegistry* registry)
//{
//  if (ExtensionRegistry* extRegistry = dynamic_cast<ExtensionRegistry*>(registry))
//    extRegistry->ScheduleChangeEvent(listeners, deltas);
//}

//SmartPointer<IStatus> RegistryStrategy::ProcessChangeEvent(const QList<IRegistryEventListener>& listeners,
//                                                const QHash<QString, CombinedEventDelta>& deltas,
//                                                IExtensionRegistry* registry)
//{
//  if (ExtensionRegistry* extRegistry = dynamic_cast<ExtensionRegistry*>(registry))
//    return extRegistry->ProcessChangeEvent(listeners, deltas);
//  return IStatus::Pointer();
//}

bool RegistryStrategy::Debug() const
{
  return false;
}

bool RegistryStrategy::DebugRegistryEvents() const
{
  return false;
}

bool RegistryStrategy::CacheUse() const
{
  return true;
}

bool RegistryStrategy::CacheLazyLoading() const
{
  return true;
}

long RegistryStrategy::GetContainerTimestamp() const
{
  return 0;
}

long RegistryStrategy::GetContributionsTimestamp() const
{
  return 0;
}

QXmlReader* RegistryStrategy::GetXMLParser() const
{
  if (theXMLParser.isNull())
  {
    theXMLParser.reset(new QXmlSimpleReader());
  }
  return theXMLParser.data();
}

QList<QString> RegistryStrategy::Translate(const QList<QString>& nonTranslated,
                                 const SmartPointer<IContributor>& contributor,
                                 const QLocale& locale)
{
  return nonTranslated;
}

QLocale RegistryStrategy::GetLocale() const
{
  return QLocale();
}

}
