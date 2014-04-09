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
#include <mitkVersion.h>
#include <itkConfigure.h>
#include <vtkConfigure.h>
#include <QPushButton>
#include <vtkVersionMacros.h>

QmitkAboutDialog::QmitkAboutDialog(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
  m_GUI.setupUi(this);

  QString mitkRevision(MITK_REVISION);
  QString mitkRevisionDescription(MITK_REVISION_DESC);
  QString itkVersion = QString("%1.%2.%3").arg(ITK_VERSION_MAJOR).arg(ITK_VERSION_MINOR).arg(ITK_VERSION_PATCH);
  QString vtkVersion = QString("%1.%2.%3").arg(VTK_MAJOR_VERSION).arg(VTK_MINOR_VERSION).arg(VTK_BUILD_VERSION);

  QString revisionText = QString("Revision: %1").arg(MITK_REVISION);

  if (!QString(MITK_REVISION_DESC).isEmpty())
    revisionText += QString("\nDescription: %1").arg(MITK_REVISION_DESC);

  m_GUI.m_RevisionLabel->setText(revisionText);
  m_GUI.m_ToolkitVersionsLabel->setText(QString("ITK %1, VTK %2, Qt %3").arg(itkVersion, vtkVersion, QT_VERSION_STR));

  QPushButton* btnModules = new QPushButton(QIcon(":/QtWidgetsExt/ModuleView.png"), "Modules");
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

