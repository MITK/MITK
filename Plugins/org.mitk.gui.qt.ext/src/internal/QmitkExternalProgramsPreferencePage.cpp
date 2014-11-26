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
    m_FFmpegProcess(NULL),
    m_GnuplotProcess(NULL)
{
}

QmitkExternalProgramsPreferencePage::~QmitkExternalProgramsPreferencePage()
{
}

void QmitkExternalProgramsPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  m_FFmpegProcess = new QProcess(m_Control);
  m_GnuplotProcess = new QProcess(m_Control);

  m_Ui->setupUi(m_Control);

  connect(m_FFmpegProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(OnFFmpegProcessError(QProcess::ProcessError)));
  connect(m_FFmpegProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(OnFFmpegProcessFinished(int, QProcess::ExitStatus)));
  connect(m_Ui->ffmpegButton, SIGNAL(clicked()), this, SLOT(OnFFmpegButtonClicked()));

  connect(m_GnuplotProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(OnGnuplotProcessError(QProcess::ProcessError)));
  connect(m_GnuplotProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(OnGnuplotProcessFinished(int, QProcess::ExitStatus)));
  connect(m_Ui->gnuplotButton, SIGNAL(clicked()), this, SLOT(OnGnuplotButtonClicked()));

  this->Update();
}

void QmitkExternalProgramsPreferencePage::OnFFmpegButtonClicked()
{
  QString filter = "ffmpeg/avconv executable ";

#if defined(WIN32)
  filter += "(ffmpeg.exe avconv.exe)";
#else
  filter += "(ffmpeg avconv)";
#endif

  QString ffmpegPath = QFileDialog::getOpenFileName(m_Control, "FFmpeg/Libav", "", filter);

  if (!ffmpegPath.isEmpty())
  {
    m_FFmpegPath = ffmpegPath;
    m_FFmpegProcess->start(ffmpegPath, QStringList() << "-version", QProcess::ReadOnly);
  }
}

void QmitkExternalProgramsPreferencePage::OnFFmpegProcessError(QProcess::ProcessError)
{
  m_FFmpegPath.clear();
  m_Ui->ffmpegLineEdit->clear();
}

void QmitkExternalProgramsPreferencePage::OnFFmpegProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitStatus == QProcess::NormalExit && exitCode == 0)
  {
    QString output = QTextCodec::codecForName("UTF-8")->toUnicode(m_FFmpegProcess->readAllStandardOutput());

    if (output.startsWith("ffmpeg") || output.startsWith("avconv"))
    {
      m_Ui->ffmpegLineEdit->setText(m_FFmpegPath);
      return;
    }
  }

  m_FFmpegPath.clear();
  m_Ui->ffmpegLineEdit->clear();
}

void QmitkExternalProgramsPreferencePage::OnGnuplotButtonClicked()
{
  QString filter = "gnuplot executable ";

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
    QString output = QTextCodec::codecForName("UTF-8")->toUnicode(m_GnuplotProcess->readAllStandardOutput());

    if (output.startsWith("gnuplot"))
    {
      m_Ui->gnuplotLineEdit->setText(m_GnuplotPath);
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
  m_Preferences->Put("ffmpeg", m_FFmpegPath.toStdString());
  m_Preferences->Put("gnuplot", m_GnuplotPath.toStdString());
  return true;
}

void QmitkExternalProgramsPreferencePage::Update()
{
  m_FFmpegPath = QString::fromStdString(m_Preferences->Get("ffmpeg", ""));

  if (!m_FFmpegPath.isEmpty())
    m_FFmpegProcess->start(m_FFmpegPath, QStringList() << "-version", QProcess::ReadOnly);

  m_GnuplotPath = QString::fromStdString(m_Preferences->Get("gnuplot", ""));

  if (!m_GnuplotPath.isEmpty())
    m_GnuplotProcess->start(m_GnuplotPath, QStringList() << "--version", QProcess::ReadOnly);
}
