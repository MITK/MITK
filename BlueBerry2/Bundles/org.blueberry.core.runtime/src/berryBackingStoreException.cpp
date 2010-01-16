#include "berryBackingStoreException.h"

#include <typeinfo>

namespace berry {

  POCO_IMPLEMENT_EXCEPTION(BackingStoreException, Poco::RuntimeException, "BackingStore Exception")

}