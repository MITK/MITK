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

#include "QmitkToolGUIArea.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
QmitkToolGUIArea::QmitkToolGUIArea( QWidget* parent, Qt::WFlags f )
#else
QmitkToolGUIArea::QmitkToolGUIArea( QWidget* parent, Qt::WindowFlags f )
#endif
:QWidget(parent,f)
{
  QWidget::setContentsMargins(0, 0, 0, 0);
}

QmitkToolGUIArea::~QmitkToolGUIArea()
{
}

