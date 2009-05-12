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
#include "QmitkBoolPropertyView.h"

QmitkBoolPropertyView::QmitkBoolPropertyView( const mitk::BoolProperty* property, QWidget* parent )
: QCheckBox( parent ),
  PropertyView( property ),
  m_BoolProperty(property)
{
  setEnabled( false );
  PropertyChanged();
}

QmitkBoolPropertyView::~QmitkBoolPropertyView()
{
}

void QmitkBoolPropertyView::PropertyChanged()
{
  if ( m_Property )
    setChecked( m_BoolProperty->GetValue() );
}

void QmitkBoolPropertyView::PropertyRemoved()
{
  m_Property = NULL;
  m_BoolProperty = NULL;
  // display "no certain value"
  setTristate(true);
  setCheckState(Qt::PartiallyChecked);
}

