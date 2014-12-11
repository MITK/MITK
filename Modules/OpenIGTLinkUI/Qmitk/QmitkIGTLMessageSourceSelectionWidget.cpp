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

#include "QmitkIGTLMessageSourceSelectionWidget.h"

//mitk headers
#include <usGetModuleContext.h>
#include <usServiceReference.h>



QmitkIGTLMessageSourceSelectionWidget::QmitkIGTLMessageSourceSelectionWidget(
    QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f)
{
  m_Controls = NULL;
  m_CurrentIGTLMessageSource = NULL;
  CreateQtPartControl(this);
}


QmitkIGTLMessageSourceSelectionWidget::~QmitkIGTLMessageSourceSelectionWidget()
{

}

void QmitkIGTLMessageSourceSelectionWidget::CreateQtPartControl(QWidget *parent)
{
  if ( !m_Controls )
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkIGTLMessageSourceSelectionWidgetControls;
    // setup GUI widgets
    m_Controls->setupUi(parent);
  }

  CreateConnections();
}

void QmitkIGTLMessageSourceSelectionWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_ServiceListWidget,
             SIGNAL(ServiceSelectionChanged(us::ServiceReferenceU)),
             this, SLOT(IGTLMessageSourceSelected(us::ServiceReferenceU)) );

    //initialize service list widget
    std::string empty = "";
    m_Controls->m_ServiceListWidget->Initialize<mitk::IGTLMessageSource>(
          mitk::IGTLMessageSource::US_PROPKEY_DEVICENAME, empty);
  }
}

void QmitkIGTLMessageSourceSelectionWidget::IGTLMessageSourceSelected(us::ServiceReferenceU s)
  {
    if (!s) //nothing selected
      {
        //reset everything
        this->m_CurrentIGTLMessageSource = NULL;
        emit IGTLMessageSourceSelected(this->m_CurrentIGTLMessageSource);
        return;
      }

    // Get storage
    us::ModuleContext* context = us::GetModuleContext();
    this->m_CurrentIGTLMessageSource =
        context->GetService<mitk::IGTLMessageSource>(s);
    emit IGTLMessageSourceSelected(this->m_CurrentIGTLMessageSource);
  }

mitk::IGTLMessageSource::Pointer QmitkIGTLMessageSourceSelectionWidget::GetSelectedIGTLMessageSource()
{
  return this->m_CurrentIGTLMessageSource;
}
