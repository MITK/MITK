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

#include "QmitkODFDetailsWidget.h"

#include <qwt_scale_engine.h>

QmitkODFDetailsWidget::QmitkODFDetailsWidget( QWidget * parent )
  : QmitkPlotWidget(parent)
{
  QFrame* canvas = qobject_cast<QFrame*>(m_Plot->canvas());
  if (canvas)
  {
    canvas->setLineWidth(0);
    canvas->setContentsMargins(0,0,0,0);
  }

  auto   scale = new QwtLinearScaleEngine();
  m_Plot->setAxisScaleEngine(0, scale);

  m_Plot->setAxisScale   ( 0, -0.5, 0.5 );
}

QmitkODFDetailsWidget::~QmitkODFDetailsWidget()
{
}
