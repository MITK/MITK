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

#include "QmitkIGTLDeviceSourceSelectionWidget.h"

//mitk headers
#include <usGetModuleContext.h>
#include <usServiceReference.h>



QmitkIGTLDeviceSourceSelectionWidget::QmitkIGTLDeviceSourceSelectionWidget(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
}


QmitkIGTLDeviceSourceSelectionWidget::~QmitkIGTLDeviceSourceSelectionWidget()
{

}

void QmitkIGTLDeviceSourceSelectionWidget::CreateQtPartControl(QWidget *parent)
{
  if ( !m_Controls )
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkIGTLDeviceSourceSelectionWidgetControls;
    // setup GUI widgets
    m_Controls->setupUi(parent);
  }

  CreateConnections();
}

void QmitkIGTLDeviceSourceSelectionWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_ServiceListWidget,
             SIGNAL(ServiceSelectionChanged(us::ServiceReferenceU)),
             this, SLOT(IGTLDeviceSourceSelected(us::ServiceReferenceU)) );

    //initialize service list widget
    std::string empty = "";
    m_Controls->m_ServiceListWidget->Initialize<mitk::IGTLDeviceSource>(
          mitk::IGTLDeviceSource::US_PROPKEY_IGTLDEVICENAME,empty);
  }
}

void QmitkIGTLDeviceSourceSelectionWidget::IGTLDeviceSourceSelected(us::ServiceReferenceU s)
  {
    if (!s) //nothing selected
      {
        //reset everything
        this->m_CurrentIGTLDeviceSource = NULL;
        emit IGTLDeviceSourceSelected(this->m_CurrentIGTLDeviceSource);
        return;
      }

    // Get storage
    us::ModuleContext* context = us::GetModuleContext();
    this->m_CurrentIGTLDeviceSource =
        context->GetService<mitk::IGTLDeviceSource>(s);
    emit IGTLDeviceSourceSelected(this->m_CurrentIGTLDeviceSource);
  }

mitk::IGTLDeviceSource::Pointer QmitkIGTLDeviceSourceSelectionWidget::GetSelectedIGTLDeviceSource()
  {
  return this->m_CurrentIGTLDeviceSource;
  }
