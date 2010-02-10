/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYQTMAINWINDOWCONTROL_H_
#define BERRYQTMAINWINDOWCONTROL_H_

#include <QMainWindow>

#include "berryQtWidgetController.h"

namespace berry {

class QtMainWindowControl : public QMainWindow
{

  Q_OBJECT

public:

  QtMainWindowControl(Shell* shell, QWidget* parent = 0, Qt::WindowFlags flags = 0);

protected:

  // used for shell listeners
  void changeEvent(QEvent* event);
  void closeEvent(QCloseEvent* closeEvent);

  // used for control listeners
  void moveEvent(QMoveEvent* event);
  void resizeEvent(QResizeEvent* event);
  void inFocusEvent(QFocusEvent* event);

private:

  QtWidgetController::Pointer controller;

};

}

#endif /* BERRYQTMAINWINDOWCONTROL_H_ */
