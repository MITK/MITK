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

#include "berryDebugOptions.h"

#include <berryPlatform.h>
#include <berryLog.h>

#include <ctkPluginContext.h>
#include <ctkException.h>

#include <QSettings>
#include <QMutex>
#include <QDir>
#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
#include <QDesktopServices>
#else
#include <QStandardPaths>
#endif

namespace berry {


const QString DebugOptions::BLUEBERRY_DEBUG = "blueberry.debug";
const QString DebugOptions::OPTIONS = ".blueberry.options";


DebugOptions::DebugOptions()
  : enabled(false)
  , context(nullptr)
{
  // if no debug option was specified, don't even bother to try.
  // Must ensure that the options slot is null as this is the signal to the
  // platform that debugging is not enabled.
  QString debugOptionsFilename = Platform::GetProperty(BLUEBERRY_DEBUG);
  if (debugOptionsFilename.isNull()) return;

  if (debugOptionsFilename.isEmpty())
  {
    // default options location is user.dir (install location may be r/o so
    // is not a good candidate for a trace options that need to be updatable by
    // by the user)
#if (QT_VERSION < QT_VERSION_CHECK(5,0,0))
    QDir userDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
#else
    QDir userDir(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).front());
#endif
    debugOptionsFilename = userDir.absoluteFilePath(OPTIONS);
  }
  QFile optionsFile(debugOptionsFilename);
  QString msgState;
  if (!optionsFile.exists())
  {
    msgState = "not found";
  }
  else
  {
    QSettings settings(debugOptionsFilename, QSettings::IniFormat);
    if (settings.status() != QSettings::NoError)
    {
      msgState = ".... did not parse";
    }
    else
    {
      foreach (const QString& key, settings.allKeys())
      {
        this->options.insert(key, settings.value(key));
      }
      this->enabled = true;
    }
  }
  BERRY_INFO << "Debug options:\n    " << optionsFile.fileName() << "    " << msgState;
}

void DebugOptions::Start(ctkPluginContext* pc)
{
  this->context = pc;
  this->listenerTracker.reset(new ctkServiceTracker<DebugOptionsListener*>(pc, this));
  this->listenerTracker->open();
}

void DebugOptions::Stop(ctkPluginContext* /*pc*/)
{
  this->listenerTracker->close();
  this->listenerTracker.reset();
  this->context = nullptr;
}

bool DebugOptions::GetBooleanOption(const QString& option, bool defaultValue) const
{
  return this->GetOption(option, defaultValue).toBool();
}

QVariant DebugOptions::GetOption(const QString& option) const
{
  return GetOption(option, QVariant());
}

QVariant DebugOptions::GetOption(const QString& option, const QVariant& defaultValue) const
{
  QMutexLocker lock(&this->mutex);
  return this->options.value(option, defaultValue);
}

int DebugOptions::GetIntegerOption(const QString& option, int defaultValue) const
{
  return this->options.value(option, defaultValue).toInt();
}

QHash<QString, QVariant> DebugOptions::GetOptions() const
{
  QHash<QString, QVariant> snapShot;
  {
    QMutexLocker lock(&this->mutex);
    if (this->IsDebugEnabled())
    {
      snapShot = this->options;
    }
    else
    {
      snapShot = this->disabledOptions;
    }
  }
  return snapShot;
}

void DebugOptions::SetOption(const QString& option, const QVariant& value)
{
  if (!this->IsDebugEnabled()) return;

  QString fireChangedEvent;
  {
    QMutexLocker lock(&this->mutex);
    // get the current value
    auto currentValue = this->options.find(option);
    if (currentValue != this->options.end())
    {
      if (currentValue.value() != value)
      {
        fireChangedEvent = this->GetSymbolicName(option);
      }
    }
    else
    {
      if (!value.isNull())
      {
        fireChangedEvent = this->GetSymbolicName(option);
      }
    }
    if (!fireChangedEvent.isEmpty())
    {
      this->options.insert(option, value);
    }
  }
  // Send the options change event outside the sync block
  if (!fireChangedEvent.isEmpty())
  {
    this->OptionsChanged(fireChangedEvent);
  }
}

void DebugOptions::SetOptions(const QHash<QString, QVariant>& ops)
{
  QHash<QString, QVariant> newOptions = ops;
  QSet<QString> fireChangesTo;
  {
    QMutexLocker lock(&this->mutex);
    if (!this->IsDebugEnabled())
    {
      this->disabledOptions = newOptions;
      // no events to fire
      return;
    }
    // first check for removals
    foreach (const QString& key, this->options.keys())
    {
      if (!newOptions.contains(key))
      {
        QString symbolicName = this->GetSymbolicName(key);
        if (!symbolicName.isEmpty())
        {
          fireChangesTo.insert(symbolicName);
        }
      }
    }
    // now check for changes to existing values
    for(auto iter = newOptions.begin(); iter != newOptions.end(); ++iter)
    {
      QVariant existingValue = this->options.value(iter.key());
      if (iter.value() != existingValue)
      {
        QString symbolicName = this->GetSymbolicName(iter.key());
        if (!symbolicName.isEmpty())
        {
          fireChangesTo.insert(symbolicName);
        }
      }
    }
    // finally set the actual options
    this->options = newOptions;
  }
  foreach (const QString& symbolicName, fireChangesTo)
  {
    this->OptionsChanged(symbolicName);
  }
}

void DebugOptions::RemoveOption(const QString& option)
{
  if (!this->IsDebugEnabled()) return;
  QString fireChangedEvent;
  {
    QMutexLocker lock(&this->mutex);
    if (this->options.remove(option))
    {
      fireChangedEvent = this->GetSymbolicName(option);
    }
  }
  // Send the options change event outside the sync block
  if (!fireChangedEvent.isEmpty())
  {
    this->OptionsChanged(fireChangedEvent);
  }
}

bool DebugOptions::IsDebugEnabled() const
{
  QMutexLocker lock(&this->mutex);
  return this->enabled;
}

void DebugOptions::SetDebugEnabled(bool enabled)
{
  bool fireChangedEvent = false;
  {
    QMutexLocker lock(&this->mutex);
    if (enabled)
    {
      if (this->IsDebugEnabled()) return;

      // enable platform debugging - there is no .options file
      Platform::GetConfiguration().setString(BLUEBERRY_DEBUG.toStdString(), "");
      this->options = this->disabledOptions;
      this->disabledOptions.clear();
      this->enabled = true;
      if (!this->options.isEmpty())
      {
        // fire changed event to indicate some options were re-enabled
        fireChangedEvent = true;
      }
    }
    else
    {
      if (!this->IsDebugEnabled()) return;
      // disable platform debugging.
      Platform::GetConfiguration().remove(BLUEBERRY_DEBUG.toStdString());
      if (!this->options.isEmpty())
      {
        // Save the current options off in case debug is re-enabled
        this->disabledOptions = this->options;
        // fire changed event to indicate some options were disabled
        fireChangedEvent = true;
      }
      this->options.clear();
      this->enabled = false;
    }
  }
  if (fireChangedEvent)
  {
    // need to fire event to listeners that options have been disabled
    this->OptionsChanged("*");
  }
}

QString DebugOptions::GetSymbolicName(const QString& option) const
{
  int firstSlashIndex = option.indexOf("/");
  if (firstSlashIndex > 0)
    return option.left(firstSlashIndex);
  return QString::null;
}

void DebugOptions::OptionsChanged(const QString& pluginSymbolicName)
{
  // use osgi services to get the listeners
  if (context == nullptr)
    return;
  // do not use the service tracker because that is only used to call all listeners initially when they are registered
  // here we only want the services with the specified name.
  QList<ctkServiceReference> listenerRefs;
  try
  {
    listenerRefs = context->getServiceReferences<DebugOptionsListener>( "(" + DebugOptions::LISTENER_SYMBOLICNAME + "=" + pluginSymbolicName + ")");
  }
  catch (const ctkInvalidArgumentException& /*e*/)
  {
    // consider logging; should not happen
  }
  if (listenerRefs.empty()) return;

  foreach (const ctkServiceReference& ref, listenerRefs)
  {
    DebugOptionsListener* service = context->getService<DebugOptionsListener>(ref);
    if (service == nullptr) continue;

    try
    {
      service->OptionsChanged(*this);
    }
    catch (const std::exception& /*e*/)
    {
      // TODO consider logging
    }
    context->ungetService(ref);
  }
}

DebugOptionsListener* DebugOptions::addingService(const ctkServiceReference& reference)
{
  DebugOptionsListener* listener = context->getService<DebugOptionsListener>(reference);
  listener->OptionsChanged(*this);
  return listener;
}

void DebugOptions::modifiedService(const ctkServiceReference& /*reference*/,
                                         DebugOptionsListener* /*service*/)
{
  // nothing
}

void DebugOptions::removedService(const ctkServiceReference& reference,
                    DebugOptionsListener* /*service*/)
{
  context->ungetService(reference);
}

}
