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

#include <berryIPreferencesService.h>
#include <berryPlatform.h>
#include <berryServiceRegistry.h>
#include <mitkExceptionMacro.h>
#include <QFileDialog>
#include <QProcess>
#include <QTextCodec>
#include <ui_QmitkExternalProgramsPreferencePage.h>
#include "QmitkExternalProgramsPreferencePage.h"

static berry::IPreferences::Pointer GetPreferences()
{
  berry::IPreferencesService::Pointer preferencesService =
    berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  if (preferencesService.IsNotNull())
  {
    berry::IPreferences::Pointer systemPreferences = preferencesService->GetSystemPreferences();

    if (systemPreferences.IsNotNull())
      return systemPreferences->Node("/org.mitk.gui.qt.ext.externalprograms");
  }

  mitkThrow();
}

QmitkExternalProgramsPreferencePage::QmitkExternalProgramsPreferencePage()
  : m_Preferences(GetPreferences()),
    m_Ui(new Ui::QmitkExternalProgramsPreferencePage),
    m_Control(NULL),
    m_GnuplotProcess(NULL)
{
}

QmitkExternalProgramsPreferencePage::~QmitkExternalProgramsPreferencePage()
{
}

void QmitkExternalProgramsPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  m_GnuplotProcess = new QProcess(m_Control);

  m_Ui->setupUi(m_Control);

  connect(m_GnuplotProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(OnGnuplotProcessError(QProcess::ProcessError)));
  connect(m_GnuplotProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(OnGnuplotProcessFinished(int, QProcess::ExitStatus)));
  connect(m_Ui->gnuplotButton, SIGNAL(clicked()), this, SLOT(OnGnuplotButtonClicked()));

  this->Update();
}

void QmitkExternalProgramsPreferencePage::OnGnuplotButtonClicked()
{
  QString filter = "Gnuplot executable ";

#if defined(WIN32)
  filter += "(gnuplot.exe)";
#else
  filter += "(gnuplot)";
#endif

  QString gnuplotPath = QFileDialog::getOpenFileName(m_Control, "Gnuplot", "", filter);

  if (!gnuplotPath.isEmpty())
  {
    m_GnuplotPath = gnuplotPath;
    m_GnuplotProcess->start(gnuplotPath, QStringList() << "--version", QProcess::ReadOnly);
  }
}

void QmitkExternalProgramsPreferencePage::OnGnuplotProcessError(QProcess::ProcessError)
{
  m_GnuplotPath.clear();
  m_Ui->gnuplotLineEdit->clear();
}

void QmitkExternalProgramsPreferencePage::OnGnuplotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitStatus == QProcess::NormalExit && exitCode == 0)
  {
    QString version = QTextCodec::codecForName("UTF-8")->toUnicode(m_GnuplotProcess->readAllStandardOutput()).trimmed();

    if (version.startsWith("gnuplot"))
    {
      m_Ui->gnuplotLineEdit->setText(QString("%1 (%2)").arg(m_GnuplotPath).arg(version.trimmed()));
      return;
    }
  }

  m_GnuplotPath.clear();
  m_Ui->gnuplotLineEdit->clear();
}

QWidget* QmitkExternalProgramsPreferencePage::GetQtControl() const
{
  return m_Control;
}

void QmitkExternalProgramsPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkExternalProgramsPreferencePage::PerformCancel()
{
}

bool QmitkExternalProgramsPreferencePage::PerformOk()
{
  m_Preferences->Put("gnuplot", m_GnuplotPath.toStdString());
  return true;
}

void QmitkExternalProgramsPreferencePage::Update()
{
  m_GnuplotPath = QString::fromStdString(m_Preferences->Get("gnuplot", ""));

  if (!m_GnuplotPath.isEmpty())
    m_GnuplotProcess->start(m_GnuplotPath, QStringList() << "--version", QProcess::ReadOnly);
}
