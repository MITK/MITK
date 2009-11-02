/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 18019 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkSegmentationPreferencePage.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>

#include <cherryIPreferencesService.h>
#include <cherryPlatform.h>

QmitkSegmentationPreferencePage::QmitkSegmentationPreferencePage()
: m_MainControl(0)
{

}

void QmitkSegmentationPreferencePage::Init(cherry::IWorkbench::Pointer )
{

}

void QmitkSegmentationPreferencePage::CreateQtControl(QWidget* parent)
{
  cherry::IPreferencesService::Pointer prefService 
    = cherry::Platform::GetServiceRegistry()
    .GetServiceById<cherry::IPreferencesService>(cherry::IPreferencesService::ID);

  m_SegmentationPreferencesNode = prefService->GetSystemPreferences()->Node("/Segmentation");

  m_MainControl = new QWidget(parent);

  QVBoxLayout* displayOptionsLayout = new QVBoxLayout;
  m_RadioOutline = new QRadioButton( "Draw as outline", m_MainControl);
  displayOptionsLayout->addWidget( m_RadioOutline );
  m_RadioOverlay = new QRadioButton( "Draw as transparent overlay", m_MainControl);
  displayOptionsLayout->addWidget( m_RadioOverlay );

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow( "2D display", displayOptionsLayout );

  m_VolumeRenderingCheckBox = new QCheckBox( "Show as volume rendering", m_MainControl );
  formLayout->addRow( "3D display", m_VolumeRenderingCheckBox );
  
  m_MainControl->setLayout(formLayout);
  this->Update();
}

QWidget* QmitkSegmentationPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkSegmentationPreferencePage::PerformOk()
{
  m_SegmentationPreferencesNode->PutBool("draw outline", m_RadioOutline->isChecked());
  m_SegmentationPreferencesNode->PutBool("volume rendering", m_VolumeRenderingCheckBox->isChecked());
  return true;
}

void QmitkSegmentationPreferencePage::PerformCancel()
{

}

void QmitkSegmentationPreferencePage::Update()
{
  //m_EnableSingleEditing->setChecked(m_SegmentationPreferencesNode->GetBool("Single click property editing", true));
  if (m_SegmentationPreferencesNode->GetBool("draw outline", true) )
  {
    m_RadioOutline->setChecked( true );
  }
  else
  {
    m_RadioOverlay->setChecked( true );
  }

  m_VolumeRenderingCheckBox->setChecked( m_SegmentationPreferencesNode->GetBool("volume rendering", true) );
}

