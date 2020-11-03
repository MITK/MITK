/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryRegistryStrategy.h"

#include "berryCoreException.h"
#include "berryCTKPluginListener.h"
#include "berryExtensionRegistry.h"
#include "berryExtensionType.h"
#include "berryRegistryConstants.h"
#include "berryRegistryContributor.h"
#include "berryRegistryMessages.h"
#include "berryRegistrySupport.h"
#include "berryStatus.h"
#include "berryLog.h"
#include "berryCTKPluginActivator.h"
#include "berryCTKPluginUtils.h"

#include <ctkPlugin.h>
#include <ctkPluginContext.h>
#include <ctkUtils.h>

#include <QFileInfo>
#include <QDateTime>
#include <QXmlSimpleReader>

namespace berry {

RegistryStrategy::RegistryStrategy(const QList<QString>& storageDirs, const QList<bool>& cacheReadOnly,
                                   QObject* key)
  : storageDirs(storageDirs), cacheReadOnly(cacheReadOnly), token(key), trackTimestamp(false)
{
  // Only do timestamp calculations if osgi.checkConfiguration is set to "true" (typically,
  // this implies -dev mode)
  ctkPluginContext* context = org_blueberry_core_runtime_Activator::getPluginContext();
  if (context)
  {
    trackTimestamp = context->getProperty(RegistryConstants::PROP_CHECK_CONFIG).toString().compare("true", Qt::CaseInsensitive) == 0;
  }
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

void RegistryStrategy::OnStart(IExtensionRegistry* reg, bool loadedFromCache)
{
  ExtensionRegistry* registry = dynamic_cast<ExtensionRegistry*>(reg);
  if (registry == nullptr)
    return;

  // register a listener to catch new plugin installations/resolutions.
  pluginListener.reset(new CTKPluginListener(registry, token, this));
  org_blueberry_core_runtime_Activator::getPluginContext()->connectPluginListener(
        pluginListener.data(), SLOT(PluginChanged(ctkPluginEvent)), Qt::DirectConnection);

  // populate the registry with all the currently installed plugins.
  // There is a small window here while ProcessPlugins is being
  // called where the pluginListener may receive a ctkPluginEvent
  // to add/remove a plugin from the registry. This is ok since
  // the registry is a synchronized object and will not add the
  // same bundle twice.
  if (!loadedFromCache)
    pluginListener->ProcessPlugins(org_blueberry_core_runtime_Activator::getPluginContext()->getPlugins());
}

void RegistryStrategy::OnStop(IExtensionRegistry* /*registry*/)
{
  if (!pluginListener.isNull())
  {
    org_blueberry_core_runtime_Activator::getPluginContext()->disconnectPluginListener(pluginListener.data());
  }
}

QObject* RegistryStrategy::CreateExecutableExtension(const SmartPointer<RegistryContributor>& contributor,
                                           const QString& className, const QString& /*overridenContributorName*/)
{
  QObject* result = nullptr;

  QSharedPointer<ctkPlugin> plugin = CTKPluginUtils::GetDefault()->GetPlugin(contributor->GetName());
  if (!plugin.isNull())
  {
    // immediately start the plugin but do not change the plugins autostart setting
    plugin->start(ctkPlugin::START_TRANSIENT);
  }
  else
  {
    QString message = QString("Unable to find plugin \"%1\" for contributor \"%2\".")
        .arg(contributor->GetName()).arg(contributor->GetActualName());
    IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, RegistryMessages::OWNER_NAME,
                                       RegistryConstants::PLUGIN_ERROR, message, BERRY_STATUS_LOC));
    throw CoreException(status);
  }

  //QString typeName = contributor->GetActualName() + "_" + className;
  QString typeName = className;
  int extensionTypeId = ExtensionType::type(typeName.toLatin1().data());
  if (extensionTypeId == 0)
  {
    QString message = QString("Unable to find class \"%1\" from contributor \"%2\"."
                              " The class was either not registered via "
                              "BERRY_REGISTER_EXTENSION_CLASS(type, pluginContext) "
                              "or you forgot to run Qt's moc on the header file.")
        .arg(className).arg(contributor->GetActualName());
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

bool RegistryStrategy::CheckContributionsTimestamp() const
{
  return trackTimestamp;
}

long RegistryStrategy::GetExtendedTimestamp(const QSharedPointer<ctkPlugin>& plugin, const QString& pluginManifest) const
{
  if (pluginManifest.isEmpty())
    return 0;

  // The plugin manifest does not have a timestamp as it is embedded into
  // the plugin itself. Try to get the timestamp of the plugin instead.
  QFileInfo pluginInfo(QUrl(plugin->getLocation()).toLocalFile());
  if (pluginInfo.exists())
  {
    return ctk::msecsTo(QDateTime::fromTime_t(0), pluginInfo.lastModified()) + plugin->getPluginId();
    //return pluginManifest.openConnection().getLastModified() + bundle.getBundleId();
  }
  else
  {
    if (Debug())
    {
      BERRY_DEBUG << "Unable to obtain timestamp for the plugin " <<
                     plugin->getSymbolicName();
    }
    return 0;
  }
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
                                 const SmartPointer<IContributor>& /*contributor*/,
                                 const QLocale& /*locale*/)
{
  return nonTranslated;
}

QLocale RegistryStrategy::GetLocale() const
{
  return QLocale();
}

}
