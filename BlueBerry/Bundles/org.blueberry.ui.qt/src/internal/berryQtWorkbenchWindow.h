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


#ifndef BERRYQTWORKBENCHWINDOW_H_
#define BERRYQTWORKBENCHWINDOW_H_

#include <internal/berryWorkbenchWindow.h>

namespace berry {

class QtWorkbenchWindow : public WorkbenchWindow
{
public:

  QtWorkbenchWindow(int number);

protected:

  void* CreatePageComposite(void* p);

  void CreateDefaultContents(Shell::Pointer shell);

};

}

#endif /* BERRYQTWORKBENCHWINDOW_H_ */
