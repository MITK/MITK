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

#ifndef QmitkExternalProgramsPreferencePage_h
#define QmitkExternalProgramsPreferencePage_h

#include <berryIPreferences.h>
#include <berryIQtPreferencePage.h>
#include <QProcess>
#include <QScopedPointer>

namespace Ui
{
  class QmitkExternalProgramsPreferencePage;
}

class QmitkExternalProgramsPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkExternalProgramsPreferencePage();
  ~QmitkExternalProgramsPreferencePage();

  void CreateQtControl(QWidget* parent);
  QWidget* GetQtControl() const;
  void Init(berry::IWorkbench::Pointer);
  void PerformCancel();
  bool PerformOk();
  void Update();

private slots:
  void OnFFmpegButtonClicked();
  void OnFFmpegProcessError(QProcess::ProcessError error);
  void OnFFmpegProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

  void OnGnuplotButtonClicked();
  void OnGnuplotProcessError(QProcess::ProcessError error);
  void OnGnuplotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
  berry::IPreferences::Pointer m_Preferences;
  QScopedPointer<Ui::QmitkExternalProgramsPreferencePage> m_Ui;
  QWidget* m_Control;

  QProcess* m_FFmpegProcess;
  QString m_FFmpegPath;

  QProcess* m_GnuplotProcess;
  QString m_GnuplotPath;
};

#endif
