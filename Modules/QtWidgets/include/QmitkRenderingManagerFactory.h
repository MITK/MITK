/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderingManagerFactory_h
#define QmitkRenderingManagerFactory_h

#include <MitkQtWidgetsExports.h>

#include "mitkRenderingManagerFactory.h"

/**
 * \ingroup QmitkModule
 * \brief Qt specific implementation of mitk::RenderingManagerFactory.
 *
 * This class create QmitkRenderingManager instances via
 * #CreateRenderingManager.
 *
 * A static instance of QmitkRenderingManagerFactory is created in
 * QmitkRenderWindow, forcing the usage of QmitkRenderingManager for the Qt
 * platform.
 */
class MITKQTWIDGETS_EXPORT QmitkRenderingManagerFactory : public mitk::RenderingManagerFactory
{
public:
  QmitkRenderingManagerFactory();

  ~QmitkRenderingManagerFactory() override;

  mitk::RenderingManager::Pointer CreateRenderingManager() const override;

private:
};

#endif
