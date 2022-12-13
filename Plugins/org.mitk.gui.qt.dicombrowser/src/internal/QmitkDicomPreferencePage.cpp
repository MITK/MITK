/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkDicomPreferencePage.h"

#include "mitkPluginActivator.h"
#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QFileDialog>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.dicomreader");
  }

  QString CreateDefaultPath()
  {
    QString path = mitk::PluginActivator::getContext()->getDataFile("").absolutePath();
    path.append("/database");
    return path;
  }
}

QmitkDicomPreferencePage::QmitkDicomPreferencePage()
  : m_MainControl(nullptr)
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
  m_MainControl = new QWidget(parent);

  auto  formLayout = new QFormLayout;
  formLayout->setHorizontalSpacing(8);
  formLayout->setVerticalSpacing(24);

  auto   displayOptionsLayout = new QHBoxLayout;
  m_PathEdit = new QLineEdit(m_MainControl);
  displayOptionsLayout->addWidget(m_PathEdit);

  m_PathSelect = new QPushButton("Select Path",m_MainControl);
  displayOptionsLayout->addWidget(m_PathSelect);

  m_PathDefault = new QPushButton("Default",m_MainControl);
  displayOptionsLayout->addWidget(m_PathDefault);

  formLayout->addRow("Local database path:",displayOptionsLayout);
  m_MainControl->setLayout(formLayout);

  connect(m_PathDefault, SIGNAL(clicked()), this, SLOT(DefaultButtonPushed()));
  connect(m_PathSelect, SIGNAL(clicked()), this, SLOT(PathSelectButtonPushed()));

  this->Update();
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
  auto* prefs = GetPreferences();
  prefs->Put("default dicom path",m_PathEdit->text().toStdString());
  return true;
}

void QmitkDicomPreferencePage::Update()
{
  auto* prefs = GetPreferences();
  const auto path = QString::fromStdString(prefs->Get("default dicom path", CreateDefaultPath().toStdString()));
  m_PathEdit->setText(path);
}

void QmitkDicomPreferencePage::DefaultButtonPushed()
{
  m_PathEdit->setText(CreateDefaultPath());
}

void QmitkDicomPreferencePage::PathSelectButtonPushed()
{
  QString path = QFileDialog::getExistingDirectory(m_MainControl,"Folder for Dicom directory","dir");
  if (!path.isEmpty())
  {
    m_PathEdit->setText(path);
  }
}
