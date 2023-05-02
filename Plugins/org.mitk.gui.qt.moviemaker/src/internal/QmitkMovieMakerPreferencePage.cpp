/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMovieMakerPreferencePage.h"

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>
#include <mitkVideoRecorder.h>

#include <QFileDialog>
#include <QStringList>
#include <QTextCodec>

#include <ui_QmitkMovieMakerPreferencePage.h>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto* preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.moviemaker");
  }
}

QmitkMovieMakerPreferencePage::QmitkMovieMakerPreferencePage()
  : m_Ui(new Ui::QmitkMovieMakerPreferencePage),
    m_Control(nullptr),
    m_FFmpegProcess(nullptr)
{
}

QmitkMovieMakerPreferencePage::~QmitkMovieMakerPreferencePage()
{
}

void QmitkMovieMakerPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkMovieMakerPreferencePage::CreateQtControl(QWidget* parent)
{
  using Self = QmitkMovieMakerPreferencePage;

  m_Control = new QWidget(parent);

  m_Ui->setupUi(m_Control);

  m_Ui->formatButtonGroup->setId(m_Ui->vp9Button, static_cast<int>(mitk::VideoRecorder::OutputFormat::WebM_VP9));
  m_Ui->formatButtonGroup->setId(m_Ui->h264Button, static_cast<int>(mitk::VideoRecorder::OutputFormat::MP4_H264));

  connect(m_Ui->ffmpegButton, &QToolButton::clicked, this, &Self::OnFFmpegButtonClicked);

  m_FFmpegProcess = new QProcess(m_Control);

  connect(m_FFmpegProcess, qOverload<QProcess::ProcessError>(&QProcess::error), this, &Self::OnFFmpegProcessError);
  connect(m_FFmpegProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, &Self::OnFFmpegProcessFinished);

  this->Update();
}

QWidget* QmitkMovieMakerPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkMovieMakerPreferencePage::PerformOk()
{
  auto* prefs = GetPreferences();

  prefs->Put("ffmpeg", m_Ui->ffmpegLineEdit->text().toStdString());
  prefs->PutInt("format", m_Ui->formatButtonGroup->checkedId());

  return true;
}

void QmitkMovieMakerPreferencePage::PerformCancel()
{
}

void QmitkMovieMakerPreferencePage::Update()
{
  auto* prefs = GetPreferences();

  m_FFmpegPath = QString::fromStdString(prefs->Get("ffmpeg", ""));

  if (!m_FFmpegPath.isEmpty())
    m_FFmpegProcess->start(m_FFmpegPath, QStringList() << "-version", QProcess::ReadOnly);

  m_Ui->formatButtonGroup->button(prefs->GetInt("format", 0))->setChecked(true);
}

void QmitkMovieMakerPreferencePage::OnFFmpegButtonClicked()
{
  QString filter = "FFmpeg executable ";

#if defined(WIN32)
  filter += "(ffmpeg.exe)";
#else
  filter += "(ffmpeg)";
#endif

  auto ffmpegPath = QFileDialog::getOpenFileName(m_Control, "FFmpeg", "", filter);

  if (!ffmpegPath.isEmpty())
  {
    m_FFmpegPath = ffmpegPath;
    m_FFmpegProcess->start(ffmpegPath, QStringList() << "-version", QProcess::ReadOnly);
  }
}

void QmitkMovieMakerPreferencePage::OnFFmpegProcessError(QProcess::ProcessError)
{
  m_FFmpegPath.clear();
  m_Ui->ffmpegLineEdit->clear();
}

void QmitkMovieMakerPreferencePage::OnFFmpegProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitStatus == QProcess::NormalExit && exitCode == 0)
  {
    auto ffmpegOutput = QTextCodec::codecForName("UTF-8")->toUnicode(m_FFmpegProcess->readAllStandardOutput());

    if (ffmpegOutput.startsWith("ffmpeg"))
    {
      m_Ui->ffmpegLineEdit->setText(m_FFmpegPath);
      return;
    }
  }

  m_FFmpegPath.clear();
  m_Ui->ffmpegLineEdit->clear();
}
