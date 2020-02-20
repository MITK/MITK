/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef BERRYABSTRACTPREFERENCESSTORAGE_H_
#define BERRYABSTRACTPREFERENCESSTORAGE_H_

#include <org_blueberry_core_runtime_Export.h>

#include <berryObject.h>


namespace berry
{

struct IPreferences;

  ///
  /// Interface to flush Preferences.
  ///
  class org_blueberry_core_runtime_EXPORT AbstractPreferencesStorage : public Object
  {

  public:

    berryObjectMacro(berry::AbstractPreferencesStorage);

    ///
    /// Saves the path, sets the root initially to 0.
    /// In subclasses try to read data from file here.
    ///
    AbstractPreferencesStorage(const QString& _File);

    ///
    /// Pure virtual (abstract class)
    ///
    ~AbstractPreferencesStorage() override;

    ///
    /// Flushes the given (or all) prefs persistently
    ///
    virtual void Flush(IPreferences* prefs) = 0;

    ///
    /// Returns the root prefs
    ///
    virtual SmartPointer<IPreferences> GetRoot() const;

    ///
    /// Returns the path of the file
    ///
    virtual QString GetFile() const;

    ///
    /// Sets the file
    ///
    virtual void SetFile(const QString& f);

  protected:

    ///
    /// Path to the file where the data is stored
    ///
    QString m_File;
    ///
    /// Pointer to the root Preferences
    ///
    SmartPointer<IPreferences> m_Root;
  };
}
#endif /* BERRYABSTRACTPREFERENCESSTORAGE_H_ */
