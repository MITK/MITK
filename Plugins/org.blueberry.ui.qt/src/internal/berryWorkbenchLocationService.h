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
