#ifndef CHERRYICHERRYPREFERENCESSERVICE_H_
#define CHERRYICHERRYPREFERENCESSERVICE_H_

#include "cherryIPreferencesService.h"

#include "Poco/File.h"

#include <vector>
#include <string>

namespace cherry 
{
  struct ICherryPreferencesService : public IPreferencesService
  {
    cherryInterfaceMacro(ICherryPreferencesService, cherry)      

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
}  // namespace cherry

#endif /*CHERRYICHERRYPREFERENCESSERVICE_H_*/
