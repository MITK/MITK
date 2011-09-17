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


#ifndef BERRYQTTRACKER_H_
#define BERRYQTTRACKER_H_

#include <berryITracker.h>
#include <berryDnDTweaklet.h>
#include <berryGuiTkIControlListener.h>

#include <QDrag>
#include <QRubberBand>
#include <QEventLoop>
#include <QCursor>

#include <map>

namespace berry {

class QtTracker;

class QtDragManager : public QObject {

  Q_OBJECT

private:

  QtTracker* tracker;

  QEventLoop* eventLoop;

  bool beingCancelled;

protected:

  bool eventFilter(QObject* o, QEvent* e);

  void Cancel();

  void Move(const QPoint& globalPos);

public:

  bool Drag(QtTracker* tracker);

};


class QtTracker : public ITracker {

private:

  QRubberBand* rubberBand;
  QtDragManager* dragManager;

  int cursorOverride;

  GuiTk::IControlListener::Events controlEvents;

  std::map<DnDTweaklet::CursorType, QCursor*> cursorMap;

public:

  QtTracker();
  ~QtTracker();

  Rectangle GetRectangle();
  void SetRectangle(const Rectangle& rectangle);

  void SetCursor(DnDTweaklet::CursorType cursor);

  bool Open();

  void AddControlListener(GuiTk::IControlListener::Pointer listener);
  void RemoveControlListener(GuiTk::IControlListener::Pointer listener);

  void HandleMove(const QPoint& globalPoint);

};

}

#endif /* BERRYQTTRACKER_H_ */
