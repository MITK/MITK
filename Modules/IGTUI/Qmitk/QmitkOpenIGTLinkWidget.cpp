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

#include "QmitkOpenIGTLinkWidget.h"
#include "QmitkTrackingDeviceConfigurationWidget.h"

#include <QFileDialog>
#include <QMessageBox>

const std::string QmitkOpenIGTLinkWidget::VIEW_ID = "org.mitk.views.OpenIGTLinkWidget";

QmitkOpenIGTLinkWidget::QmitkOpenIGTLinkWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();
  m_ErrorMessage = "";
}

QmitkOpenIGTLinkWidget::~QmitkOpenIGTLinkWidget()
{
}

void QmitkOpenIGTLinkWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkOpenIGTLinkWidget;
    m_Controls->setupUi(parent);
  }
}

void QmitkOpenIGTLinkWidget::CreateConnections()
{
  if (m_Controls)
  {
    //connect( (QObject*)(m_Controls->connectButton), SIGNAL(clicked()), this, SLOT(OnConnect()) );
  }
}

void QmitkOpenIGTLinkWidget::OnConnect()
{
  emit TrackingDeviceConnected();
}