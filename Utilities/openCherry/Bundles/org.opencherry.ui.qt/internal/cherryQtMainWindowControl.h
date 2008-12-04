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


#ifndef CHERRYQTMAINWINDOWCONTROL_H_
#define CHERRYQTMAINWINDOWCONTROL_H_

#include <QMainWindow>

#include "cherryQtAbstractControlWidget.h"

namespace cherry {

class QtMainWindowControl : public QMainWindow, public QtAbstractControlWidget
{

  Q_OBJECT

public:

  QtMainWindowControl(QWidget* parent = 0, Shell* shell = 0, Qt::WindowFlags flags = 0);

  Shell* GetShell();

protected:

  // used for shell listeners
  void changeEvent(QEvent* event);
  void closeEvent(QCloseEvent* closeEvent);

  // used for control listeners
  void moveEvent(QMoveEvent* event);
  void resizeEvent(QResizeEvent* event);
  void inFocusEvent(QFocusEvent* event);

private:

  Shell* shell;
};

}

#endif /* CHERRYQTMAINWINDOWCONTROL_H_ */
