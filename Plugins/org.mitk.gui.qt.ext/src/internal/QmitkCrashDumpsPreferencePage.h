/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkCrashDumpsPreferencePage_h
#define QmitkCrashDumpsPreferencePage_h

#include <berryIQtPreferencePage.h>

class QCheckBox;
class QSpinBox;

/**
 * \brief Crash-dump settings: recording, retention and the UI-freeze watchdog.
 *
 * The values are not MITK preferences: they live in a settings file in the
 * crash-dump folder (see mitk::CrashDumpSettings), because they are needed
 * before the preferences exist, and they take effect at the next start.
 * Registered in every build; without crash-dump support it says so.
 */
class QmitkCrashDumpsPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  void Init(berry::IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;

  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

private:
  void UpdateWatchdogEnabledState();

  QWidget* m_Control = nullptr;
  QCheckBox* m_EnabledCheckBox = nullptr;
  QSpinBox* m_MaxDumpsSpinBox = nullptr;
  QCheckBox* m_WatchdogCheckBox = nullptr;
  QSpinBox* m_WatchdogSpinBox = nullptr;
};

#endif
