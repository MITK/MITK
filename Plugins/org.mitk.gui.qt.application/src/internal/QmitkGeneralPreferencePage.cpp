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

#include "QmitkGeneralPreferencePage.h"

#include <QVBoxLayout>

#include <berryIPreferencesService.h>
#include <berryQtPreferences.h>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <mitkSceneIO.h>

QmitkGeneralPreferencePage::QmitkGeneralPreferencePage()
: m_MainControl(0)
{

}

void QmitkGeneralPreferencePage::Init(berry::IWorkbench::Pointer )
{
}

void QmitkGeneralPreferencePage::CreateQtControl(QWidget* parent)
{
  //empty page
  m_MainControl = new QWidget(parent);
  m_SavePersistentDataWithSceneCheckBox = new QCheckBox("Save Application Data with Scene Files");
  m_LoadPersistentDataWithSceneCheckBox = new QCheckBox("Load Application Data with Scene Files");

  QLabel* helpLabel = new QLabel("<html><strong>Help:</strong>&nbsp;Modules and plugins of the MITK Workbench can store data in an internal database. This database can be included into scene files while saving or restored while loading a scene file. This is useful if the modules you are using support this internal database and you want to save images along with application data (e.g. settings, parameters, etc.).</html>");
  helpLabel->setWordWrap(true);

  QVBoxLayout *layout2 = new QVBoxLayout;
  layout2->addWidget(helpLabel);
  layout2->addWidget(m_SavePersistentDataWithSceneCheckBox);
  layout2->addWidget(m_LoadPersistentDataWithSceneCheckBox);

  QGroupBox* persistentDataGb = new QGroupBox("Persistent Data handling");
  persistentDataGb->setLayout(layout2);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(persistentDataGb);
  layout->addStretch();
  m_MainControl->setLayout(layout);


  berry::IPreferencesService::Pointer prefService
      = berry::Platform::GetServiceRegistry()
      .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  prefs = prefService->GetSystemPreferences()->Node("/General");

  this->Update();
}

QWidget* QmitkGeneralPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkGeneralPreferencePage::PerformOk()
{
    prefs->PutBool("savePersistentDataWithScene", m_SavePersistentDataWithSceneCheckBox->isChecked());
    prefs->PutBool("loadPersistentDataWithScene", m_LoadPersistentDataWithSceneCheckBox->isChecked());

    mitk::SceneIO::SetSavePersistentDataWithScene(m_SavePersistentDataWithSceneCheckBox->isChecked());
    mitk::SceneIO::SetLoadPersistentDataWithScene( m_LoadPersistentDataWithSceneCheckBox->isChecked());
    prefs->Flush();
  return true;
}

void QmitkGeneralPreferencePage::PerformCancel()
{

}

void QmitkGeneralPreferencePage::Update()
{
    bool savePersistentDataWithSceneCheckBox = prefs->GetBool("savePersistentDataWithScene", false);
    bool loadPersistentDataWithSceneCheckBox = prefs->GetBool("loadPersistentDataWithScene", false);

    m_SavePersistentDataWithSceneCheckBox->setChecked(savePersistentDataWithSceneCheckBox);
    m_LoadPersistentDataWithSceneCheckBox->setChecked(loadPersistentDataWithSceneCheckBox);
}
