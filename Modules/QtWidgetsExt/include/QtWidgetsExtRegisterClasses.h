/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QtWidgetsExtRegisterClasses_h
#define QtWidgetsExtRegisterClasses_h

#include <MitkQtWidgetsExtExports.h>
#include <mitkCommon.h>

/**
 * \brief Register QtWidgetsExt module classes with the MITK framework.
 *
 * Initializes the Qt-specific callback-from-GUI-thread implementation
 * (QmitkCallbackFromGUIThread). This function is idempotent and safe
 * to call multiple times.
 *
 * \sa QmitkCallbackFromGUIThread
 */
MITKQTWIDGETSEXT_EXPORT void QtWidgetsExtRegisterClasses();

#endif
