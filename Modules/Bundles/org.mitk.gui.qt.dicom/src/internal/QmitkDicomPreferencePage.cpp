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
#include "QmitkDicomPreferencePage.h"
#include "QmitkDicomEditor.h"

#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>
#include <berryPlatform.h>

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>

QmitkDicomPreferencePage::QmitkDicomPreferencePage() 
: m_MainControl(0)
{
   
}

QmitkDicomPreferencePage::~QmitkDicomPreferencePage()
{

}

void QmitkDicomPreferencePage::Init(berry::IWorkbench::Pointer )
{

}

void QmitkDicomPreferencePage::CreateQtControl(QWidget* parent)
{
  berry::IPreferencesService::Pointer prefService= 
      berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);    

  m_DicomPreferencesNode = prefService->GetSystemPreferences()->Node(QmitkDicomEditor::EDITOR_ID).Cast<berry::IBerryPreferences>();;
  assert( m_DicomPreferencesNode );

    m_MainControl = new QWidget(parent);
    m_EnableSingleEditing = new QCheckBox;
    m_PlaceNewNodesOnTop= new QCheckBox;
    m_ShowHelperObjects= new QCheckBox;
    m_ShowNodesContainingNoData= new QCheckBox;
    m_UseSurfaceDecimation= new QCheckBox;
//  QString pluginDirectory;
//mitk::PluginActivator::getContext()->getDataFile(pluginDirectory);
//pluginDirectory.append("/");
//QString databaseDirectory;
////databaseDirectory.append(pluginDirectory);
//databaseDirectory.append(QString("C:/DicomDatabase"));
//QDir tmp(databaseDirectory);
//tmp.mkpath(databaseDirectory);
//m_Controls.internalDataWidget->SetDatabaseDirectory(databaseDirectory);
//QString listenerDirectory("C:/DICOMListenerDirectory");
//StartDicomDirectoryListener(listenerDirectory);
//QmitkStoreSCPLauncherBuilder builder;
//builder.AddPort()->AddTransferSyntax()->AddOtherNetworkOptions()->AddMode()->AddOutputDirectory(listenerDirectory);
//m_StoreSCPLauncher = new QmitkStoreSCPLauncher(&builder);
//m_StoreSCPLauncher->StartStoreSCP();

  QFormLayout *formLayout = new QFormLayout; 
  formLayout->addRow("&Single click property editing:", m_EnableSingleEditing);
  formLayout->addRow("&Place new nodes on top:", m_PlaceNewNodesOnTop);
  formLayout->addRow("&Show helper objects:", m_ShowHelperObjects);
  formLayout->addRow("&Show nodes containing no data", m_ShowNodesContainingNoData);
  formLayout->addRow("&Use surface decimation:", m_UseSurfaceDecimation);

  m_MainControl->setLayout(formLayout);
  this->Update();
}

QWidget* QmitkDicomPreferencePage::GetQtControl() const
{
    return m_MainControl;
}

bool QmitkDicomPreferencePage::PerformOk()
{
  m_DicomPreferencesNode->PutBool("Single click property editing"
                                        , m_EnableSingleEditing->isChecked());
  m_DicomPreferencesNode->PutBool("Place new nodes on top"
                                        , m_PlaceNewNodesOnTop->isChecked());
  m_DicomPreferencesNode->PutBool("Show helper objects"
                                        , m_ShowHelperObjects->isChecked());
  m_DicomPreferencesNode->PutBool("Show nodes containing no data"
                                        , m_ShowNodesContainingNoData->isChecked());
  m_DicomPreferencesNode->PutBool("Use surface decimation"
                                        , m_UseSurfaceDecimation->isChecked());
  return true;
}

void QmitkDicomPreferencePage::PerformCancel()
{

}

void QmitkDicomPreferencePage::Update()
{
  m_EnableSingleEditing->setChecked(m_DicomPreferencesNode->GetBool("Single click property editing", true));
  m_PlaceNewNodesOnTop->setChecked(m_DicomPreferencesNode->GetBool("Place new nodes on top", true));
  m_ShowHelperObjects->setChecked(m_DicomPreferencesNode->GetBool("Show helper objects", false));
  m_ShowNodesContainingNoData->setChecked(m_DicomPreferencesNode->GetBool("Show nodes containing no data", false));
  m_UseSurfaceDecimation->setChecked(m_DicomPreferencesNode->GetBool("Use surface decimation", true));
}