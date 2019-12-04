/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKNAVIGATIONDATASEQUENTIALPLAYERCONTROLWIDGET_H
#define QMITKNAVIGATIONDATASEQUENTIALPLAYERCONTROLWIDGET_H

#include <QWidget>
#include "mitkNavigationDataSequentialPlayer.h"
#include "MitkIGTUIExports.h"

class QTimer;

namespace Ui {
  class QmitkNavigationDataSequentialPlayerControlWidget;
}

class MITKIGTUI_EXPORT QmitkNavigationDataSequentialPlayerControlWidget : public QWidget
{
  Q_OBJECT

signals:
  void SignalUpdate();
  void SignalEndReached();

public slots:
  void OnStop();
  void OnPlayPause();
  void OnRestart();

protected slots:
  void OnUpdate();
  void OnUpdateIntervalChanged(int);

public:
  explicit QmitkNavigationDataSequentialPlayerControlWidget(QWidget *parent = nullptr);
  ~QmitkNavigationDataSequentialPlayerControlWidget() override;

  void SetPlayer(mitk::NavigationDataSequentialPlayer::Pointer player);

protected:
  void UpdatePlayerDisplay();

private:
  mitk::NavigationDataSequentialPlayer::Pointer m_Player;

  QTimer* m_UpdateTimer;

  Ui::QmitkNavigationDataSequentialPlayerControlWidget *ui;
};

#endif // QMITKNAVIGATIONDATASEQUENTIALPLAYERCONTROLWIDGET_H
