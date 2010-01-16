#ifndef BERRYIBERRYPREFERENCES_H_
#define BERRYIBERRYPREFERENCES_H_

#include "berryRuntimeDll.h"
#include "berryIPreferences.h"
#include "berryMessage.h"

#include <vector>
#include <string>
#include <exception>

namespace berry 
{

  ///
  /// Like IEclipsePreferences an extension to the osgi-IPreferences
  /// to send out events when nodes or values changed in a node.
  ///
  struct BERRY_RUNTIME IBerryPreferences : virtual public IPreferences
  {
    berryInterfaceMacro(IBerryPreferences, berry)

    ///
    /// Invoked when this node was changed, that is when a property
    /// was changed or when a new child node was inserted.
    ///
    berry::Message1<const IBerryPreferences*> OnChanged;    
    
  };

}  // namespace berry

#endif /*BERRYIBERRYPREFERENCES_H_*/
