/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCrashTestView_h
#define QmitkCrashTestView_h

#include <QmitkAbstractView.h>

#include <memory>

namespace Ui
{
  class QmitkCrashTestViewControls;
}

/**
 * \brief Deliberately crashes, throws, or freezes the application to
 *        exercise the crash-dump facility end to end.
 *
 * QA aid for the in-application paths no automated test reaches: the
 * next-start crash dialog, the "Capture diagnostics" button on the safe-mode
 * error dialog, and the UI-freeze watchdog. A list of the dumps currently in
 * the database - including the snapshot areas the next-start dialog never
 * shows - makes captures observable in-session, before shutdown purges or
 * dialog discards remove them. The plugin is off by default and must never
 * be part of a shipped application.
 */
class QmitkCrashTestView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkCrashTestView();
  ~QmitkCrashTestView() override;

protected:
  void SetFocus() override;

  void CreateQtPartControl(QWidget* parent) override;

private Q_SLOTS:

  void OnTriggerHardCrash();
  void OnThrowUnhandledException();
  void OnFreezeUiThread();
  void OnRefreshDumpList();
  void OnOpenDumpFolder();

private:
  std::unique_ptr<Ui::QmitkCrashTestViewControls> m_Controls;
};

#endif
