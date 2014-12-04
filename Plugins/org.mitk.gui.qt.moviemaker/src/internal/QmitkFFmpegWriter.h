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

#ifndef QmitkFFmpegWriter_h
#define QmitkFFmpegWriter_h

#include <QProcess>
#include <QSize>

class QmitkFFmpegWriter : public QObject
{
  Q_OBJECT

public:
  explicit QmitkFFmpegWriter(QObject* parent = NULL);
  ~QmitkFFmpegWriter();

  QString GetFFmpegPath() const;
  void SetFFmpegPath(const QString& path);

  QSize GetSize() const;
  void SetSize(const QSize& size);
  void SetSize(int width, int height);

  int GetFramerate() const;
  void SetFramerate(int framerate);

  QString GetOutputPath() const;
  void SetOutputPath(const QString& path);

  void Start();
  bool IsRunning() const;
  void WriteFrame(const unsigned char* frame);
  void Stop();

private slots:
  void OnProcessError(QProcess::ProcessError error);
  void OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
  QProcess* m_Process;
  QString m_FFmpegPath;
  QSize m_Size;
  int m_Framerate;
  QString m_OutputPath;
  bool m_IsRunning;
};

#endif
