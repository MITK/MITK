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

//#define _USE_MATH_DEFINES
#include <QmitkFileDialog.h>

// STL Headers
#include <list>

//microservices
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>

#include <mitkCommon.h>

const std::string QmitkFileDialog::VIEW_ID = "org.mitk.views.QmitkFileDialog";

QmitkFileDialog::QmitkFileDialog(QWidget* parent, Qt::WindowFlags f): QFileDialog(parent, f)
{
  //m_controls = null;
  //CreateQtPartControl(this);
}

QmitkFileDialog::~QmitkFileDialog()
{
}

//////////////////// INITIALIZATION /////////////////////

//void QmitkFileDialog::CreateQtPartControl(QWidget *parent)
//{
//  if (!m_Controls)
//  {
//    // create GUI widgets
//    m_Controls = new Ui::QmitkFileDialogControls;
//    m_Controls->setupUi(parent);
//    this->CreateConnections();
//  }
//  m_Context = us::GetModuleContext();
//}

void QmitkFileDialog::CreateConnections()
{
  //connect( m_Controls->m_ServiceList, SIGNAL(currentItemChanged( QListWidgetItem *, QListWidgetItem *)), this, SLOT(OnServiceSelectionChanged()) );
}
