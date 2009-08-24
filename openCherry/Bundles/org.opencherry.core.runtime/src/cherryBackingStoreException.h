#ifndef CHERRYBACKINGSTOREEXCEPTION_H_
#define CHERRYBACKINGSTOREEXCEPTION_H_

#include "cherryRuntimeDll.h"
#include <Poco/Exception.h>

namespace cherry 
{
  /**
   * Thrown to indicate that a preferences operation could not complete because of
   * a failure in the backing store, or a failure to contact the backing store.
   * 
   * @version $Revision: 1.11 $
   */
  POCO_DECLARE_EXCEPTION(CHERRY_RUNTIME, BackingStoreException, Poco::RuntimeException);
}

#endif /* CHERRYBACKINGSTOREEXCEPTION_H_ */