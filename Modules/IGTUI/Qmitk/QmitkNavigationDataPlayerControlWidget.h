/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKNAVIGATIONDATAPLAYERCONTROLWIDGET_H
#define QMITKNAVIGATIONDATAPLAYERCONTROLWIDGET_H

#include <QWidget>
#include "mitkNavigationDataPlayer.h"
#include "MitkIGTUIExports.h"

class QTimer;

namespace Ui {
  class QmitkNavigationDataPlayerControlWidget;
}

class MITKIGTUI_EXPORT QmitkNavigationDataPlayerControlWidget : public QWidget
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

public:
  explicit QmitkNavigationDataPlayerControlWidget(QWidget *parent = nullptr);
  ~QmitkNavigationDataPlayerControlWidget() override;

  void SetPlayer(mitk::NavigationDataPlayer::Pointer player);

private:
  void ResetPlayerDisplay();

  mitk::NavigationDataPlayer::Pointer m_Player;

  QTimer*                             m_UpdateTimer;

  Ui::QmitkNavigationDataPlayerControlWidget *ui;
};

#endif // QMITKNAVIGATIONDATAPLAYERCONTROLWIDGET_H
