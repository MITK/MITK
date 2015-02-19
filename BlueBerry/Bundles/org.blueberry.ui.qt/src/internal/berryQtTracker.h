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


#ifndef BERRYQTTRACKER_H_
#define BERRYQTTRACKER_H_

#include <berryConstants.h>
#include <berryGuiTkIControlListener.h>

class QCursor;
class QEventLoop;
class QRubberBand;

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

  static CursorType PositionToCursorType(int positionConstant);

  /**
   * Converts a DnDTweaklet::CursorType (CURSOR_LEFT, CURSOR_RIGHT, CURSOR_TOP, CURSOR_BOTTOM, CURSOR_CENTER) into a BlueBerry constant
   * (Constants::LEFT, Constants::RIGHT, Constants::TOP, Constants::BOTTOM, Constants::CENTER)
   *
   * @param dragCursorId
   * @return a BlueBerry Constants::* constant
   */
  static int CursorTypeToPosition(CursorType dragCursorId);

  bool Drag(QtTracker* tracker);

};

/**
 *  Instances of this class implement a rubber banding rectangle that is
 *  drawn onto a parent control or display.
 *  These rectangles can be specified to respond to mouse and key events
 *  by either moving or resizing themselves accordingly.  Trackers are
 *  typically used to represent window geometries in a lightweight manner.
 *
 */
class QtTracker {

private:

  QRubberBand* rubberBand;
  QtDragManager* dragManager;

  int cursorOverride;

  GuiTk::IControlListener::Events controlEvents;

  QHash<CursorType, QCursor*> cursorMap;

public:

  QtTracker();
  ~QtTracker();

  QRect GetRectangle() const;
  void SetRectangle(const QRect& rectangle);

  void SetCursor(CursorType cursor);

  bool Open();

  void AddControlListener(GuiTk::IControlListener::Pointer listener);
  void RemoveControlListener(GuiTk::IControlListener::Pointer listener);

  void HandleMove(const QPoint& globalPoint);

};

}

#endif /* BERRYQTTRACKER_H_ */
