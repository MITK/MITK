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

#include "QmitkFreeSurferParcellationWidget.h"

QmitkFreeSurferParcellationWidget::QmitkFreeSurferParcellationWidget(QWidget *parent) :
  QWidget( parent ),
  m_Handler( new QmitkFreeSurferParcellationHandler() )
{
  this->m_View.setupUi( this );
  this->connect( this->m_Handler, SIGNAL(clicked(QString)), this, SLOT(OnHandlerClicked(QString)) );
  this->connect( this->m_Handler, SIGNAL(changed(QString)), this, SLOT(OnHandlerHovered(QString)) );
}

QmitkFreeSurferParcellationWidget::~QmitkFreeSurferParcellationWidget()
{
  if( m_Handler )
  {
    this->disconnect( this->m_Handler, SIGNAL(clicked(QString)), this, SLOT(OnHandlerClicked(QString)) );
    this->disconnect( this->m_Handler, SIGNAL(changed(QString)), this, SLOT(OnHandlerHovered(QString)) );
    delete m_Handler;
    m_Handler = nullptr;
  }
}

void QmitkFreeSurferParcellationWidget::OnHandlerClicked(const QString &name)
{
  this->m_View.clickedLabel->setText( name );
  emit this->Clicked( name );
}

void QmitkFreeSurferParcellationWidget::OnHandlerHovered(const QString &name)
{
  this->m_View.hoveredLabel->setText( name + " hovered" );
  emit this->Hovered( name );
}
