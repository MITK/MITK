/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-07-07 16:57:15 +0200 (Di, 07 Jul 2009) $
 Version:   $Revision: 18019 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "QmitkDataManagerPreferencePage.h"
#include "QmitkDataManagerView.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

QmitkDataManagerPreferencePage::QmitkDataManagerPreferencePage()
: m_MainControl(0)
{

}

void QmitkDataManagerPreferencePage::Init(berry::IWorkbench::Pointer )
{

}

void QmitkDataManagerPreferencePage::CreateQtControl(QWidget* parent)
{
  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  m_DataManagerPreferencesNode = prefService->GetSystemPreferences()->Node(QmitkDataManagerView::VIEW_ID);

  m_MainControl = new QWidget(parent); 
  m_EnableSingleEditing = new QCheckBox;
  m_PlaceNewNodesOnTop = new QCheckBox;
  m_ShowHelperObjects = new QCheckBox;
  m_ShowNodesContainingNoData = new QCheckBox;
  m_UseSurfaceDecimation = new QCheckBox;

  QFormLayout *formLayout = new QFormLayout; 
  formLayout->addRow("&Single click property editing:", m_EnableSingleEditing);
  formLayout->addRow("&Place new nodes on top:", m_PlaceNewNodesOnTop);
  formLayout->addRow("&Show helper objects:", m_ShowHelperObjects);
  formLayout->addRow("&Show nodes containing no data", m_ShowNodesContainingNoData);
  formLayout->addRow("&Use surface decimation:", m_UseSurfaceDecimation);

  m_MainControl->setLayout(formLayout);
  this->Update();
}

QWidget* QmitkDataManagerPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkDataManagerPreferencePage::PerformOk()
{  
  m_DataManagerPreferencesNode->PutBool("Single click property editing"
                                        , m_EnableSingleEditing->isChecked());
  m_DataManagerPreferencesNode->PutBool("Place new nodes on top"
                                        , m_PlaceNewNodesOnTop->isChecked());
  m_DataManagerPreferencesNode->PutBool("Show helper objects"
                                        , m_ShowHelperObjects->isChecked());
  m_DataManagerPreferencesNode->PutBool("Show nodes containing no data"
                                        , m_ShowNodesContainingNoData->isChecked());
  m_DataManagerPreferencesNode->PutBool("Use surface decimation"
                                        , m_UseSurfaceDecimation->isChecked());
  return true;
}

void QmitkDataManagerPreferencePage::PerformCancel()
{

}

void QmitkDataManagerPreferencePage::Update()
{  
  m_EnableSingleEditing->setChecked(m_DataManagerPreferencesNode->GetBool("Single click property editing", true));
  m_PlaceNewNodesOnTop->setChecked(m_DataManagerPreferencesNode->GetBool("Place new nodes on top", true));
  m_ShowHelperObjects->setChecked(m_DataManagerPreferencesNode->GetBool("Show helper objects", false));
  m_ShowNodesContainingNoData->setChecked(m_DataManagerPreferencesNode->GetBool("Show nodes containing no data", false));
  m_UseSurfaceDecimation->setChecked(m_DataManagerPreferencesNode->GetBool("Use surface decimation", true));
}
