#ifndef BERRYPREFERENCESSERVICE_H_
#define BERRYPREFERENCESSERVICE_H_

#include "berryRuntimeDll.h"
//#include "berryService.h"
#include "berryIBerryPreferencesService.h"
#include "berryAbstractPreferencesStorage.h"

#include <vector>
#include <map>

namespace berry 
{
  /**
  * Implementation of the IPreferencesService Interface
  */
  class BERRY_RUNTIME PreferencesService : public IBerryPreferencesService
  {
  public:
    berryObjectMacro(PreferencesService)

    //# From berry::Service
    virtual bool IsA(const std::type_info& type) const;
    virtual const std::type_info& GetType() const;

    ///
    /// Returns the default name for the preferences data file
    ///
    static std::string GetDefaultPreferencesFileName();

    ///
    /// Returns the path to the directory where all preference data is stored.
    ///
    static std::string GetDefaultPreferencesDirPath();

    ///
    /// Reads in all users for which preferences exist.
    ///
    PreferencesService(std::string _PreferencesDir="");

    ///
    /// Nothing to do here so far.
    ///
    virtual ~PreferencesService();

    /**
    * If no system preference file exists create a new AbstractPreferencesStorage.
    * \see IPreferencesService::GetSystemPreferences()
    */
    virtual IPreferences::Pointer GetSystemPreferences();

    /**
    * If no user preference file exists create a new AbstractPreferencesStorage.
    * \see IPreferencesService::GetUserPreferences()
    */
    virtual IPreferences::Pointer GetUserPreferences(std::string name);

    /**
    * \see IPreferencesService::GetUsers()
    */
    virtual std::vector<std::string> GetUsers() const;
    

    ///
    /// \see IPreferencesService::ImportPreferences()
    ///
    virtual void ImportPreferences(Poco::File f, std::string name="");

    ///
    /// \see IPreferencesService::ExportPreferences()
    ///
    virtual void ExportPreferences(Poco::File f, std::string name="");

    ///
    /// flushes all preferences
    ///
    virtual void ShutDown();
  protected:
    ///
    /// Helper func for ImportPreferences(). Imports all nodes of an IPreferences tree recursively
    ///
    void ImportNode( IPreferences::Pointer nodeToImport , IPreferences::Pointer rootOfOldPrefs );
    ///
    /// Holds the directory where the preferences files will be stored
    ///
    std::string m_PreferencesDir;
    ///
    /// Maps all user names to their preference storage.
    ///
    std::map<std::string, AbstractPreferencesStorage::Pointer> m_PreferencesStorages;
    ///
    /// A mutex to avoid concurrency crashes. Mutable because we need to use Mutex::lock() in const functions
    ///
    mutable Poco::Mutex m_Mutex;
    
  };
}  // namespace berry

#endif /*BERRYPREFERENCESSERVICE_H_*/
