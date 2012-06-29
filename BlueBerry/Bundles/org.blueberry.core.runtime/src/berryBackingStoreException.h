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
#include <ctkException.h>

namespace berry
{
  /**
   * Thrown to indicate that a preferences operation could not complete because of
   * a failure in the backing store, or a failure to contact the backing store.
   *
   * @version $Revision$
   */
  CTK_DECLARE_EXCEPTION(org_blueberry_core_runtime_EXPORT, BackingStoreException, ctkException)
}

#endif /* BERRYBACKINGSTOREEXCEPTION_H_ */
