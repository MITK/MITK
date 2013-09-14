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
#ifndef BERRYABSTRACTPREFERENCESSTORAGE_H_
#define BERRYABSTRACTPREFERENCESSTORAGE_H_

#include <org_blueberry_core_runtime_Export.h>
#include "berryIPreferences.h"

#include "Poco/File.h"

namespace berry
{

  ///
  /// Interface to flush Preferences.
  ///
  class BERRY_RUNTIME AbstractPreferencesStorage : public Object
  {

  public:
    berryObjectMacro(berry::AbstractPreferencesStorage);

    ///
    /// Saves the path, sets the root initially to 0.
    /// In subclasses try to read data from file here.
    ///
    AbstractPreferencesStorage(const Poco::File& _File);

    ///
    /// Pure virtual (abstract class)
    ///
    virtual ~AbstractPreferencesStorage();

    ///
    /// Flushes the given (or all) prefs persistently
    ///
    virtual void Flush(IPreferences* prefs) throw(Poco::Exception, BackingStoreException) = 0;

    ///
    /// Returns the root prefs
    ///
    virtual IPreferences::Pointer GetRoot() const;

    ///
    /// Returns the path of the file
    ///
    virtual const Poco::File& GetFile() const;

    ///
    /// Sets the file
    ///
    virtual void SetFile(const Poco::File& f);

  protected:

    ///
    /// Path to the file where the data is stored
    ///
    Poco::File m_File;
    ///
    /// Pointer to the root Preferences
    ///
    IPreferences::Pointer m_Root;
  };
}
#endif /* BERRYABSTRACTPREFERENCESSTORAGE_H_ */
