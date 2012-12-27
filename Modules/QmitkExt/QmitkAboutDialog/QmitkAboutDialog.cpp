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

#include "QmitkAboutDialog.h"
#include "QmitkModulesDialog.h"
#include <QPushButton>
#include <mitkVersion.h>

QmitkAboutDialog::QmitkAboutDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
  m_GUI.setupUi(this);

  QString mitkRevision(MITK_REVISION);
  QString mitkRevisionDescription(MITK_REVISION_DESC);

  m_GUI.m_RevisionLabel->setText(m_GUI.m_RevisionLabel->text().arg(mitkRevision).arg(mitkRevisionDescription));

  QPushButton* btnModules = new QPushButton(QIcon(":/qmitk/ModuleView.png"), "Modules");
  m_GUI.m_ButtonBox->addButton(btnModules, QDialogButtonBox::ActionRole);

  connect(btnModules, SIGNAL(clicked()), this, SLOT(ShowModules()));
  connect(m_GUI.m_ButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

QmitkAboutDialog::~QmitkAboutDialog()
{
}

void QmitkAboutDialog::ShowModules()
{
  QmitkModulesDialog dialog(this);
  dialog.exec();
}

QString QmitkAboutDialog::GetAboutText() const
{
  return m_GUI.m_AboutLabel->text();
}

QString QmitkAboutDialog::GetCaptionText() const
{
  return m_GUI.m_CaptionLabel->text();
}

QString QmitkAboutDialog::GetRevisionText() const
{
  return m_GUI.m_RevisionLabel->text();
}

void QmitkAboutDialog::SetAboutText(const QString &text)
{
  m_GUI.m_AboutLabel->setText(text);
}

void QmitkAboutDialog::SetCaptionText(const QString &text)
{
  m_GUI.m_CaptionLabel->setText(text);
}

void QmitkAboutDialog::SetRevisionText(const QString &text)
{
  m_GUI.m_RevisionLabel->setText(text);
}
