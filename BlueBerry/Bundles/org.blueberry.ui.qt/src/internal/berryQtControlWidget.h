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


#ifndef BERRYQTCONTROLWIDGET_H_
#define BERRYQTCONTROLWIDGET_H_

#include <QFrame>

#include "berryQtWidgetController.h"

namespace berry {

class QtControlWidget : public QFrame
{

public:

  QtControlWidget(QWidget* parent, Shell* shell, Qt::WindowFlags f = 0);

  ~QtControlWidget();

  void FireActivateEvent();

protected:

  // used for shell listeners
  void changeEvent(QEvent* event);
  void closeEvent(QCloseEvent* closeEvent);

  // used for control listeners
  void moveEvent(QMoveEvent* event);
  void resizeEvent(QResizeEvent* event);


private:

  QtWidgetController::Pointer controller;
};

}

#endif /* BERRYQTCONTROLWIDGET_H_ */
