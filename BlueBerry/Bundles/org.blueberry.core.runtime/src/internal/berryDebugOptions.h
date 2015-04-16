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

#ifndef BERRYDEBUGOPTIONS_H
#define BERRYDEBUGOPTIONS_H

#include <berryIDebugOptions.h>

#include "berryDebugOptionsListener.h"

#include <ctkServiceTracker.h>
#include <ctkServiceTrackerCustomizer.h>

namespace berry {

/**
 * Used to get debug options settings.
 */
class org_blueberry_core_runtime_EXPORT DebugOptions : public QObject, public IDebugOptions,
    private ctkServiceTrackerCustomizer<DebugOptionsListener*>
{
  Q_OBJECT
  Q_INTERFACES(berry::IDebugOptions)

public:

  DebugOptions();

  void Start(ctkPluginContext* pc);
  void Stop(ctkPluginContext* pc);

  bool GetBooleanOption(const QString& option, bool defaultValue) const;
  QVariant GetOption(const QString& option) const;
  QVariant GetOption(const QString& option, const QVariant& defaultValue) const;

  int GetIntegerOption(const QString& option, int defaultValue) const;

  QHash<QString, QVariant> GetOptions() const;

  void SetOption(const QString& option, const QVariant& value);

  void SetOptions(const QHash<QString, QVariant>& ops);

  void RemoveOption(const QString& option);

  bool IsDebugEnabled() const;

  void SetDebugEnabled(bool enabled);

private:

  static const QString BLUEBERRY_DEBUG;

  /** mutex used to lock the options maps */
  mutable QMutex mutex;
  /** A boolean value indicating if debug was enabled */
  bool enabled;
  /** A current map of all the options with values set */
  QHash<QString, QVariant> options;
  /** A map of all the disabled options with values set at the time debug was disabled */
  QHash<QString, QVariant> disabledOptions;
  /** The singleton object of this class */
  //static Impl* singleton = nullptr;
  /** The default name of the .options file if loading when the -debug command-line argument is used */
  static const QString OPTIONS;

  ctkPluginContext* context;
  QScopedPointer<ctkServiceTracker<DebugOptionsListener*> > listenerTracker;

  QString GetSymbolicName(const QString& option) const;

  /**
   * Notifies the trace listener for the specified plug-in that its option-path has changed.
   * @param pluginSymbolicName The plug-in of the owning trace listener to notify.
   */
  void OptionsChanged(const QString& bundleSymbolicName);

  DebugOptionsListener* addingService(const ctkServiceReference& reference);
  void modifiedService(const ctkServiceReference& reference,
                       DebugOptionsListener* service);
  void removedService(const ctkServiceReference& reference,
                     DebugOptionsListener* service);

};

}

#endif // BERRYDEBUGOPTIONS_H
