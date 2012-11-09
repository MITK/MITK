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

#include "QmitkDicomPreferencePage.h"
#include "QmitkDicomEditor.h"

#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>
#include <berryPlatform.h>

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
    m_MainControl->setWindowTitle(QApplication::translate("DicomPreferencePage", "Form", 0, QApplication::UnicodeUTF8));
    formLayout = new QFormLayout(m_MainControl);
    formLayout->setObjectName(QString::fromUtf8("formLayout"));
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    label = new QLabel(m_MainControl);
    label->setObjectName(QString::fromUtf8("label"));
    label->setText(QApplication::translate("DicomPreferencePage", "Database directory:", 0, QApplication::UnicodeUTF8));

    formLayout->setWidget(0, QFormLayout::LabelRole, label);

    DatabaseLineEdit = new QLineEdit(m_MainControl);
    DatabaseLineEdit->setObjectName(QString::fromUtf8("DatabaseLineEdit"));

    formLayout->setWidget(0, QFormLayout::FieldRole, DatabaseLineEdit);

    label_2 = new QLabel(m_MainControl);
    label_2->setObjectName(QString::fromUtf8("label_2"));
    label_2->setText(QApplication::translate("DicomPreferencePage", "Dicom listener directory:", 0, QApplication::UnicodeUTF8));

    formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

    ListenerLineEdit = new QLineEdit(m_MainControl);
    ListenerLineEdit->setObjectName(QString::fromUtf8("ListenerLineEdit"));

    formLayout->setWidget(1, QFormLayout::FieldRole, ListenerLineEdit);

    frame = new QFrame(m_MainControl);
    frame->setObjectName(QString::fromUtf8("frame"));
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);

    formLayout->setWidget(3, QFormLayout::FieldRole, frame);



}

QWidget* QmitkDicomPreferencePage::GetQtControl() const
{
    return m_MainControl;
}

void QmitkDicomPreferencePage::PerformCancel()
{
}

bool QmitkDicomPreferencePage::PerformOk()
{
    return true;
}

void QmitkDicomPreferencePage::Update()
{
}