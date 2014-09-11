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

#ifndef QMITKNAVIGATIONDATASEQUENTIALPLAYERCONTROLWIDGET_H
#define QMITKNAVIGATIONDATASEQUENTIALPLAYERCONTROLWIDGET_H

#include <QWidget>
#include "mitkNavigationDataSequentialPlayer.h"
#include "MitkIGTUIExports.h"

class QTimer;

namespace Ui {
  class QmitkNavigationDataSequentialPlayerControlWidget;
}

class MitkIGTUI_EXPORT QmitkNavigationDataSequentialPlayerControlWidget : public QWidget
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
  explicit QmitkNavigationDataSequentialPlayerControlWidget(QWidget *parent = 0);
  ~QmitkNavigationDataSequentialPlayerControlWidget();

  void SetPlayer(mitk::NavigationDataSequentialPlayer::Pointer player);

protected:
  void UpdatePlayerDisplay();

private:
  mitk::NavigationDataSequentialPlayer::Pointer m_Player;

  QTimer* m_UpdateTimer;

  Ui::QmitkNavigationDataSequentialPlayerControlWidget *ui;
};

#endif // QMITKNAVIGATIONDATASEQUENTIALPLAYERCONTROLWIDGET_H
