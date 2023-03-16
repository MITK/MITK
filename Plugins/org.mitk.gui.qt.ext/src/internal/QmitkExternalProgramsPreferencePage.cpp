/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkExceptionMacro.h>
#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>
#include <QFileDialog>
#include <QProcess>
#include <QTextCodec>
#include <ui_QmitkExternalProgramsPreferencePage.h>
#include "QmitkExternalProgramsPreferencePage.h"

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.gui.qt.ext.externalprograms");
  }
}

QmitkExternalProgramsPreferencePage::QmitkExternalProgramsPreferencePage()
  : m_Ui(new Ui::QmitkExternalProgramsPreferencePage),
    m_Control(nullptr),
    m_FFmpegProcess(nullptr),
    m_GnuplotProcess(nullptr)
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
  QString filter = "ffmpeg executable ";

#if defined(WIN32)
  filter += "(ffmpeg.exe)";
#else
  filter += "(ffmpeg)";
#endif

  QString ffmpegPath = QFileDialog::getOpenFileName(m_Control, "FFmpeg", "", filter);

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

    if (output.startsWith("ffmpeg"))
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
  auto* prefs = GetPreferences();

  prefs->Put("ffmpeg", m_FFmpegPath.toStdString());
  prefs->Put("gnuplot", m_GnuplotPath.toStdString());

  return true;
}

void QmitkExternalProgramsPreferencePage::Update()
{
  auto* prefs = GetPreferences();

  m_FFmpegPath = QString::fromStdString(prefs->Get("ffmpeg", ""));

  if (!m_FFmpegPath.isEmpty())
    m_FFmpegProcess->start(m_FFmpegPath, QStringList() << "-version", QProcess::ReadOnly);

  m_GnuplotPath = QString::fromStdString(prefs->Get("gnuplot", ""));

  if (!m_GnuplotPath.isEmpty())
    m_GnuplotProcess->start(m_GnuplotPath, QStringList() << "--version", QProcess::ReadOnly);
}
