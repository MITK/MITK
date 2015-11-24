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

#include "QmitkVirtualTrackerWidget.h"
#include "QmitkTrackingDeviceConfigurationWidget.h"

#include <QFileDialog>
#include <QMessageBox>

const std::string QmitkVirtualTrackerWidget::VIEW_ID = "org.mitk.views.VirtualTrackerWidget";

QmitkVirtualTrackerWidget::QmitkVirtualTrackerWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();
  m_ErrorMessage = "";
}

QmitkVirtualTrackerWidget::~QmitkVirtualTrackerWidget()
{
}

void QmitkVirtualTrackerWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkVirtualTrackerWidget;
    m_Controls->setupUi(parent);
  }
}

void QmitkVirtualTrackerWidget::CreateConnections()
{
  if (m_Controls)
  {
    //connect( (QObject*)(m_Controls->connectButton), SIGNAL(clicked()), this, SLOT(OnConnect()) );
  }
}

void QmitkVirtualTrackerWidget::OnConnect()
{
  emit TrackingDeviceConnected();
}