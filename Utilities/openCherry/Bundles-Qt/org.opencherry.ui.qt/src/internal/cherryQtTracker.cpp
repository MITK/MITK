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

#include "cherryQtTracker.h"

#include <QEvent>
#include <QKeyEvent>
#include <QApplication>

namespace cherry
{

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
  tracker->HandleMove(globalPos);
}

bool QtDragManager::Drag(QtTracker* tracker)
{

  if (tracker == 0)
    return false;

  this->tracker = tracker;
  beingCancelled = false;

  QApplication::instance()->installEventFilter(this);

  //    if (!QWidget::mouseGrabber())
  //      rubberBand->grabMouse();

  eventLoop = new QEventLoop;
  eventLoop->exec();
  delete eventLoop;
  eventLoop = 0;

  return !beingCancelled;
}

QtTracker::QtTracker() :
  rubberBand(0), dragManager(0), cursorOverride(0)
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

  QPixmap pixCursorTop(":/resources/cursor_top.xpm");
  QCursor* cursorTop = new QCursor(pixCursorTop, 15, 8);
  cursorMap.insert(std::make_pair(DnDTweaklet::CURSOR_TOP, cursorTop));

  QPixmap pixCursorRight(":/resources/cursor_right.xpm");
  QCursor* cursorRight = new QCursor(pixCursorRight, 23, 15);
  cursorMap.insert(std::make_pair(DnDTweaklet::CURSOR_RIGHT, cursorRight));

  QPixmap pixCursorBottom(":/resources/cursor_bottom.xpm");
  QCursor* cursorBottom = new QCursor(pixCursorBottom, 16, 23);
  cursorMap.insert(std::make_pair(DnDTweaklet::CURSOR_BOTTOM, cursorBottom));

  QPixmap pixCursorLeft(":/resources/cursor_left.xpm");
  QCursor* cursorLeft = new QCursor(pixCursorLeft, 8, 15);
  cursorMap.insert(std::make_pair(DnDTweaklet::CURSOR_LEFT, cursorLeft));

  QPixmap pixCursorCenter(":/resources/cursor_center.xpm");
  QCursor* cursorCenter = new QCursor(pixCursorCenter, 15, 15);
  cursorMap.insert(std::make_pair(DnDTweaklet::CURSOR_CENTER, cursorCenter));

  QPixmap pixCursorOffscreen(":/resources/cursor_offscreen.xpm");
  QCursor* cursorOffscreen = new QCursor(pixCursorOffscreen, 15, 15);
  cursorMap.insert(std::make_pair(DnDTweaklet::CURSOR_OFFSCREEN, cursorOffscreen));

  QCursor* cursorInvalid = new QCursor(Qt::ForbiddenCursor);
  cursorMap.insert(std::make_pair(DnDTweaklet::CURSOR_INVALID, cursorInvalid));
}

QtTracker::~QtTracker()
{
  delete rubberBand;

  for (std::map<DnDTweaklet::CursorType, QCursor*>::iterator iter = cursorMap.begin();
       iter != cursorMap.end(); ++iter)
  {
    delete iter->second;
  }
}

Rectangle QtTracker::GetRectangle()
{
  const QRect& rect = rubberBand->geometry();
  return Rectangle(rect.x(), rect.y(), rect.width(), rect.height());
}

void QtTracker::SetRectangle(const Rectangle& rectangle)
{
  rubberBand->setGeometry(rectangle.x, rectangle.y, rectangle.width, rectangle.height);
}

void QtTracker::SetCursor(DnDTweaklet::CursorType cursorType)
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

void QtTracker::AddControlListener(GuiTk::IControlListener::Pointer listener)
{
  controlEvents.AddListener(listener);
}

void QtTracker::RemoveControlListener(GuiTk::IControlListener::Pointer listener)
{
  controlEvents.RemoveListener(listener);
}

void QtTracker::HandleMove(const QPoint& globalPoint)
{
  GuiTk::ControlEvent::Pointer event(
          new GuiTk::ControlEvent(this, globalPoint.x(), globalPoint.y(), 0, 0));

  controlEvents.movedEvent(event);
}

}

