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

    m_Controls->m_spaceHolderGroupBox->setStyleSheet("QGroupBox {border: 0px transparent;}");
    m_Controls->m_spaceHolderGroupBox2->setStyleSheet("QGroupBox {border: 0px transparent;}");

    this->CreateConnections();
  }
}


void QmitkFiducialRegistrationWidget::CreateConnections()
{
  connect( (QObject*)(m_Controls->m_AddTrackingFiducialBtn), SIGNAL(clicked()), this, SIGNAL(AddedTrackingFiducial()) );
  connect( (QObject*)(m_Controls->m_AddImageFiducialBtn), SIGNAL(clicked()), this, SIGNAL(AddedImageFiducial()) );
  connect( (QObject*)(m_Controls->m_RegisterFiducialsBtn), SIGNAL(clicked()), this, SIGNAL(PerformFiducialRegistration()) );
  connect( (QObject*)(m_Controls->m_UseICPRegistration), SIGNAL(toggled(bool)), this, SIGNAL(FindFiducialCorrespondences(bool)) );

  //unselects the edit button of the other widget if one is selected
  connect( (QObject*)(m_Controls->m_RegistrationImagePoints), SIGNAL(EditPointSets(bool)), this, SLOT(DisableEditButtonRegistrationTrackingPoints(bool)));
  connect( (QObject*)(m_Controls->m_RegistrationTrackingPoints), SIGNAL(EditPointSets(bool)), this, SLOT(DisableEditButtonRegistrationImagePoints(bool)));
}

void QmitkFiducialRegistrationWidget::DisableEditButtonRegistrationImagePoints(bool activated)
{
if (activated) m_Controls->m_RegistrationImagePoints->UnselectEditButton();
}
void QmitkFiducialRegistrationWidget::DisableEditButtonRegistrationTrackingPoints(bool activated)
{
if (activated) m_Controls->m_RegistrationTrackingPoints->UnselectEditButton();
}

void QmitkFiducialRegistrationWidget::SetWidgetAppearanceMode(WidgetAppearanceMode widgetMode)
{
  if (widgetMode==LANDMARKMODE)
  {
    this->HideContinousRegistrationRadioButton(true);
    this->HideStaticRegistrationRadioButton(true);
    this->HideFiducialRegistrationGroupBox();
    this->HideUseICPRegistrationCheckbox(true);
    this->HideImageFiducialButton(false);
    this->m_Controls->sourceLandmarksGroupBox->setTitle("Target/Reference landmarks");
    this->m_Controls->targetLandmarksGroupBox->setTitle("Source Landmarks");
    this->m_Controls->m_AddImageFiducialBtn->setText("Add target landmark");
    this->m_Controls->m_AddTrackingFiducialBtn->setText("Add source landmark");
  }
  else if (widgetMode==FIDUCIALMODE)
  {
    this->HideContinousRegistrationRadioButton(false);
    this->HideStaticRegistrationRadioButton(false);
    this->HideFiducialRegistrationGroupBox();
    this->HideUseICPRegistrationCheckbox(false);
    this->HideImageFiducialButton(true);
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
  HideFiducialRegistrationGroupBox();
}

void QmitkFiducialRegistrationWidget::HideContinousRegistrationRadioButton( bool on )
{
  m_Controls->m_rbContinousRegistration->setHidden(on);
  HideFiducialRegistrationGroupBox();
}

void QmitkFiducialRegistrationWidget::HideFiducialRegistrationGroupBox()
{
  if (m_Controls->m_rbStaticRegistration->isHidden() && m_Controls->m_rbContinousRegistration->isHidden())
  {
    m_Controls->m_gbFiducialRegistration->setHidden(true);
  }
  else
  {
    m_Controls->m_gbFiducialRegistration->setHidden(false);
  }
}

void QmitkFiducialRegistrationWidget::HideUseICPRegistrationCheckbox( bool on )
{
  m_Controls->m_UseICPRegistration->setHidden(on);
}

void QmitkFiducialRegistrationWidget::HideImageFiducialButton( bool on )
{
  m_Controls->m_AddImageFiducialBtn->setHidden(on);
  AdjustButtonSpacing();

}

void QmitkFiducialRegistrationWidget::HideTrackingFiducialButton( bool on )
{
  m_Controls->m_AddTrackingFiducialBtn->setHidden(on);
  AdjustButtonSpacing();
}

void QmitkFiducialRegistrationWidget::AdjustButtonSpacing()
{
  if (m_Controls->m_AddImageFiducialBtn->isHidden() && m_Controls->m_AddTrackingFiducialBtn->isHidden())
  {
    m_Controls->m_spaceHolderGroupBox->setHidden(true);
    m_Controls->m_spaceHolderGroupBox2->setHidden(true);
  }
  else
  {
    m_Controls->m_spaceHolderGroupBox->setHidden(false);
    m_Controls->m_spaceHolderGroupBox2->setHidden(false);
  }
}
