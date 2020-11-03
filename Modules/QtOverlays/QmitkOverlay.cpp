/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkOverlay.h"

#include <QBoxLayout>

#include <QGraphicsDropShadowEffect>

QmitkOverlay::QmitkOverlay(const char *id)
  : QObject(), m_Id(id), m_Position(top_Left), m_Layer(-1), m_Widget(nullptr), m_WidgetIsCustom(false)
{
}

QmitkOverlay::~QmitkOverlay()
{
  if (m_Widget && !m_WidgetIsCustom)
  {
    m_Widget->deleteLater();
    m_Widget = nullptr;
  }
}

QmitkOverlay::DisplayPosition QmitkOverlay::GetPosition()
{
  return m_Position;
}

void QmitkOverlay::SetPosition(DisplayPosition pos)
{
  m_Position = pos;
}

unsigned int QmitkOverlay::GetLayer()
{
  return m_Layer;
}

void QmitkOverlay::SetLayer(unsigned int layer)
{
  m_Layer = layer;
}

QWidget *QmitkOverlay::GetWidget()
{
  return m_Widget;
}

void QmitkOverlay::AddDropShadow(QWidget *widget)
{
  if (m_Widget)
  {
    auto effect = new QGraphicsDropShadowEffect(widget);
    effect->setOffset(QPointF(1.0, 1.0));
    effect->setBlurRadius(0);
    effect->setColor(Qt::black);

    widget->setGraphicsEffect(effect);
  }
}
