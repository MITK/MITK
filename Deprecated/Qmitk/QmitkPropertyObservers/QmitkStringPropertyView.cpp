/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include <QmitkStringPropertyView.h>

QmitkStringPropertyView::QmitkStringPropertyView( const mitk::StringProperty* property, QWidget* parent, const char* name )
: QLabel( parent, name ),
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

