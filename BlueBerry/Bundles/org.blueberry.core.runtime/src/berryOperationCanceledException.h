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
