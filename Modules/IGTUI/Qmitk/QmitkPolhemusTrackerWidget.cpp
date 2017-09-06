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

#include "QmitkPolhemusTrackerWidget.h"

#include <QFileDialog>
#include <QScrollBar>

#include <itksys/SystemTools.hxx>
#include <Poco/Path.h>
#include <QSettings>

const std::string QmitkPolhemusTrackerWidget::VIEW_ID = "org.mitk.views.PolhemusTrackerWidget";

QmitkPolhemusTrackerWidget::QmitkPolhemusTrackerWidget(QWidget* parent, Qt::WindowFlags f)
  : QmitkAbstractTrackingDeviceWidget(parent, f)
  , m_Controls(nullptr)
{
}
void QmitkPolhemusTrackerWidget::Initialize()
{
  InitializeSuperclassWidget();
  CreateQtPartControl(this);
}

QmitkPolhemusTrackerWidget::~QmitkPolhemusTrackerWidget()
{
  delete m_Controls;
}

void QmitkPolhemusTrackerWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkPolhemusTrackerWidget;
    m_Controls->setupUi(parent);
  }
}

void QmitkPolhemusTrackerWidget::CreateConnections()
{
  if (m_Controls)
  {
    connect((QObject*)(m_Controls->m_hemisphereTracking), SIGNAL(clicked()), this, SLOT(on_m_hemisphereTracking_clicked()));
  }
}


mitk::TrackingDevice::Pointer QmitkPolhemusTrackerWidget::ConstructTrackingDevice()
{
  if (m_TrackingDevice.IsNull())
  {
    m_TrackingDevice = mitk::PolhemusTrackingDevice::New();
    m_TrackingDevice->SetHemisphereTrackingEnabled(m_Controls->m_hemisphereTracking->isChecked());
  }

  return static_cast<mitk::TrackingDevice::Pointer>(m_TrackingDevice);
}



QmitkPolhemusTrackerWidget* QmitkPolhemusTrackerWidget::Clone(QWidget* parent) const
{
  QmitkPolhemusTrackerWidget* clonedWidget = new QmitkPolhemusTrackerWidget(parent);
  clonedWidget->Initialize();
  return clonedWidget;
}

void QmitkPolhemusTrackerWidget::on_m_hemisphereTracking_clicked()
{
  m_TrackingDevice->SetHemisphereTrackingEnabled(m_Controls->m_hemisphereTracking->isChecked());
}
