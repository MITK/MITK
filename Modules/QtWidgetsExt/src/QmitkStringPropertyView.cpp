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
#include "QmitkStringPropertyView.h"

QmitkStringPropertyView::QmitkStringPropertyView( const mitk::StringProperty* property, QWidget* parent )
: QLabel( parent ),
  PropertyView( property ),
  m_StringProperty(property)
{
  PropertyChanged();
}

QmitkStringPropertyView::~QmitkStringPropertyView()
{
}

void QmitkStringPropertyView::PropertyChanged()
{
  if ( m_Property )
    setText( m_StringProperty->GetValue() );
}

void QmitkStringPropertyView::PropertyRemoved()
{
  m_Property = NULL;
  m_StringProperty = NULL;
  setText("n/a");
}

