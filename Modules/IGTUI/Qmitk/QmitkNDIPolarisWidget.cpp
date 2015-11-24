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

#include "QmitkNDIPolarisWidget.h"
#include "QmitkTrackingDeviceConfigurationWidget.h"

#include <QFileDialog>
#include <QMessageBox>

const std::string QmitkNDIPolarisWidget::VIEW_ID = "org.mitk.views.NDIPolarisWidget";

QmitkNDIPolarisWidget::QmitkNDIPolarisWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();
  m_ErrorMessage = "";
}

QmitkNDIPolarisWidget::~QmitkNDIPolarisWidget()
{
}

void QmitkNDIPolarisWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkNDIPolarisWidget;
    m_Controls->setupUi(parent);
  }
}

void QmitkNDIPolarisWidget::CreateConnections()
{
  if (m_Controls)
  {
    //connect( (QObject*)(m_Controls->connectButton), SIGNAL(clicked()), this, SLOT(OnConnect()) );
  }
}

void QmitkNDIPolarisWidget::OnConnect()
{
  emit TrackingDeviceConnected();
}