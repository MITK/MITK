#ifndef CHERRYICHERRYPREFERENCES_H_
#define CHERRYICHERRYPREFERENCES_H_

#include "cherryRuntimeDll.h"
#include "cherryIPreferences.h"
#include "cherryMessage.h"

#include <vector>
#include <string>
#include <exception>

namespace cherry 
{

  ///
  /// Like IEclipsePreferences an extension to the osgi-IPreferences
  /// to send out events when nodes or values changed in a node.
  ///
  struct CHERRY_RUNTIME ICherryPreferences : virtual public IPreferences
  {
    cherryInterfaceMacro(ICherryPreferences, cherry)

    ///
    /// Invoked when this node was changed, that is when a property
    /// was changed or when a new child node was inserted.
    ///
    cherry::Message1<const ICherryPreferences*> OnChanged;    
    
  };

}  // namespace cherry

#endif /*CHERRYICHERRYPREFERENCES_H_*/
