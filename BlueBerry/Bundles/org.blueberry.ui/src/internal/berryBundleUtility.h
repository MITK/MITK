/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#ifndef BERRYBUNDLEUTILITY_H_
#define BERRYBUNDLEUTILITY_H_

#include <berryIBundle.h>

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

  static bool IsActive(IBundle::Pointer bundle);

  static bool IsActivated(IBundle::Pointer bundle);

  // TODO: needs a better name
  static bool IsReady(IBundle::Pointer bundle);

  static bool IsReady(QSharedPointer<ctkPlugin> plugin);

  static bool IsReady(IBundle::State bundleState);

  static bool IsReady(ctkPlugin::State pluginState);

  static bool IsActive(const std::string& bundleId);

  static bool IsActivated(const std::string& bundleId);

  static bool IsReady(const std::string& bundleId);

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
//      Log(const std::string& bundleId, const Poco::Exception* exception);
};

}

#endif /* BERRYBUNDLEUTILITY_H_ */
