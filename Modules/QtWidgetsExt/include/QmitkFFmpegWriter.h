/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFFmpegWriter_h
#define QmitkFFmpegWriter_h

#include <QProcess>
#include <QSize>

#include "MitkQtWidgetsExtExports.h"

class MITKQTWIDGETSEXT_EXPORT QmitkFFmpegWriter : public QObject
{
  Q_OBJECT

public:
  explicit QmitkFFmpegWriter(QObject *parent = nullptr);
  ~QmitkFFmpegWriter() override;

  QString GetFFmpegPath() const;
  void SetFFmpegPath(const QString &path);

  QSize GetSize() const;
  void SetSize(const QSize &size);
  void SetSize(int width, int height);

  int GetFramerate() const;
  void SetFramerate(int framerate);

  QString GetOutputPath() const;
  void SetOutputPath(const QString &path);

  void Start();
  bool IsRunning() const;
  void WriteFrame(const unsigned char *frame);
  void Stop();

private slots:
  void OnProcessError(QProcess::ProcessError error);
  void OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
  QProcess *m_Process;
  QString m_FFmpegPath;
  QSize m_Size;
  int m_Framerate;
  QString m_OutputPath;
  bool m_IsRunning;
};

#endif
