/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef BERRYBACKINGSTOREEXCEPTION_H_
#define BERRYBACKINGSTOREEXCEPTION_H_

#include <org_blueberry_core_runtime_Export.h>
#include <Poco/Exception.h>

namespace berry
{
  /**
   * Thrown to indicate that a preferences operation could not complete because of
   * a failure in the backing store, or a failure to contact the backing store.
   *
   * @version $Revision$
   */
  POCO_DECLARE_EXCEPTION(BERRY_RUNTIME, BackingStoreException, Poco::RuntimeException);
}

#endif /* BERRYBACKINGSTOREEXCEPTION_H_ */
