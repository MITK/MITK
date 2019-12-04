/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef BERRYIBERRYPREFERENCESSERVICE_H_
#define BERRYIBERRYPREFERENCESSERVICE_H_

#include "berryIPreferencesService.h"

namespace berry
{

struct IBerryPreferencesService : public IPreferencesService
{
  /**
   * Try to import the prefs from the given file.
   * Existing properties will be overridden!
   * Returns true if the preferences could be imported.
   */
  virtual void ImportPreferences(const QString& f, const QString& name="") = 0;

  /**
   * Exports the current system preferences to the given file.
   */
  virtual void ExportPreferences(const QString& f, const QString& name="") = 0;

};

}  // namespace berry

Q_DECLARE_INTERFACE(berry::IBerryPreferencesService, "org.blueberry.core.runtime.IBerryPreferencesService")

#endif /*BERRYIBERRYPREFERENCESSERVICE_H_*/
