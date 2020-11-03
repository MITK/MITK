/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkIGTLMessageSourceSelectionWidget.h"

//mitk headers
#include <usGetModuleContext.h>
#include <usServiceReference.h>



QmitkIGTLMessageSourceSelectionWidget::QmitkIGTLMessageSourceSelectionWidget(
    QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f)
{
  m_Controls = nullptr;
  m_CurrentIGTLMessageSource = nullptr;
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
        this->m_CurrentIGTLMessageSource = nullptr;
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

mitk::IGTLMessageSource::Pointer QmitkIGTLMessageSourceSelectionWidget::AutoSelectFirstIGTLMessageSource()
{
  if( m_Controls->m_ServiceListWidget->GetAllServiceReferences().size() != 0 )
  {
    us::ModuleContext* context = us::GetModuleContext();
    this->m_CurrentIGTLMessageSource =
      context->GetService<mitk::IGTLMessageSource>(
        m_Controls->m_ServiceListWidget->GetAllServiceReferences().at(0) );
  }
  else
  {
    this->m_CurrentIGTLMessageSource = nullptr;
    MITK_WARN("CurrentIGTLMessageSource") << "There was no OpenIGTLink message source to select."
    << "The OpenIGTLink message source must be selected manually.";
  }

  return this->m_CurrentIGTLMessageSource;
}
