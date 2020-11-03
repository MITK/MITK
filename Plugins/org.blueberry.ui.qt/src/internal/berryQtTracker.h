/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQTTRACKER_H_
#define BERRYQTTRACKER_H_

#include <berryConstants.h>
#include <berryGuiTkIControlListener.h>

#include <QRect>
#include <QPoint>

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

  bool eventFilter(QObject* o, QEvent* e) override;

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
class QtTracker : public QObject
{
  Q_OBJECT

private:

  QRubberBand* rubberBand;
  QtDragManager* dragManager;

  int cursorOverride;

  QHash<CursorType, QCursor*> cursorMap;

public:

  QtTracker();
  ~QtTracker() override;

  QRect GetRectangle() const;
  void SetRectangle(const QRect& rectangle);

  void SetCursor(CursorType cursor);

  bool Open();

  Q_SIGNAL void Moved(QtTracker* tracker, const QPoint& globalPoint);

};

class QtTrackerMoveListener : public QObject
{
  Q_OBJECT

public:
  QtTrackerMoveListener(Object::Pointer draggedItem, const QRect& sourceBounds,
                        const QPoint& initialLocation, bool allowSnapping);

  Q_SLOT void Moved(QtTracker* tracker, const QPoint& globalPoint);

private:

  bool allowSnapping;
  Object::Pointer draggedItem;
  QRect sourceBounds;
  QPoint initialLocation;
};

}

#endif /* BERRYQTTRACKER_H_ */
