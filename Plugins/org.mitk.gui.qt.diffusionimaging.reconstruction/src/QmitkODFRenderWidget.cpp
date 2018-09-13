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

#include "QmitkODFRenderWidget.h"
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <vtkSmartPointer.h>
#include <mitkPlaneGeometry.h>

QmitkODFRenderWidget::QmitkODFRenderWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  //create Layouts
  QmitkODFRenderWidgetLayout = new QHBoxLayout( this );

  //Set Layout to widget
  this->setLayout(QmitkODFRenderWidgetLayout);

  //Create RenderWindow
  m_RenderWindow = new QmitkRenderWindow(this, "odf render widget");
  m_RenderWindow->setMaximumSize(300,300);
  m_RenderWindow->GetRenderer()->SetMapperID( mitk::BaseRenderer::Standard3D );

  //m_RenderWindow->SetLayoutIndex( 3 );
  QmitkODFRenderWidgetLayout->addWidget( m_RenderWindow );
}


QmitkODFRenderWidget::~QmitkODFRenderWidget()
{
}

