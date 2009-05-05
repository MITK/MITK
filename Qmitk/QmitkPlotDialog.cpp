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

#include <qlayout.h>
#include <qpushbutton.h>

#include "QmitkPlotDialog.h"

QmitkPlotDialog::QmitkPlotDialog(const char* title, QWidget* parent, const char* name): QDialog(parent, name) 
{
  QVBoxLayout* boxLayout = new QVBoxLayout(this);
  m_Plot = new QmitkPlotWidget( this, title ) ;
  m_CloseDialogButton = new QPushButton("close plot window", this);  
  boxLayout->addWidget( m_Plot );
  boxLayout->addWidget( m_CloseDialogButton );
  connect( m_CloseDialogButton, SIGNAL( clicked( ) ), this, SLOT( accept() ) );
}

QmitkPlotDialog::~QmitkPlotDialog()
{
  delete m_Plot;
}

QmitkPlotWidget* QmitkPlotDialog::GetPlot()
{
  return m_Plot;
}

QwtPlot* QmitkPlotDialog::GetQwtPlot()
{
  return m_Plot->GetPlot();
}
