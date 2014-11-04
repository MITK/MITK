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

#include "QmitkFFmpegWriter.h"
#include <QMessageBox>
#include <mitkLogMacros.h>

QmitkFFmpegWriter::QmitkFFmpegWriter(QObject* parent)
  : QObject(parent),
    m_Process(new QProcess(this))
{
  this->connect(m_Process, SIGNAL(error(QProcess::ProcessError)),
    this, SLOT(OnProcessError(QProcess::ProcessError)));

  this->connect(m_Process, SIGNAL(finished(int, QProcess::ExitStatus)),
    this, SLOT(OnProcessFinished(int, QProcess::ExitStatus)));
}

QmitkFFmpegWriter::~QmitkFFmpegWriter()
{
}

QString QmitkFFmpegWriter::GetFFmpegPath() const
{
  return m_FFmpegPath;
}

void QmitkFFmpegWriter::SetFFmpegPath(const QString& path)
{
  m_FFmpegPath = path;
}

QSize QmitkFFmpegWriter::GetSize() const
{
  return m_Size;
}

void QmitkFFmpegWriter::SetSize(const QSize& size)
{
  m_Size = size;
}

void QmitkFFmpegWriter::SetSize(int width, int height)
{
  m_Size = QSize(width, height);
}

int QmitkFFmpegWriter::GetFramerate() const
{
  return m_Framerate;
}

void QmitkFFmpegWriter::SetFramerate(int framerate)
{
  m_Framerate = framerate;
}

QString QmitkFFmpegWriter::GetOutputPath() const
{
  return m_OutputPath;
}

void QmitkFFmpegWriter::SetOutputPath(const QString& path)
{
  m_OutputPath = path;
}

void QmitkFFmpegWriter::Start()
{
  m_Process->start(m_FFmpegPath, QStringList()
    << "-y"
    << "-f" << "rawvideo"
    << "-pix_fmt" << "rgb24"
    << "-s" << QString("%1x%2").arg(m_Size.width()).arg(m_Size.height())
    << "-r" << QString("%1").arg(m_Framerate)
    << "-i" << "-"
    << "-vf" << "vflip"
    << "-pix_fmt" << "yuv420p"
    << "-crf" << "18"
    << m_OutputPath);

  m_Process->waitForStarted();
}

void QmitkFFmpegWriter::WriteFrame(const unsigned char* frame)
{
  if (frame == NULL || !m_Process->isOpen())
    return;

  m_Process->write(reinterpret_cast<const char*>(frame), m_Size.width() * m_Size.height() * 3);
  m_Process->waitForBytesWritten();
}

void QmitkFFmpegWriter::Stop()
{
  m_Process->closeWriteChannel();
}

void QmitkFFmpegWriter::OnProcessError(QProcess::ProcessError error)
{
  switch (error)
  {
  case QProcess::FailedToStart:
    QMessageBox::critical(NULL, "FFmpeg Writer", "FFmpeg failed to start!");
    break;

  case QProcess::Crashed:
    QMessageBox::critical(NULL, "FFmpeg Writer", "FFmpeg crashed!");
    break;

  case QProcess::Timedout:
     QMessageBox::critical(NULL, "FFmpeg Writer", "FFmpeg timed out!");
    break;

  case QProcess::WriteError:
     QMessageBox::critical(NULL, "FFmpeg Writer", "Could not write to FFmpeg!");
    break;

  case QProcess::ReadError:
    QMessageBox::critical(NULL, "FFmpeg Writer", "Could not read from FFmpeg!");
    break;

  default:
    QMessageBox::critical(NULL, "FFmpeg Writer", "An unknown error occurred!");
    break;
  }

  MITK_ERROR << QString::fromLatin1(m_Process->readAllStandardError()).toStdString();
}

void QmitkFFmpegWriter::OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitStatus != QProcess::CrashExit)
  {
    if (exitCode != 0)
    {
      QMessageBox::critical(NULL, "FFmpeg Writer", QString("FFmpeg exit code: %1").arg(exitCode));
      MITK_ERROR << QString::fromLatin1(m_Process->readAllStandardError()).toStdString();
    }
  }

  m_Process->close();
}
