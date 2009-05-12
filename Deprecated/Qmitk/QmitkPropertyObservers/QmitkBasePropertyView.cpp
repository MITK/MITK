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
#include <QmitkBasePropertyView.h>

QmitkBasePropertyView::QmitkBasePropertyView( const mitk::BaseProperty* property, QWidget* parent, const char* name )
: QLabel( parent, name ),
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

