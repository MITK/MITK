/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYSOLSTICEEXCEPTIONS_H_
#define BERRYSOLSTICEEXCEPTIONS_H_

#include <org_blueberry_core_runtime_Export.h>
#include <ctkException.h>

namespace berry {

/**
 * This exception is thrown to blow out of a long-running method
 * when the user cancels it.
 * <p>
 * This class is not intended to be subclassed by clients but
 * may be instantiated.
 * </p>
 */
CTK_DECLARE_EXCEPTION(org_blueberry_core_runtime_EXPORT, OperationCanceledException, ctkRuntimeException)

}

#endif /* BERRYSOLSTICEEXCEPTIONS_H_ */
