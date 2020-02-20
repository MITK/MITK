/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCustomWidgetOverlay.h"

QmitkCustomWidgetOverlay::QmitkCustomWidgetOverlay(const char *id) : QmitkOverlay(id)
{
  QmitkOverlay::AddDropShadow(m_Widget);
}

QmitkCustomWidgetOverlay::~QmitkCustomWidgetOverlay()
{
}

void QmitkCustomWidgetOverlay::SetWidget(QWidget *widget)
{
  if (widget != nullptr)
  {
    m_Widget = widget;
    m_WidgetIsCustom = true;
  }
}

QSize QmitkCustomWidgetOverlay::GetNeededSize()
{
  return m_Widget->size();
}
