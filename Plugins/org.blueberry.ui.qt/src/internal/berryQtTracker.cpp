/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryTweaklets.h"

#include "berryQtTracker.h"

#include "berryConstants.h"

#include "berryIDropTarget.h"
#include "berryDragUtil.h"
#include "berryGuiWidgetsTweaklet.h"

#include <QEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QRubberBand>
#include <QPixmap>

namespace berry
{

CursorType QtDragManager::PositionToCursorType(int positionConstant)
{
  if (positionConstant == Constants::LEFT)
    return CURSOR_LEFT;
  if (positionConstant == Constants::RIGHT)
    return CURSOR_RIGHT;
  if (positionConstant == Constants::TOP)
    return CURSOR_TOP;
  if (positionConstant == Constants::BOTTOM)
    return CURSOR_BOTTOM;
  if (positionConstant == Constants::CENTER)
    return CURSOR_CENTER;

  return CURSOR_INVALID;
}

int QtDragManager::CursorTypeToPosition(CursorType dragCursorId)
{
  switch (dragCursorId)
  {
  case CURSOR_LEFT:
    return Constants::LEFT;
  case CURSOR_RIGHT:
    return Constants::RIGHT;
  case CURSOR_TOP:
    return Constants::TOP;
  case CURSOR_BOTTOM:
    return Constants::BOTTOM;
  case CURSOR_CENTER:
    return Constants::CENTER;
  default:
    return Constants::DEFAULT;
  }
}

bool QtDragManager::eventFilter(QObject* o, QEvent* e)
{
  if (beingCancelled)
  {
    if (e->type() == QEvent::KeyRelease && ((QKeyEvent*) e)->key()
        == Qt::Key_Escape)
    {
      QApplication::instance()->removeEventFilter(this);
      beingCancelled = false;
      eventLoop->exit();
      return true; // block the key release
    }
    return false;
  }

  if (!o->isWidgetType())
    return false;

  if (e->type() == QEvent::MouseMove)
  {
    QMouseEvent* me = (QMouseEvent *) e;
    this->Move(me->globalPos());
    return true;
  }
  else if (e->type() == QEvent::MouseButtonRelease)
  {
    //DEBUG("pre drop");
    QApplication::instance()->removeEventFilter(this);
    beingCancelled = false;
    eventLoop->exit();
    return true;
  }

  if (e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease)
  {
    QKeyEvent *ke = ((QKeyEvent*) e);
    if (ke->key() == Qt::Key_Escape && e->type() == QEvent::KeyPress)
    {
      this->Cancel();
      QApplication::instance()->removeEventFilter(this);
      //beingCancelled = false;
      eventLoop->exit();
    }
    else
    {
      // move(QCursor::pos());
    }
    return true; // Eat all key events
  }

  // ### We bind modality to widgets, so we have to do this
  // ###  "manually".
  // DnD is modal - eat all other interactive events
  switch (e->type())
  {
  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonRelease:
  case QEvent::MouseButtonDblClick:
  case QEvent::MouseMove:
  case QEvent::KeyPress:
  case QEvent::KeyRelease:
  case QEvent::Wheel:
  case QEvent::ShortcutOverride:
#ifdef QT3_SUPPORT
    case QEvent::Accel:
    case QEvent::AccelAvailable:
#endif
    return true;
  default:
    return false;
  }
}

void QtDragManager::Cancel()
{
  beingCancelled = true;
}

void QtDragManager::Move(const QPoint& globalPos)
{
  emit tracker->Moved(tracker, globalPos);
}

bool QtDragManager::Drag(QtTracker* tracker)
{

  if (tracker == nullptr)
    return false;

  this->tracker = tracker;
  beingCancelled = false;

  QApplication::instance()->installEventFilter(this);

  //    if (!QWidget::mouseGrabber())
  //      rubberBand->grabMouse();

  eventLoop = new QEventLoop;
  eventLoop->exec();
  delete eventLoop;
  eventLoop = nullptr;

  return !beingCancelled;
}

QtTracker::QtTracker() :
  rubberBand(nullptr), dragManager(nullptr), cursorOverride(0)
{
  rubberBand = new QRubberBand(QRubberBand::Rectangle);
  QPalette rubberPalette(rubberBand->palette());
  //rubberPalette.setColor(QPalette::Button, QColor(Qt::darkRed));
  rubberPalette.setBrush(QPalette::Foreground, QBrush(Qt::darkRed));
  rubberPalette.setBrush(QPalette::Window, QBrush(Qt::darkRed));
  rubberPalette.setBrush(QPalette::Background, QBrush(Qt::darkRed));
  rubberPalette.setBrush(QPalette::Base, QBrush(Qt::darkRed));
  rubberPalette.setBrush(QPalette::Text, QBrush(Qt::darkRed));
  rubberBand->setPalette(rubberPalette);
  rubberBand->ensurePolished();

  QPixmap pixCursorTop(":/org.blueberry.ui.qt/cursor_top.xpm");
  auto  cursorTop = new QCursor(pixCursorTop, 15, 8);
  cursorMap.insert(CURSOR_TOP, cursorTop);

  QPixmap pixCursorRight(":/org.blueberry.ui.qt/cursor_right.xpm");
  auto  cursorRight = new QCursor(pixCursorRight, 23, 15);
  cursorMap.insert(CURSOR_RIGHT, cursorRight);

  QPixmap pixCursorBottom(":/org.blueberry.ui.qt/cursor_bottom.xpm");
  auto  cursorBottom = new QCursor(pixCursorBottom, 16, 23);
  cursorMap.insert(CURSOR_BOTTOM, cursorBottom);

  QPixmap pixCursorLeft(":/org.blueberry.ui.qt/cursor_left.xpm");
  auto  cursorLeft = new QCursor(pixCursorLeft, 8, 15);
  cursorMap.insert(CURSOR_LEFT, cursorLeft);

  QPixmap pixCursorCenter(":/org.blueberry.ui.qt/cursor_center.xpm");
  auto  cursorCenter = new QCursor(pixCursorCenter, 15, 15);
  cursorMap.insert(CURSOR_CENTER, cursorCenter);

  QPixmap pixCursorOffscreen(":/org.blueberry.ui.qt/cursor_offscreen.xpm");
  auto  cursorOffscreen = new QCursor(pixCursorOffscreen, 15, 15);
  cursorMap.insert(CURSOR_OFFSCREEN, cursorOffscreen);

  auto  cursorInvalid = new QCursor(Qt::ForbiddenCursor);
  cursorMap.insert(CURSOR_INVALID, cursorInvalid);
}

QtTracker::~QtTracker()
{
  delete rubberBand;

  for (QHash<CursorType, QCursor*>::iterator iter = cursorMap.begin();
       iter != cursorMap.end(); ++iter)
  {
    delete iter.value();
  }
}

QRect QtTracker::GetRectangle() const
{
  return rubberBand->geometry();
}

void QtTracker::SetRectangle(const QRect& rectangle)
{
  rubberBand->setGeometry(rectangle);
}

void QtTracker::SetCursor(CursorType cursorType)
{
  QCursor* cursor = cursorMap[cursorType];
  if (!cursor) return;

  if (cursorOverride > 0)
  {
    QApplication::changeOverrideCursor(*cursor);
  }
  else
  {
    ++cursorOverride;
    QApplication::setOverrideCursor(*cursor);
  }
}

bool QtTracker::Open()
{
  rubberBand->show();

  dragManager = new QtDragManager();
  bool result = dragManager->Drag(this);
  delete dragManager;

  rubberBand->hide();

  while (cursorOverride > 0)
  {
    QApplication::restoreOverrideCursor();
    --cursorOverride;
  }

  return result;
}

QtTrackerMoveListener::QtTrackerMoveListener(Object::Pointer draggedItem,
                                             const QRect& sourceBounds,
                                             const QPoint& initialLocation,
                                             bool allowSnapping)
  : allowSnapping(allowSnapping)
  , draggedItem(draggedItem)
  , sourceBounds(sourceBounds)
  , initialLocation(initialLocation)
{
}

void QtTrackerMoveListener::Moved(QtTracker* tracker, const QPoint& location)
{
  // Select a drop target; use the global one by default
  IDropTarget::Pointer target;

  QWidget* targetControl = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetCursorControl();

  // Get the drop target for this location
  target = DragUtil::GetDropTarget(targetControl, draggedItem, location,
      tracker->GetRectangle());

  // Set up the tracker feedback based on the target
  QRect snapTarget;
  if (target != 0)
  {
    snapTarget = target->GetSnapRectangle();

    tracker->SetCursor(target->GetCursor());
  }
  else
  {
    tracker->SetCursor(CURSOR_INVALID);
  }

  // If snapping then reset the tracker's rectangle based on the current drop target
  if (allowSnapping)
  {
    if (snapTarget.width() <= 0 || snapTarget.height() <= 0)
    {
      snapTarget = QRect(sourceBounds.x() + location.x() - initialLocation.x(),
          sourceBounds.y() + location.y() - initialLocation.y(), sourceBounds.width(),
          sourceBounds.height());
    }

    // Try to prevent flicker: don't change the rectangles if they're already in
    // the right location
    QRect currentRectangle = tracker->GetRectangle();

    if (!(currentRectangle == snapTarget))
    {
      tracker->SetRectangle(snapTarget);
    }
  }

}

}

