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

#ifndef QmitkResidualViewWidget_H_
#define QmitkResidualViewWidget_H_

//#include <QObject>
#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QGraphicsPixmapItem>

#include <org_mitk_gui_qt_diffusionimaging_Export.h>


#include "QmitkExtExports.h"


/**
 * \brief Widget for displaying the residual between an original dwi image and the dwi estimated from a tensor image
 *
 */
class DIFFUSIONIMAGING_EXPORT QmitkResidualViewWidget : public QGraphicsView
{

Q_OBJECT

public:


  QmitkResidualViewWidget(QWidget *parent = 0);

  QmitkResidualViewWidget(QGraphicsScene *scene, QWidget *parent = 0);


 // ~QmitkResidualViewWidget();


  void SetResidualPixmapItem(QGraphicsPixmapItem* item)
  {
    m_ResidualPixmapItem = item;
  }

signals:

  void pointSelected(int slice, int volume);

protected:

  void wheelEvent(QWheelEvent *event);

  void mouseMoveEvent(QMouseEvent* event);

  void mousePressEvent(QMouseEvent* event);

  void mouseReleaseEvent(QMouseEvent* event);

  QPointF m_CurrentCenterPoint;

  QGraphicsPixmapItem* m_ResidualPixmapItem;

  QPoint m_LastPanPoint;

  void SetCenter(const QPointF& centerPoint);



};

#endif
