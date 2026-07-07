/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRestApiView_h
#define QmitkRestApiView_h

#include <QmitkAbstractView.h>
#include <mitkILifecycleAwarePart.h>
#include <mitkIRestServerService.h>

#include <usServiceTracker.h>

#include <QTimer>
#include <cstdint>
#include <memory>

namespace Ui
{
  class QmitkRestApiViewControls;
}

/**
 * @brief View for monitoring and controlling the REST API server.
 *
 * Features:
 * - Shows server status (running/stopped)
 * - Start/stop server controls
 * - Tab showing nodes queried via REST API (with restapi.uid property)
 * - Tab showing nodes modified via REST API (with restapi.modified property)
 * - Tab showing request log with client IP, method, endpoint, and response code
 * - List of client IPs that have sent requests
 * - Last endpoint requested with response code
 */
class QmitkRestApiView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkRestApiView();
  ~QmitkRestApiView() override;

protected:
  void SetFocus() override;
  void CreateQtPartControl(QWidget* parent) override;

  // ILifecycleAwarePart
  void Activated() override;
  void Deactivated() override;
  void Visible() override;
  void Hidden() override;

private Q_SLOTS:
  void OnStartStopClicked();
  void OnRefreshStatus();
  void OnCopyUrlClicked();
  void OnClearLogClicked();

private:
  void UpdateServerStatus();
  void UpdateRequestLogTable();
  void SetupNodeInspectors();
  mitk::IRestServerService* GetRestServerService() const;

  std::unique_ptr<Ui::QmitkRestApiViewControls> m_Controls;
  std::unique_ptr<us::ServiceTracker<mitk::IRestServerService>> m_ServiceTracker;
  QTimer* m_StatusTimer;
  uint64_t m_LastLogVersion{0};
};

#endif
