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

#ifndef BERRYBUNDLEUTILITY_H_
#define BERRYBUNDLEUTILITY_H_

#include <ctkPlugin.h>

#include <QSharedPointer>

namespace berry
{

/**
 * A set of static methods that provide an nicer interface to common platform
 * operations related to bundle management.
 */
class BundleUtility
{
public:

  static bool IsActive(ctkPlugin* plugin);

  static bool IsActivated(ctkPlugin* plugin);

  // TODO: needs a better name
  static bool IsReady(ctkPlugin* plugin);

  static bool IsReady(ctkPlugin::State pluginState);

  static bool IsActive(const QString& bundleId);

  static bool IsActivated(const QString& bundleId);

  static bool IsReady(const QString& bundleId);

  static QSharedPointer<ctkPlugin> FindPlugin(const QString& symbName);

  //    static URL find(Bundle bundle, String path) {
  //        if (bundle == null) {
  //      return null;
  //    }
  //        return Platform.find(bundle, new Path(path));
  //    }
  //
  //    static URL find(String bundleId, String path) {
  //        return find(Platform.getBundle(bundleId), path);
  //    }

//  static void
//      Log(const QString& bundleId, const Poco::Exception* exception);
};

}

#endif /* BERRYBUNDLEUTILITY_H_ */
