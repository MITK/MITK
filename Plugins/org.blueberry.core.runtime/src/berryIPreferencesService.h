/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef BERRYIPREFERENCESSERVICE_H_
#define BERRYIPREFERENCESSERVICE_H_

#include <org_blueberry_core_runtime_Export.h>

#include <QtPlugin>

namespace berry
{

template<class T> class SmartPointer;

struct IPreferences;

/**
 * The Preferences Service.
 *
 * <p>
 * Each bundle using this service has its own set of preference trees: one for
 * system preferences, and one for each user.
 *
 * <p>
 * A <code>PreferencesService</code> object is specific to the bundle which
 * obtained it from the service registry. If a bundle wishes to allow another
 * bundle to access its preferences, it should pass its
 * <code>PreferencesService</code> object to that bundle.
 *
 */
struct org_blueberry_core_runtime_EXPORT IPreferencesService
{
  virtual ~IPreferencesService();

  /**
   * Returns the root system node for the calling bundle.
   *
   * @return The root system node for the calling bundle.
   */
  virtual SmartPointer<IPreferences> GetSystemPreferences() = 0;

  /**
   * Returns the root node for the specified user and the calling bundle.
   *
   * @param name The user for which to return the preference root node.
   * @return The root node for the specified user and the calling bundle.
   */
  virtual SmartPointer<IPreferences> GetUserPreferences(const QString& name) = 0;

  /**
   * Returns the names of users for which node trees exist.
   *
   * @return The names of users for which node trees exist.
   */
  virtual QStringList GetUsers() const = 0;
};

}  // namespace berry

Q_DECLARE_INTERFACE(berry::IPreferencesService, "org.blueberry.service.IPreferencesService")

#endif /*BERRYIPREFERENCESSERVICE_H_*/
