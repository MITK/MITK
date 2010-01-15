/*=========================================================================
 
 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/


#ifndef CHERRYQTWORKBENCHWINDOW_H_
#define CHERRYQTWORKBENCHWINDOW_H_

#include <cherryWorkbenchWindow.h>

namespace cherry {

class QtWorkbenchWindow : public WorkbenchWindow
{
public:

  QtWorkbenchWindow(int number);

protected:

  void* CreatePageComposite(void* p);

  void CreateDefaultContents(Shell::Pointer shell);

};

}

#endif /* CHERRYQTWORKBENCHWINDOW_H_ */
