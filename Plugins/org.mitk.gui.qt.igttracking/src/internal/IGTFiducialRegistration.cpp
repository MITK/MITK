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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

//Mitk
#include <mitkDataNode.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateDataType.h>
#include <mitkIRenderingManager.h>
#include <mitkImageGenerator.h>

// Qmitk
#include "IGTFiducialRegistration.h"

// Qt
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

// MicroServices
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include "usServiceReference.h"

const std::string IGTFiducialRegistration::VIEW_ID = "org.mitk.views.IGTFiducialRegistration";

void IGTFiducialRegistration::SetFocus()
{
}

void IGTFiducialRegistration::CreateQtPartControl( QWidget *parent )
{

  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  connect(m_Controls.m_TrackingDeviceSelectionWidget, SIGNAL(NavigationDataSourceSelected(mitk::NavigationDataSource::Pointer)), this, SLOT(PointerSelectionChanged()));

  m_Controls.m_DataStorageComboBox->SetDataStorage(this->GetDataStorage());
  m_Controls.m_DataStorageComboBox->SetAutoSelectNewItems(false);
  m_Controls.m_DataStorageComboBox->SetPredicate(mitk::NodePredicateOr::New(mitk::NodePredicateDataType::New("Surface"), mitk::NodePredicateDataType::New("Image")));
  //Initialize Fiducial Registration Widget
  //foreach(QmitkRenderWindow* renderWindow, this->GetRenderWindowPart()->GetQmitkRenderWindows().values())
  //{
   // this->m_Controls.m_FiducialRegistrationWidget->AddSliceNavigationController(renderWindow->GetSliceNavigationController());
  //}

}

void IGTFiducialRegistration::PointerSelectionChanged()
{
  this->m_Controls.m_FiducialRegistrationWidget->SetTrackerFiducialsNode(m_Controls.m_TrackingDeviceSelectionWidget->GetSelectedNavigationTool()->GetDataNode());
}

void IGTFiducialRegistration::ImageSelectionChanged()
{
  this->m_Controls.m_FiducialRegistrationWidget->SetImageFiducialsNode(m_Controls.m_DataStorageComboBox->GetSelectedNode());
}

IGTFiducialRegistration::IGTFiducialRegistration()
{

}

IGTFiducialRegistration::~IGTFiducialRegistration()
{

}
