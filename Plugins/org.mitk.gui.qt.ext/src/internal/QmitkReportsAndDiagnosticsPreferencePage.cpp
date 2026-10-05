/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkReportsAndDiagnosticsPreferencePage.h"

#include <QLabel>
#include <QVBoxLayout>

void QmitkReportsAndDiagnosticsPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkReportsAndDiagnosticsPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);

  auto* label = new QLabel(
    "When MITK closes unexpectedly or stops responding, it can save diagnostic data (crash dumps) "
    "on this computer. Handed in with a problem report, they help us find and fix the cause.<br/><br/>"
    "MITK never uploads diagnostic data on its own. The pages below this one control what is recorded "
    "and how long it is kept.");
  label->setWordWrap(true);

  auto* layout = new QVBoxLayout(m_Control);
  layout->addWidget(label);
  layout->addStretch();
}

QWidget* QmitkReportsAndDiagnosticsPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkReportsAndDiagnosticsPreferencePage::PerformOk()
{
  return true;
}

void QmitkReportsAndDiagnosticsPreferencePage::PerformCancel()
{
}

void QmitkReportsAndDiagnosticsPreferencePage::Update()
{
}
