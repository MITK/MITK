/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef BERRYPREFERENCESSERVICE_H_
#define BERRYPREFERENCESSERVICE_H_

#include <org_blueberry_core_runtime_Export.h>

#include "berryIBerryPreferencesService.h"

#include <QObject>
#include <QHash>
#include <QMutex>

namespace berry
{

class AbstractPreferencesStorage;

  /**
  * Implementation of the IPreferencesService Interface
  */
  class org_blueberry_core_runtime_EXPORT PreferencesService : public QObject, public IBerryPreferencesService
  {
    Q_OBJECT
    Q_INTERFACES(berry::IBerryPreferencesService berry::IPreferencesService)

  public:

    ///
    /// Returns the default name for the preferences data file
    ///
    static QString GetDefaultPreferencesFileName();

    ///
    /// Returns the path to the directory where all preference data is stored.
    ///
    static QString GetDefaultPreferencesDirPath();

    ///
    /// Reads in all users for which preferences exist.
    ///
    PreferencesService(const QString& _PreferencesDir="");

    ///
    /// Nothing to do here so far.
    ///
    ~PreferencesService() override;

    /**
    * If no system preference file exists create a new AbstractPreferencesStorage.
    * \see IPreferencesService::GetSystemPreferences()
    */
    SmartPointer<IPreferences> GetSystemPreferences() override;

    /**
    * If no user preference file exists create a new AbstractPreferencesStorage.
    * \see IPreferencesService::GetUserPreferences()
    */
    SmartPointer<IPreferences> GetUserPreferences(const QString& name) override;

    /**
    * \see IPreferencesService::GetUsers()
    */
    QStringList GetUsers() const override;


    ///
    /// \see IPreferencesService::ImportPreferences()
    ///
    void ImportPreferences(const QString &f, const QString& name="") override;

    ///
    /// \see IPreferencesService::ExportPreferences()
    ///
    void ExportPreferences(const QString &f, const QString& name="") override;

    ///
    /// flushes all preferences
    ///
    virtual void ShutDown();

  protected:

    SmartPointer<IPreferences> GetUserPreferences_unlocked(const QString& name);

    ///
    /// Helper func for ImportPreferences(). Imports all nodes of an IPreferences tree recursively
    ///
    void ImportNode(const SmartPointer<IPreferences>& nodeToImport, const SmartPointer<IPreferences>& rootOfOldPrefs);
    ///
    /// Holds the directory where the preferences files will be stored
    ///
    QString m_PreferencesDir;
    ///
    /// Maps all user names to their preference storage.
    ///
    QHash<QString, SmartPointer<AbstractPreferencesStorage> > m_PreferencesStorages;
    ///
    /// A mutex to avoid concurrency crashes. Mutable because we need to use Mutex::lock() in const functions
    ///
    mutable QMutex m_Mutex;

  };
}  // namespace berry

#endif /*BERRYPREFERENCESSERVICE_H_*/
