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

#ifndef BERRYIEXECUTABLEEXTENSIONFACTORY_H
#define BERRYIEXECUTABLEEXTENSIONFACTORY_H

#include <QtPlugin>

#include <org_blueberry_core_runtime_Export.h>

namespace berry {

/**
 * This interface allows extension providers to control how the instances provided
 * to extension-points are being created by referring to the factory instead of
 * referring to a class. For example, the following extension to the preference page
 * extension-point uses a factory called <code>PreferencePageFactory</code>.
 * \code
 * <extension point="org.blueberry.ui.preferencePages">
 *   <page  name="..."  class="org.eclipse.update.ui.PreferencePageFactory:org.eclipse.update.ui.preferences.MainPreferencePage">
 *   </page>
 * </extension>
 * \endcode
 *
 * <p>
 * Effectively, factories give full control over the create executable extension process.
 * </p><p>
 * The factories are responsible for handling the case where the concrete instance
 * implements {@link IExecutableExtension}.
 * </p><p>
 * Given that factories are instantiated as executable extensions, they must provide a
 * 0-argument public constructor.
 * Like any other executable extension, they can be configured by implementing
 * the IExecutableExtension interface.
 * </p><p>
 *
 * @see IConfigurationElement
 */
struct org_blueberry_core_runtime_EXPORT IExecutableExtensionFactory
{

  virtual ~IExecutableExtensionFactory();

  /**
   * Creates and returns a new instance.
   *
   * @exception CoreException if an instance of the executable extension
   *   could not be created for any reason
   */
  virtual QObject* Create() = 0;
};

}

Q_DECLARE_INTERFACE(berry::IExecutableExtensionFactory, "org.blueberry.IExecutableExtensionFactory")

#endif // BERRYIEXECUTABLEEXTENSIONFACTORY_H
