/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
