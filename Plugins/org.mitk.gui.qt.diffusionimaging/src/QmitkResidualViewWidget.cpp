/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "QmitkResidualViewWidget.h"
#include <iostream>
#include <QToolTip>


QmitkResidualViewWidget::QmitkResidualViewWidget( QWidget * parent )
  : QGraphicsView(parent)
{


}


QmitkResidualViewWidget::QmitkResidualViewWidget(QGraphicsScene *scene, QWidget *parent)
  : QGraphicsView(scene, parent)
{

}


void QmitkResidualViewWidget::mousePressEvent(QMouseEvent* event) {
  // Panning
  m_LastPanPoint = event->pos();
  setCursor(Qt::ClosedHandCursor);



  QGraphicsItem *item = this->itemAt(event->pos());
  if(item == m_ResidualPixmapItem)
  {
    QPointF sceneCoord(mapToScene(event->pos()));
    QPointF imageCoord(item->mapFromParent(sceneCoord));

    int volume = (int)imageCoord.y();
    int slice = (int)imageCoord.x();

    emit pointSelected(slice, volume);

  }
}


void QmitkResidualViewWidget::mouseReleaseEvent(QMouseEvent* event) {
  setCursor(Qt::OpenHandCursor);
  m_LastPanPoint = QPoint();
}

void QmitkResidualViewWidget::mouseMoveEvent(QMouseEvent *event)
{
  if(!m_LastPanPoint.isNull()) {
    QPointF delta = mapToScene(m_LastPanPoint) - mapToScene(event->pos());
    m_LastPanPoint = event->pos();
    SetCenter(m_CurrentCenterPoint + delta);
  }
}

void QmitkResidualViewWidget::wheelEvent(QWheelEvent *event)
{

  // Position of the mouse in scene coordinates
  QPointF before(mapToScene(event->pos()));

  QPointF screenCenter = m_CurrentCenterPoint;

  double factor = 1.15;
  if(event->delta() > 0)
  {
    scale(factor, factor);
  }
  else
  {
    scale(1.0 / factor, 1.0 / factor);
  }

  //Get the position after scaling, in scene coords
  QPointF after(mapToScene(event->pos()));

  //Get the offset of how the screen moved
  QPointF offset = before - after;

  //Adjust to the new center for correct zooming
  QPointF newCenter = screenCenter + offset;
  SetCenter(newCenter);

}


/**
  * Sets the current centerpoint.  Also updates the scene's center point.
  * Unlike centerOn, which has no way of getting the floating point center
  * back, SetCenter() stores the center point.  It also handles the special
  * sidebar case.  This function will claim the centerPoint to sceneRec ie.
  * the centerPoint must be within the sceneRec.
  */

void QmitkResidualViewWidget::SetCenter(const QPointF& center) {

  QRectF visibleArea = mapToScene(rect()).boundingRect();
  QRectF sceneBounds = sceneRect();

  double boundX = visibleArea.width() / 2.0 ;
  double boundY = visibleArea.height() / 2.0;
  double boundWidth = sceneBounds.width() -2.0 * boundX;
  double boundHeight = sceneBounds.height() - 2.0 * boundY;

  //The max boundary that the centerPoint can be to
  QRectF bounds(boundX, boundY, boundWidth, boundHeight);

  if(bounds.contains(center))
  {
    m_CurrentCenterPoint = center;
  }
  else
  {
    //We need to clamp or use the center of the screen
    if(visibleArea.contains(sceneBounds))
    {
      //Use the center of scene ie. we can see the whole scene
      m_CurrentCenterPoint = sceneBounds.center();
    }
    else{

      m_CurrentCenterPoint = center;

      //We need to clamp the center. The centerPoint is too large
      if(center.x() > bounds.x() + bounds.width())
      {
        m_CurrentCenterPoint.setX(bounds.x() + bounds.width());
      }
      else if(center.x() < bounds.x()) {
        m_CurrentCenterPoint.setX(bounds.x());
      }

      if(center.y() > bounds.y() + bounds.height())
      {
        m_CurrentCenterPoint.setY(bounds.y() + bounds.height());
      }
      else if(center.y() < bounds.y())
      {
        m_CurrentCenterPoint.setY(bounds.y());
      }

    }
  }

  // Update the scrollbars
  centerOn(m_CurrentCenterPoint);
}






