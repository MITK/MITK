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

#include <list>

#include <guitk/cherryGuiTkIControlListener.h>

namespace cherry {

class QtControlWidget : public QFrame
{
  Q_OBJECT

public:

  QtControlWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

  void AddControlListener(GuiTk::IControlListener::Pointer listener);
  void RemoveControlListener(GuiTk::IControlListener::Pointer listener);

protected:

  void moveEvent(QMoveEvent* event);
  void resizeEvent(QResizeEvent* event);
  void showEvent(QShowEvent* event);

private:

  typedef std::list<GuiTk::IControlListener::Pointer> ListenersListType;
  ListenersListType controlListeners;
};

}

#endif /* CHERRYQTCONTROLWIDGET_H_ */
