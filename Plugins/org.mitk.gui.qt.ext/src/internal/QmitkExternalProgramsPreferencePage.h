/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkExternalProgramsPreferencePage_h
#define QmitkExternalProgramsPreferencePage_h

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
  ~QmitkExternalProgramsPreferencePage() override;

  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;
  void Init(berry::IWorkbench::Pointer) override;
  void PerformCancel() override;
  bool PerformOk() override;
  void Update() override;

private slots:
  void OnGnuplotButtonClicked();
  void OnGnuplotProcessError(QProcess::ProcessError error);
  void OnGnuplotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
  QScopedPointer<Ui::QmitkExternalProgramsPreferencePage> m_Ui;
  QWidget* m_Control;

  QProcess* m_GnuplotProcess;
  QString m_GnuplotPath;
};

#endif
