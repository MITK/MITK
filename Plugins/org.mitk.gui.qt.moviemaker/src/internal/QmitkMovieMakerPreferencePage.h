/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMovieMakerPreferencePage_h
#define QmitkMovieMakerPreferencePage_h

#include <berryIQtPreferencePage.h>
#include <QProcess>
#include <QString>

class QWidget;

namespace Ui
{
  class QmitkMovieMakerPreferencePage;
}

class QmitkMovieMakerPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkMovieMakerPreferencePage();
  ~QmitkMovieMakerPreferencePage() override;

  void Init(berry::IWorkbench::Pointer workbench) override;
  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;
  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

private slots:
  void OnFFmpegButtonClicked();
  void OnFFmpegProcessError(QProcess::ProcessError error);
  void OnFFmpegProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
  Ui::QmitkMovieMakerPreferencePage* m_Ui;
  QWidget* m_Control;

  QProcess* m_FFmpegProcess;
  QString m_FFmpegPath;
};

#endif
