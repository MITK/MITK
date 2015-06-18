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
