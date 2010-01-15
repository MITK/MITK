#include "cherryBackingStoreException.h"

#include <typeinfo>

namespace cherry {

  POCO_IMPLEMENT_EXCEPTION(BackingStoreException, Poco::RuntimeException, "BackingStore Exception")

}