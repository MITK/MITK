/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-03-21 19:27:37 +0100 (Sa, 21 Mrz 2009) $
Version:   $Revision: 16719 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkFiducialRegistrationWidget.h"


#define FRW_LOG MITK_INFO("Fiducial Registration Widget")
#define FRW_WARN MITK_WARN("Fiducial Registration Widget")
#define FRW_DEBUG MITK_DEBUG("Fiducial Registration Widget")

/* VIEW MANAGEMENT */
QmitkFiducialRegistrationWidget::QmitkFiducialRegistrationWidget(QWidget* parent)  
: QWidget(parent), m_Controls(NULL),m_MultiWidget(NULL), m_ImageFiducialsNode(NULL), m_TrackerFiducialsNode(NULL)
{
  CreateQtPartControl(this);
}


QmitkFiducialRegistrationWidget::~QmitkFiducialRegistrationWidget()
{
  m_Controls = NULL;
}


void QmitkFiducialRegistrationWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkFiducialRegistrationWidget;
    m_Controls->setupUi(parent);

    // hide additional image fiducial button
    m_Controls->m_AddImageFiducialBtn->setHidden(true);

    this->CreateConnections();
  }
}


void QmitkFiducialRegistrationWidget::CreateConnections()
{     
  connect( (QObject*)(m_Controls->m_AddTrackingFiducialBtn), SIGNAL(clicked()), this, SIGNAL(AddedTrackingFiducial()) );
  connect( (QObject*)(m_Controls->m_AddImageFiducialBtn), SIGNAL(clicked()), this, SIGNAL(AddedImageFiducial()) );
  connect( (QObject*)(m_Controls->m_RegisterFiducialsBtn), SIGNAL(clicked()), this, SIGNAL(PerformFiducialRegistration()) );
  connect( (QObject*)(m_Controls->m_UseICPRegistration), SIGNAL(toggled(bool)), this, SIGNAL(FindFiducialCorrespondences(bool)) );
}

void QmitkFiducialRegistrationWidget::SetWidgetAppearanceMode(WidgetAppearanceMode widgetMode)
{
  if (widgetMode==LANDMARKMODE)
  {
    this->HideContinousRegistrationRadioButton(true);
    this->HideStaticRegistrationRadioButton(true);
    this->HideUseICPRegistrationCheckbox(true);
    this->HideImageFiducialButton(false);
    this->m_Controls->registrationGroupBox->setTitle("");
    this->m_Controls->sourceLandmarksGroupBox->setTitle("Target/Reference landmarks");
    this->m_Controls->targetLandmarksGroupBox->setTitle("Source Landmarks");
    this->m_Controls->m_AddImageFiducialBtn->setText("Add target landmark");
    this->m_Controls->m_AddTrackingFiducialBtn->setText("Add source landmark");
  }
  else if (widgetMode==FIDUCIALMODE)
  {
    this->HideContinousRegistrationRadioButton(false);
    this->HideStaticRegistrationRadioButton(false);
    this->HideUseICPRegistrationCheckbox(false);
    this->HideImageFiducialButton(true);
    this->m_Controls->registrationGroupBox->setTitle("Select fiducials in image and OR (world)");
    this->m_Controls->sourceLandmarksGroupBox->setTitle("Image fiducials");
    this->m_Controls->targetLandmarksGroupBox->setTitle("OR fiducials");
    this->m_Controls->m_AddImageFiducialBtn->setText("Add image fiducial");
    this->m_Controls->m_AddTrackingFiducialBtn->setText("Add current instrument position");
  }
}

void QmitkFiducialRegistrationWidget::SetQualityDisplayText( QString text )
{
  if (text == NULL)
    return;
  m_Controls->m_RegistrationQualityDisplay->setText(text); // set text on the QLabel
}

bool QmitkFiducialRegistrationWidget::UseICPIsChecked()
{
  if(m_Controls->m_UseICPRegistration->isChecked())
    return true;
  else
    return false;
}

void QmitkFiducialRegistrationWidget::SetImageFiducialsNode( mitk::DataNode::Pointer imageFiducialsNode )
{
  if(imageFiducialsNode.IsNull())
  {
    FRW_WARN<< "tracker fiducial node is NULL";
    return;
  }
  if(m_MultiWidget == NULL)
  {
    FRW_WARN<< "stdMultiWidget is NULL";
    return;
  }
  m_Controls->m_RegistrationImagePoints->SetMultiWidget(m_MultiWidget); // pass multiWidget to pointListWidget
  m_Controls->m_RegistrationImagePoints->SetPointSetNode(imageFiducialsNode); // pass node to pointListWidget


}

void QmitkFiducialRegistrationWidget::SetTrackerFiducialsNode( mitk::DataNode::Pointer trackerFiducialsNode )
{
  if(trackerFiducialsNode.IsNull())
  {
    FRW_WARN<< "tracker fiducial node is NULL";
    return;
  }
  if(m_MultiWidget == NULL)
  {
    FRW_WARN<< "stdMultiWidget is NULL";
    return;
  }
  m_Controls->m_RegistrationTrackingPoints->SetMultiWidget(m_MultiWidget); // pass multiWidget to pointListWidget
  m_Controls->m_RegistrationTrackingPoints->SetPointSetNode(trackerFiducialsNode); // pass node to pointListWidget

}

void QmitkFiducialRegistrationWidget::SetMultiWidget( QmitkStdMultiWidget* multiWidget )
{
  m_MultiWidget=multiWidget;
}

mitk::DataNode::Pointer QmitkFiducialRegistrationWidget::GetImageFiducialsNode()
{
  return m_ImageFiducialsNode;
}

mitk::DataNode::Pointer QmitkFiducialRegistrationWidget::GetTrackerFiducialsNode()
{
  return m_TrackerFiducialsNode;
}

void QmitkFiducialRegistrationWidget::HideStaticRegistrationRadioButton( bool on )
{
  m_Controls->m_rbStaticRegistration->setHidden(on);
}

void QmitkFiducialRegistrationWidget::HideContinousRegistrationRadioButton( bool on )
{
  m_Controls->m_rbContinousRegistration->setHidden(on);
}

void QmitkFiducialRegistrationWidget::HideUseICPRegistrationCheckbox( bool on )
{
  m_Controls->m_UseICPRegistration->setHidden(on);
}

void QmitkFiducialRegistrationWidget::HideImageFiducialButton( bool on )
{
  m_Controls->m_AddImageFiducialBtn->setHidden(on);
}
