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

#ifndef BERRYIDEBUGOPTIONS_H
#define BERRYIDEBUGOPTIONS_H

#include <org_blueberry_core_runtime_Export.h>

#include <QString>
#include <QHash>
#include <QVariant>

namespace berry {

/**
 * Used to get debug options settings.
 */
struct org_blueberry_core_runtime_EXPORT IDebugOptions
{

  virtual ~IDebugOptions();

  /**
   * The service property (named &quot;listener.symbolic.name&quot;) which specifies
   * the bundle symbolic name of a {@link DebugOptionsListener} service.
   */
  static const QString LISTENER_SYMBOLICNAME; // = "listener.symbolic.name";

  /**
   * Returns the identified option as a boolean value. The specified
   * defaultValue is returned if no such option is found or if debug is not enabled.
   *
   * <p>
   * Options are specified in the general form <i>&lt;Plugin-SymbolicName&gt;/&lt;option-path&gt;</i>.
   * For example, <code>org.blueberry.core.runtime/debug</code>
   * </p>
   *
   * @param option the name of the option to lookup
   * @param defaultValue the value to return if no such option is found
   * @return the value of the requested debug option or the
   * defaultValue if no such option is found.
   */
  virtual bool GetBooleanOption(const QString& option, bool defaultValue) const = 0;

  /**
   * Returns the identified option.  A null value
   * is returned if no such option is found or if debug is not enabled.
   *
   * <p>
   * Options are specified
   * in the general form <i>&lt;Plugin-SymbolicName&gt;/&lt;option-path&gt;</i>.
   * For example, <code>org.blueberry.core.runtime/debug</code>
   * </p>
   *
   * @param option the name of the option to lookup
   * @return the value of the requested debug option or <code>null</code>
   */
  virtual QVariant GetOption(const QString& option) const = 0;

  /**
   * Returns the identified option. The specified defaultValue is
   * returned if no such option is found or if debug is not enabled.
   *
   * <p>
   * Options are specified
   * in the general form <i>&lt;Plugin-SymbolicName&gt;/&lt;option-path&gt;</i>.
   * For example, <code>org.blueberry.core.runtime/debug</code>
   * </p>
   *
   * @param option the name of the option to lookup
   * @param defaultValue the value to return if no such option is found
   * @return the value of the requested debug option or the
   * defaultValue if no such option is found.
   */
  virtual QVariant GetOption(const QString& option, const QVariant& defaultValue) const = 0;

  /**
   * Returns the identified option as an int value. The specified
   * defaultValue is returned if no such option is found or if an
   * error occurs while converting the option value
   * to an integer or if debug is not enabled.
   *
   * <p>
   * Options are specified
   * in the general form <i>&lt;Plugin-SymbolicName&gt;/&lt;option-path&gt;</i>.
   * For example, <code>org.blueberry.core.runtime/debug</code>
   * </p>
   *
   * @param option the name of the option to lookup
   * @param defaultValue the value to return if no such option is found
   * @return the value of the requested debug option or the
   * defaultValue if no such option is found.
   */
  virtual int GetIntegerOption(const QString& option, int defaultValue) const = 0;

  /**
   * Returns a snapshot of the current options. All
   * keys and values are of type <code>string</code>.  If no
   * options are set then an empty map is returned.
   * <p>
   * If debug is not enabled then the snapshot of the current disabled
   * values is returned. See SetDebugEnabled(bool).
   * </p>
   * @return a snapshot of the current options.
   */
  virtual QHash<QString, QVariant> GetOptions() const = 0;

  /**
   * Sets the identified option to the identified value. If debug is
   * not enabled then the specified option is not changed.
   * @param option the name of the option to set
   * @param value the value of the option to set
   */
  virtual void SetOption(const QString& option, const QVariant& value) = 0;

  /**
   * Sets the current option key/value pairs to the specified options.
   * The specified map replaces all keys and values of the current debug options.
   * <p>
   * If debug is not enabled then the specified options are saved as
   * the disabled values and no notifications will be sent.
   * See SetDebugEnabled(bool).
   * If debug is enabled then notifications will be sent to the
   * listeners which have options that have been changed, added or removed.
   * </p>
   *
   * @param options the new set of options
   */
  virtual void SetOptions(const QHash<QString, QVariant>& ops) = 0;

  /**
   * Removes the identified option.  If debug is not enabled then
   * the specified option is not removed.
   * @param option the name of the option to remove
   */
  virtual void RemoveOption(const QString& option) = 0;

  /**
   * Returns true if debugging/tracing is currently enabled.
   * @return true if debugging/tracing is currently enabled;  Otherwise false is returned.
   */
  virtual bool IsDebugEnabled() const = 0;

  /**
   * Enables or disables debugging/tracing.
   * <p>
   * When debug is disabled all debug options are unset.
   * When disabling debug the current debug option values are
   * stored in memory as disabled values. If debug is re-enabled the
   * disabled values will be set back and enabled.  The disabled values
   * are only stored in memory and if the framework is restarted then
   * the disabled option values will be lost.
   * </p>
   * @param value If <code>true</code>, debug is enabled, otherwise
   * debug is disabled.
   */
  virtual void SetDebugEnabled(bool enabled) = 0;

};

}

Q_DECLARE_INTERFACE(berry::IDebugOptions, "org.blueberry.IDebugOptions")

#endif // BERRYIDEBUGOPTIONS_H
