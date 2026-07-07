/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRestApiPreferencePage_h
#define QmitkRestApiPreferencePage_h

#include <berryIQtPreferencePage.h>
#include <QScopedPointer>
#include <memory>

namespace Ui
{
  class QmitkRestApiPreferencesControls;
}

/**
 * @brief Preferences page for REST API server configuration.
 *
 * Allows users to configure:
 * - Server enabled/disabled
 * - Auto-start on workbench launch
 * - Host and port
 * - Thread pool size
 * - Read/write timeouts
 * - Security: client access mode, authentication, rate limiting
 * - File access restrictions
 * - HTTPS / TLS settings
 */
class QmitkRestApiPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkRestApiPreferencePage();
  ~QmitkRestApiPreferencePage() override;

  void Init(berry::IWorkbench::Pointer workbench) override;
  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;
  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

private slots:
  void OnClientAccessModeChanged(int index);
  void OnShowTokenToggled();
  void OnGenerateToken();
  void OnCopyToken();
  void OnFileAccessModeChanged(int index);
  void OnAddDirectory();
  void OnRemoveDirectory();
  void OnHttpsToggled(bool checked);
  void OnBrowseCert();
  void OnBrowseKey();

private:
  QWidget* m_Control;
  std::unique_ptr<Ui::QmitkRestApiPreferencesControls> m_Ui;
};

#endif
