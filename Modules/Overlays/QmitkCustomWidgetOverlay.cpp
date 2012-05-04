/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-01-14 14:20:26 +0100 (Thu, 14 Jan 2010) $
Version:   $Revision: 21047 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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



