/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIAPPLICATIONCONTEXT_H
#define BERRYIAPPLICATIONCONTEXT_H

#include <org_blueberry_core_runtime_Export.h>

#include <QHash>
#include <QVariant>
#include <QString>

namespace berry {

/**
 * The context used to start an application.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 * @noextend This interface is not intended to be extended by clients.
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct org_blueberry_core_runtime_EXPORT IApplicationContext
{

  virtual ~IApplicationContext();

  /**
   * A key used to store arguments for the application.  The content of this argument
   * is unchecked and should conform to the expectations of the application being invoked.
   * Typically this is a <code>QStringList</code>.
   * <p>
   *
   * If the properties used to launch an application do
   * not contain a value for this key then command line arguments used to launch
   * the platform are set in the arguments of the application context.
   */
  static const QString APPLICATION_ARGS; // = "application.args";

  /**
   * A key used to store unprocessed arguments for the application.
   * This is a <code>QStringList</code>.
   * <p>
   *
   * If the properties used to launch an application do
   * not contain a value for this key then command line arguments used to launch
   * the platform are set in the unprocessed arguments of the application context.
   */
  static const QString APPLICATION_ARGS_UNPROCESSED; // = "application.args.unprocessed";

  /**
   * The arguments used for the application.  The arguments from
   * QObject::dynamicPropertyNames() of a QObject service object registered under
   * the interface "org.blueberry.core.runtime.AppDescriptor" are used as the arguments
   * for this context when an application is launched.
   *
   * @return a map of application arguments.
   */
  virtual QHash<QString, QVariant> GetArguments() const = 0;

  /**
   * This method should be called once the application is completely initialized and running.
   * This method will perform certain operations that are needed once an application is running.
   * One example is bringing down a splash screen if it exists.
   */
  virtual void ApplicationRunning() = 0;

};

}

Q_DECLARE_INTERFACE(berry::IApplicationContext, "org.blueberry.IApplicationContext")

#endif // BERRYIAPPLICATIONCONTEXT_H
