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


#include "QmitkCustomWidgetOverlay.h"


QmitkCustomWidgetOverlay::QmitkCustomWidgetOverlay( const char* id )
: QmitkOverlay(id)
{
  QmitkOverlay::AddDropShadow( m_Widget );
}

QmitkCustomWidgetOverlay::~QmitkCustomWidgetOverlay()
{
}

void QmitkCustomWidgetOverlay::SetWidget( QWidget* widget )
{
  if ( widget != NULL )
  {
    m_Widget = widget;
    m_WidgetIsCustom = true;
  }
}


QSize QmitkCustomWidgetOverlay::GetNeededSize()
{
  return m_Widget->size();
}
