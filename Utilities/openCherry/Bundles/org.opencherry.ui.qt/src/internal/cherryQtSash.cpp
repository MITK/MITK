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

#include "cherryQtSash.h"

#include <cherryConstants.h>

#include <QMouseEvent>

namespace cherry
{

/*!
 Creates a QtSash with the given orientation, parent, and opaqueness.
 */
QtSash::QtSash(Qt::Orientation orientation, QWidget *parent, bool opaque) :
  QWidget(parent), opaque(opaque), orientation(orientation), rubberBand(0),
      lastX(0), lastY(0)
{
  if (orientation == Qt::Horizontal)
    this->setCursor(Qt::SplitVCursor);
  else
    this->setCursor(Qt::SplitHCursor);
}

QtSash::~QtSash()
{
  std::cout << "DELETING Sash Widget\n";
}

void QtSash::AddSelectionListener(GuiTk::ISelectionListener::Pointer listener)
{
  selectionEvents.AddListener(listener);
}

void QtSash::RemoveSelectionListener(
    GuiTk::ISelectionListener::Pointer listener)
{
  selectionEvents.AddListener(listener);
}

/*!
 Returns the sash's orientation.
 */
Qt::Orientation QtSash::GetOrientation() const
{
  return orientation;
}

/*!
 Returns true if widgets are resized dynamically (opaquely), otherwise
 returns false.
 */
bool QtSash::OpaqueResize() const
{
  return opaque;
}

/*!
 Tells the splitter to move this handle to position \a pos, which is
 the distance from the left or top edge of the widget.

 Note that \a pos is also measured from the left (or top) for
 right-to-left languages. This function will map \a pos to the
 appropriate position before calling QSplitter::moveSplitter().

 \sa QSplitter::moveSplitter() closestLegalPosition()
 */
//void QtSash::moveSplitter(int pos)
//{
//    Q_D(QSplitterHandle);
//    if (d->s->isRightToLeft() && d->orient == Qt::Horizontal)
//        pos = d->s->contentsRect().width() - pos;
//    d->s->moveSplitter(pos, d->s->indexOf(this));
//}


/*!
 \reimp
 */
//QSize QtSash::sizeHint() const
//{
//    Q_D(const QSplitterHandle);
//    int hw = d->s->handleWidth();
//    QStyleOption opt(0);
//    opt.init(d->s);
//    opt.state = QStyle::State_None;
//    return parentWidget()->style()->sizeFromContents(QStyle::CT_Splitter, &opt, QSize(hw, hw), d->s)
//        .expandedTo(QApplication::globalStrut());
//}

/*!
 \reimp
 */
bool QtSash::event(QEvent *event)
{
  //    switch(event->type()) {
  //    case QEvent::HoverEnter:
  //        d->hover = true;
  //        update();
  //        break;
  //    case QEvent::HoverLeave:
  //        d->hover = false;
  //        update();
  //        break;
  //    default:
  //        break;
  //    }
  return QWidget::event(event);
}

/*!
 \reimp
 */
void QtSash::mouseMoveEvent(QMouseEvent *e)
{
  if (!dragging && !(e->buttons() & Qt::LeftButton))
    return;

  QPoint eventPoint(e->globalX(), e->globalY());
  eventPoint = this->parentWidget()->mapFromGlobal(eventPoint);
  int eventX = eventPoint.x();
  int eventY = eventPoint.y();
//  int eventX = e->globalX();
//  int eventY = e->globalY();

  //int x = OS.GTK_WIDGET_X (handle);
  //int y = OS.GTK_WIDGET_Y (handle);
  int width = this->geometry().width();
  int height = this->geometry().height();
  //int parentBorder = 0;
  //int parentWidth = OS.GTK_WIDGET_WIDTH (parent.handle);
  //int parentHeight = OS.GTK_WIDGET_HEIGHT (parent.handle);

  int newX = lastX;
  int newY = lastY;

  if ((orientation & Qt::Vertical) != 0)
  {
    //newX = std::min(std::max (0, eventX + x - startX - parentBorder), parentWidth - width);
    newX = eventX;
  }
  else
  {
    // newY = Math.min (Math.max (0, eventY + y - startY - parentBorder), parentHeight - height);
    newY = eventY;
  }
  if (newX == lastX && newY == lastY)
    return;
  drawRubberBand(lastX, lastY, width, height);

  GuiTk::SelectionEvent::Pointer event(new GuiTk::SelectionEvent(this));
  event->x = newX;
  event->y = newY;
  event->width = width;
  event->height = height;
  if (!opaque)
  {
    event->detail = Constants::DRAG;
  }
  selectionEvents.selected(event);
  if (event->doit)
  {
    lastX = event->x;
    lastY = event->y;
  }

  //parent.update (true, (style & SWT.SMOOTH) == 0);
  drawRubberBand(lastX, lastY, width, height);
  if (opaque)
  {
    setGeometry(lastX, lastY, width, height);
    // widget could be disposed at this point
  }
}

/*!
 \reimp
 */
void QtSash::mousePressEvent(QMouseEvent *e)
{
  if (e->button() == Qt::LeftButton)
  {
//    const QRect& rect = this->geometry();
//    QPoint p1(this->mapToGlobal(rect.topLeft()));
//    QPoint p2(this->mapToGlobal(rect.bottomRight()));
//    startRect = QRect(p1, p2);

    startRect = this->geometry();

    lastX = startRect.x();
    lastY = startRect.y();
    GuiTk::SelectionEvent::Pointer event(new GuiTk::SelectionEvent(this));
    event->x = lastX;
    event->y = lastY;
    event->width = startRect.width();
    event->height = startRect.height();
    if (!opaque)
    {
      event->detail = Constants::DRAG;
    }
    selectionEvents.selected(event);

    if (event->doit)
    {
      dragging = true;
      lastX = event->x;
      lastY = event->y;
      //parent.update (true, (style & SWT.SMOOTH) == 0);
      drawRubberBand(lastX, lastY, startRect.width(), startRect.height());
      if (opaque)
      {
        this->setGeometry(lastX, lastY, startRect.width(), startRect.height());
        // widget could be disposed at this point
      }
    }
  }

}

/*!
 \reimp
 */
void QtSash::mouseReleaseEvent(QMouseEvent *e)
{
  if (dragging && e->button() == Qt::LeftButton)
  {

    this->drawRubberBand(-1, -1, -1, -1);

    dragging = false;
    const QRect& rect = this->geometry();
    int width = rect.width();
    int height = rect.height();
    GuiTk::SelectionEvent::Pointer event(new GuiTk::SelectionEvent(this));
    event->x = lastX;
    event->y = lastY;
    event->width = width;
    event->height = height;
    //drawBand (lastX, lastY, width, height);
    selectionEvents.selected(event);
    if (event->doit)
    {
      if (opaque)
      {
        this->setGeometry(event->x, event->y, width, height);
        // widget could be disposed at this point
      }
    }
  }
}

void QtSash::drawRubberBand(int x, int y, int width, int height)
{
  if (opaque)
    return;

  if (x < 0 || y < 0)
  {
    if (this->rubberBand)
      this->rubberBand->hide();
    return;
  }

  if (!this->rubberBand)
  {
    this->rubberBand = new QRubberBand(QRubberBand::Line, this->parentWidget());
    // For accessibility to identify this special widget.
    this->rubberBand->setObjectName(QLatin1String("qt_rubberband"));
  }
  this->rubberBand->setGeometry(x, y, width, height);
  if (!this->rubberBand->isVisible())
    this->rubberBand->show();
}

}
