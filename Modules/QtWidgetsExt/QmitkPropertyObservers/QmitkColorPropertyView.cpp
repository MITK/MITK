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
#include "QmitkColorPropertyView.h"
#include <QPixmap>
#define ROUND_P(x) ((int)((x)+0.5))

QmitkColorPropertyView::QmitkColorPropertyView( const mitk::ColorProperty* property, QWidget* parent )
: QLabel( parent ),
  PropertyView( property ),
  m_ColorProperty(property)
{
  setText("  "); // two spaces for some minimun height
  setMinimumSize(15,15);
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
  if ( m_Property )
    DisplayColor();
}

void QmitkColorPropertyView::PropertyRemoved()
{
  m_Property = NULL;
  m_ColorProperty = NULL;
}

void QmitkColorPropertyView::DisplayColor()
{
  const mitk::Color& tmp_col(m_ColorProperty->GetColor());

  QColor color( ROUND_P(tmp_col[0] * 255.0), ROUND_P(tmp_col[1] * 255.0) , ROUND_P(tmp_col[2] * 255.0) );

  m_WidgetPalette.setColor(QPalette::Background, color);
}

