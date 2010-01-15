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


#ifndef CHERRYQTPERSPECTIVESWITCHER_H_
#define CHERRYQTPERSPECTIVESWITCHER_H_

#include <QToolBar>
#include <QHash>

#include <cherryIPerspectiveListener.h>
#include <cherryIWorkbenchWindow.h>

namespace cherry {

class QtPerspectiveSwitcher : public QToolBar
{
  Q_OBJECT

public:

  QtPerspectiveSwitcher(IWorkbenchWindow::Pointer window);
  ~QtPerspectiveSwitcher();

private:

  IWorkbenchWindow::Pointer window;
  IPerspectiveListener::Pointer perspListener;

  QHash<QString, QAction*> perspIdToActionMap;

  friend struct QtPerspectiveSwitcherListener;
};

}

#endif /* CHERRYQTPERSPECTIVESWITCHER_H_ */
