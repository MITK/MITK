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
#include "QmitkBasePropertyView.h"

QmitkBasePropertyView::QmitkBasePropertyView( const mitk::BaseProperty* property, QWidget* parent)
: QLabel( parent ),
  PropertyView( property )
{
  PropertyChanged();
}

QmitkBasePropertyView::~QmitkBasePropertyView()
{
}

void QmitkBasePropertyView::PropertyChanged()
{
  if ( m_Property )
    setText( m_Property->GetValueAsString().c_str() );
}

void QmitkBasePropertyView::PropertyRemoved()
{
  m_Property = NULL;
  setText("n/a");
}

