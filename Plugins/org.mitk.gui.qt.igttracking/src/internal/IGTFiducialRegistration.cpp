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

  //Connect signals and slots
  connect(m_Controls.m_ChooseSelectedPointer, SIGNAL(clicked()), this, SLOT(PointerSelectionChanged()));
  connect(m_Controls.m_ChooseSelectedImage, SIGNAL(clicked()), this, SLOT(ImageSelectionChanged()));

  //Initialize Combobox
  m_Controls.m_DataStorageComboBox->SetDataStorage(this->GetDataStorage());
  m_Controls.m_DataStorageComboBox->SetAutoSelectNewItems(false);
  m_Controls.m_DataStorageComboBox->SetPredicate(mitk::NodePredicateOr::New(mitk::NodePredicateDataType::New("Surface"), mitk::NodePredicateDataType::New("Image")));

  //Initialize Fiducial Registration Widget
  m_Controls.m_FiducialRegistrationWidget->setDataStorage(this->GetDataStorage());
  m_Controls.m_FiducialRegistrationWidget->HideStaticRegistrationRadioButton(true);
  m_Controls.m_FiducialRegistrationWidget->HideContinousRegistrationRadioButton(true);
  m_Controls.m_FiducialRegistrationWidget->HideUseICPRegistrationCheckbox(true);

}

void IGTFiducialRegistration::InitializeRegistration()
{
  foreach(QmitkRenderWindow* renderWindow, this->GetRenderWindowPart()->GetQmitkRenderWindows().values())
  {
    this->m_Controls.m_FiducialRegistrationWidget->AddSliceNavigationController(renderWindow->GetSliceNavigationController());
  }
}

void IGTFiducialRegistration::PointerSelectionChanged()
{
  InitializeRegistration();
  int toolID = m_Controls.m_TrackingDeviceSelectionWidget->GetSelectedToolID();
  m_TrackingPointer = m_Controls.m_TrackingDeviceSelectionWidget->GetSelectedNavigationDataSource()->GetOutput(toolID);
  m_Controls.m_FiducialRegistrationWidget->setTrackerNavigationData(m_TrackingPointer);
  m_Controls.m_PointerLabel->setText(m_Controls.m_TrackingDeviceSelectionWidget->GetSelectedNavigationTool()->GetToolName().c_str());
}

void IGTFiducialRegistration::ImageSelectionChanged()
{
  InitializeRegistration();
  m_Controls.m_ImageLabel->setText(m_Controls.m_DataStorageComboBox->GetSelectedNode()->GetName().c_str());
  m_Controls.m_FiducialRegistrationWidget->setImageNode(m_Controls.m_DataStorageComboBox->GetSelectedNode());
}

IGTFiducialRegistration::IGTFiducialRegistration()
{

}

IGTFiducialRegistration::~IGTFiducialRegistration()
{

}
