/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYWORKBENCHLOCATIONSERVICE_H
#define BERRYWORKBENCHLOCATIONSERVICE_H

#include "berryIWorkbenchLocationService.h"

namespace berry {

class WorkbenchLocationService : public IWorkbenchLocationService
{

private:

  IWorkbenchPartSite* const partSite;
  QString serviceScope;
  IWorkbench* const workbench;
  IWorkbenchWindow* const window;
  int level;

public:

  WorkbenchLocationService(const QString& serviceScope, IWorkbench* workbench,
                           IWorkbenchWindow* window, IWorkbenchPartSite* partSite,
                           int level);

  IWorkbenchPartSite* GetPartSite() const override;

  QString GetServiceScope() const override;

  IWorkbench* GetWorkbench() const override;

  IWorkbenchWindow* GetWorkbenchWindow() const override;

  int GetServiceLevel() const override;

};

}

#endif // BERRYWORKBENCHLOCATIONSERVICE_H
