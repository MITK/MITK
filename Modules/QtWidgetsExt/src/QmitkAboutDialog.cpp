/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAboutDialog.h"
#include "QmitkModulesDialog.h"
#include <QPushButton>
#include <QRegularExpression>
#include <itkConfigure.h>
#include <mitkVersion.h>
#include <vtkVersionMacros.h>

namespace
{
  QString CreateRevisionLabelText()
  {
    auto revisionText = QString("<html><head/><body><p>Revision: <a href=\"https://phabricator.mitk.org/rMITK%1\"><span style=\"text-decoration: underline; color:#5555ff;\">%1</span></a>").arg(MITK_REVISION);
    QString mitkRevisionDescription(MITK_REVISION_DESC);

    if (!mitkRevisionDescription.isEmpty())
      revisionText += QString("<br>Description: %1").arg(mitkRevisionDescription);

    revisionText += "</p></body></html>";

    return revisionText;
  }

  QString CreateToolkitVersionsLabelText()
  {
    auto itkVersion = QString("%1.%2.%3").arg(ITK_VERSION_MAJOR).arg(ITK_VERSION_MINOR).arg(ITK_VERSION_PATCH);
    return QString("ITK %1, VTK %2, Qt %3").arg(itkVersion, VTK_VERSION, QT_VERSION_STR);
  }

  QString MatchDocumentationToReleaseVersion(QString nightly)
  {
    QRegularExpression regEx("^v(\\d\\d\\d\\d\\.\\d\\d)$"); // "v", 4 digits, dot, 2 digits
    auto match = regEx.match(MITK_REVISION_DESC);

    if (match.hasMatch())
      nightly.replace("nightly", match.captured(1)); // Replace with captured 4 digits, dot, 2 digits

    return nightly;
  }
}

QmitkAboutDialog::QmitkAboutDialog(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
  m_GUI.setupUi(this);

  m_GUI.m_RevisionLabel->setText(CreateRevisionLabelText());
  m_GUI.m_ToolkitVersionsLabel->setText(CreateToolkitVersionsLabelText());
  m_GUI.m_AboutLabel->setText(MatchDocumentationToReleaseVersion(m_GUI.m_AboutLabel->text()));

  auto* btnModules = new QPushButton(QIcon(":/QtWidgetsExt/ModuleView.png"), "Modules");
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
