#ifndef CHERRYIPREFERENCESSERVICE_H_
#define CHERRYIPREFERENCESSERVICE_H_

#include "cherryRuntimeDll.h"
#include "cherryService.h"
#include "cherryIPreferences.h"

#include <vector>
#include <string>

namespace cherry 
{

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
  struct CHERRY_RUNTIME IPreferencesService : public Service
  {
    ///
    /// A unique ID for the Service.
    ///
    static const std::string ID;

    cherryInterfaceMacro(IPreferencesService, cherry)      

	  /**
	   * Returns the root system node for the calling bundle.
	   * 
	   * @return The root system node for the calling bundle.
	   */
    virtual IPreferences::Pointer GetSystemPreferences() = 0;

	  /**
	   * Returns the root node for the specified user and the calling bundle.
	   * 
	   * @param name The user for which to return the preference root node. 
	   * @return The root node for the specified user and the calling bundle.
	   */
	  virtual IPreferences::Pointer GetUserPreferences(std::string name) = 0;

	  /**
	   * Returns the names of users for which node trees exist.
	   * 
	   * @return The names of users for which node trees exist.
	   */
    virtual std::vector<std::string> GetUsers() const = 0;
  };
}  // namespace cherry

#endif /*CHERRYIPREFERENCESSERVICE_H_*/
