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


#ifndef CHERRYQTCONTROLWIDGET_H_
#define CHERRYQTCONTROLWIDGET_H_

#include <QFrame>

#include "cherryQtAbstractControlWidget.h"

namespace cherry {

class QtControlWidget : public QFrame, public QtAbstractControlWidget
{
  Q_OBJECT

public:

  QtControlWidget(QWidget* parent = 0, Shell* shell = 0, Qt::WindowFlags f = 0);

  ~QtControlWidget();

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

#endif /* CHERRYQTCONTROLWIDGET_H_ */
