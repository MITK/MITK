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

#include "QmitkStdMultiWidgetEditorPreferencePage.h"
#include "QmitkStdMultiWidgetEditor.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

QmitkStdMultiWidgetEditorPreferencePage::QmitkStdMultiWidgetEditorPreferencePage()
: m_MainControl(0)
{

}

QmitkStdMultiWidgetEditorPreferencePage::QmitkStdMultiWidgetEditorPreferencePage(const QmitkStdMultiWidgetEditorPreferencePage& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

void QmitkStdMultiWidgetEditorPreferencePage::Init(berry::IWorkbench::Pointer )
{

}

void QmitkStdMultiWidgetEditorPreferencePage::CreateQtControl(QWidget* parent)
{
  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  m_StdMultiWidgetEditorPreferencesNode = prefService->GetSystemPreferences()->Node(QmitkStdMultiWidgetEditor::EDITOR_ID);

  m_MainControl = new QWidget(parent);
  m_EnableFlexibleZooming = new QCheckBox;  

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow("&Use constrained zooming and padding", m_EnableFlexibleZooming);


  m_MainControl->setLayout(formLayout);
  this->Update();
}

QWidget* QmitkStdMultiWidgetEditorPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkStdMultiWidgetEditorPreferencePage::PerformOk()
{
  m_StdMultiWidgetEditorPreferencesNode->PutBool("Use constrained zooming and padding"
                                        , m_EnableFlexibleZooming->isChecked());

  return true;
}

void QmitkStdMultiWidgetEditorPreferencePage::PerformCancel()
{

}

void QmitkStdMultiWidgetEditorPreferencePage::Update()
{
  m_EnableFlexibleZooming->setChecked(m_StdMultiWidgetEditorPreferencesNode->GetBool("Use constrained zooming and padding", true));

}
