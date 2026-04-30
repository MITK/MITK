/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRegisterClasses_h
#define QmitkRegisterClasses_h

#include <MitkQtWidgetsExports.h>

/**
 * \ingroup QmitkModule
 * \brief Registers MITK Qt widget related classes (e.g., rendering factory, application cursor).
 *
 * This function must be called once before using MITK rendering in a Qt application.
 * It registers the QmitkRenderingManagerFactory and the QmitkApplicationCursor
 * as global instances.
 *
 * \sa QmitkRenderingManagerFactory
 * \sa QmitkApplicationCursor
 */
MITKQTWIDGETS_EXPORT void QmitkRegisterClasses();

#endif
