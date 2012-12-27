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

#include "Poco/File.h"

#include <vector>
#include <string>

namespace berry
{
  struct IBerryPreferencesService : public IPreferencesService
  {
    berryInterfaceMacro(IBerryPreferencesService, berry)

    ///
    /// Try to import the prefs from the given file.
    /// Existing properties will be overridden!
    /// Returns true if the preferences could be imported.
    ///
    virtual void ImportPreferences(Poco::File f, std::string name="") = 0;

    ///
    /// Exports the current system preferences to the given file.
    ///
    virtual void ExportPreferences(Poco::File f, std::string name="") = 0;

  };
}  // namespace berry

#endif /*BERRYIBERRYPREFERENCESSERVICE_H_*/
