/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkColorPropertyView.h"
#include <QPixmap>
#define ROUND_P(x) ((int)((x) + 0.5))

QmitkColorPropertyView::QmitkColorPropertyView(const mitk::ColorProperty *property, QWidget *parent)
  : QLabel(parent), PropertyView(property), m_ColorProperty(property)
{
  setText("  "); // two spaces for some minimun height
  setMinimumSize(15, 15);
  PropertyChanged();

  m_WidgetPalette = QWidget::palette();
  QWidget::setPalette(m_WidgetPalette);
  QWidget::setAutoFillBackground(true);
}

QmitkColorPropertyView::~QmitkColorPropertyView()
{
}

void QmitkColorPropertyView::PropertyChanged()
{
  if (m_Property)
    DisplayColor();
}

void QmitkColorPropertyView::PropertyRemoved()
{
  m_Property = nullptr;
  m_ColorProperty = nullptr;
}

void QmitkColorPropertyView::DisplayColor()
{
  const mitk::Color &tmp_col(m_ColorProperty->GetColor());

  QColor color(ROUND_P(tmp_col[0] * 255.0), ROUND_P(tmp_col[1] * 255.0), ROUND_P(tmp_col[2] * 255.0));

  m_WidgetPalette.setColor(QPalette::Background, color);
}
