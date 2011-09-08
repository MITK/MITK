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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "IGTTrackingLabView.h"
#include "QmitkStdMultiWidget.h"

#include <QmitkNDIConfigurationWidget.h>
#include <QmitkFiducialRegistrationWidget.h>
#include <QmitkUpdateTimerWidget.h>
#include <QmitkToolTrackingStatusWidget.h>


// Qt
#include <QMessageBox>


const std::string IGTTrackingLabView::VIEW_ID = "org.mitk.views.igttrackinglab";

IGTTrackingLabView::IGTTrackingLabView()
: QmitkFunctionality()
{
}

IGTTrackingLabView::~IGTTrackingLabView()
{
}

void IGTTrackingLabView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  
  m_ToolBox = new QToolBox(parent);
  m_Controls.m_VBoxLayout->addWidget(m_ToolBox);


  this->CreateBundleWidgets( parent );


}


void IGTTrackingLabView::CreateBundleWidgets( QWidget* parent )
{
    // configuration widget
  m_NDIConfigWidget = new QmitkNDIConfigurationWidget(parent);
  m_NDIConfigWidget->SetToolTypes(QStringList () << "Instrument" << "Fiducial" << "Skinmarker" << "Unknown" );

  m_ToolBox->addItem(m_NDIConfigWidget, "Configuration");

  // registration widget
  m_RegistrationWidget = new QmitkFiducialRegistrationWidget(parent);
  m_RegistrationWidget->HideStaticRegistrationRadioButton(true);
  m_RegistrationWidget->HideContinousRegistrationRadioButton(true);
  m_RegistrationWidget->HideUseICPRegistrationCheckbox(true);
  m_RegistrationWidget->SetQualityDisplayText("FRE");

  m_ToolBox->addItem(m_RegistrationWidget, "Registration");



  // tracking status
  m_ToolStatusWidget = new QmitkToolTrackingStatusWidget( parent );
 
  m_Controls.m_VBoxLayout->addWidget(m_ToolStatusWidget);

  // update timer
  m_RenderingTimerWidget = new QmitkUpdateTimerWidget( parent );
  m_RenderingTimerWidget->SetPurposeLabelText(QString("Navigation"));
  m_RenderingTimerWidget->SetTimerInterval( 50 );  // set rendering timer at 20Hz (updating every 50msec)

   m_Controls.m_VBoxLayout->addWidget(m_RenderingTimerWidget);

}




//void IGTTrackingLabView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
//{ 
//  // iterate all selected objects, adjust warning visibility
//  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
//    it != nodes.end();
//    ++it )
//  {
//    mitk::DataNode::Pointer node = *it;
//
//    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
//    {
//      m_Controls.labelWarning->setVisible( false );
//      m_Controls.buttonPerformImageProcessing->setEnabled( true );
//      return;
//    }
//  }
//
//  m_Controls.labelWarning->setVisible( true );
//  m_Controls.buttonPerformImageProcessing->setEnabled( false );
//}


